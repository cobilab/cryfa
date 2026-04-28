// SPDX-FileCopyrightText: 2026 Morteza Hosseini
// SPDX-License-Identifier: GPL-3.0-only

/**
 * @file ordered_pipeline.hpp
 * @brief Ordered pipeline functions
 */

#ifndef CRYFA_ORDERED_PIPELINE_HPP
#define CRYFA_ORDERED_PIPELINE_HPP

#include <algorithm>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <exception>
#include <map>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "../def.hpp"

namespace cryfa {

template <typename Chunk, typename ReadChunk, typename PackChunk, typename Emit>
void run_ordered_pipeline(size_t worker_count, ReadChunk&& read_chunk, PackChunk&& pack_chunk,
                          Emit&& emit) {
  worker_count = std::max<size_t>(1, worker_count);

  struct WorkItem {
    u64 index = 0;
    Chunk chunk;
  };

  const size_t max_in_flight = std::max<size_t>(1, worker_count * 2);
  std::mutex mutex;
  std::condition_variable work_ready;
  std::condition_variable space_ready;
  std::condition_variable result_ready;
  std::deque<WorkItem> work_queue;
  std::map<u64, std::string> results;
  std::exception_ptr error;
  bool reader_done = false;
  u64 chunks_read = 0;
  u64 next_to_write = 0;
  size_t in_flight = 0;

  auto set_error = [&](std::exception_ptr ptr) {
    std::lock_guard<std::mutex> lock(mutex);
    if (!error) {
      error = ptr;
    }
    reader_done = true;
    work_ready.notify_all();
    space_ready.notify_all();
    result_ready.notify_all();
  };

  std::thread reader([&]() {
    try {
      while (std::optional<Chunk> chunk = read_chunk()) {
        std::unique_lock<std::mutex> lock(mutex);
        space_ready.wait(lock, [&]() { return error || in_flight < max_in_flight; });
        if (error) {
          return;
        }

        work_queue.push_back(WorkItem{chunks_read++, std::move(*chunk)});
        ++in_flight;
        work_ready.notify_one();
      }

      std::lock_guard<std::mutex> lock(mutex);
      reader_done = true;
      work_ready.notify_all();
      result_ready.notify_all();
    } catch (...) {
      set_error(std::current_exception());
    }
  });

  std::vector<std::thread> workers;
  workers.reserve(worker_count);
  for (size_t i = 0; i != worker_count; ++i) {
    workers.emplace_back([&]() {
      try {
        while (true) {
          WorkItem item;
          {
            std::unique_lock<std::mutex> lock(mutex);
            work_ready.wait(lock, [&]() { return error || !work_queue.empty() || reader_done; });
            if (error || (work_queue.empty() && reader_done)) {
              return;
            }
            item = std::move(work_queue.front());
            work_queue.pop_front();
          }

          std::string packed = pack_chunk(std::move(item.chunk));

          std::lock_guard<std::mutex> lock(mutex);
          results.emplace(item.index, std::move(packed));
          result_ready.notify_all();
        }
      } catch (...) {
        set_error(std::current_exception());
      }
    });
  }

  try {
    while (true) {
      std::string packed;
      {
        std::unique_lock<std::mutex> lock(mutex);
        result_ready.wait(lock, [&]() {
          return error || results.contains(next_to_write) ||
                 (reader_done && next_to_write == chunks_read);
        });

        if (error) {
          std::rethrow_exception(error);
        }
        if (reader_done && next_to_write == chunks_read) {
          break;
        }

        auto result = results.find(next_to_write);
        packed = std::move(result->second);
        results.erase(result);
        --in_flight;
        ++next_to_write;
        space_ready.notify_one();
      }

      emit(packed);
    }
  } catch (...) {
    set_error(std::current_exception());
  }

  if (reader.joinable()) {
    reader.join();
  }
  for (auto& worker : workers) {
    if (worker.joinable()) {
      worker.join();
    }
  }

  if (error) {
    std::rethrow_exception(error);
  }
}

}  // namespace cryfa

#endif  // CRYFA_ORDERED_PIPELINE_HPP

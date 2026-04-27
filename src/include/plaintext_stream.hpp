#ifndef CRYFA_PLAINTEXT_STREAM_HPP
#define CRYFA_PLAINTEXT_STREAM_HPP

#include <algorithm>
#include <condition_variable>
#include <cstddef>
#include <deque>
#include <exception>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>

#include "../def.hpp"

namespace cryfa {

class PlaintextStream {
 public:
  explicit PlaintextStream(size_t max_buffered = std::max<size_t>(CHUNK_TARGET_SIZE * 4,
                                                                  IO_BUFFER_SIZE * 4))
      : max_buffered_(max_buffered) {}

  void push(std::string_view plaintext) {
    if (plaintext.empty()) {
      return;
    }

    std::unique_lock<std::mutex> lock(mutex_);
    space_ready_.wait(lock, [&]() {
      return error_ || buffered_bytes_ + plaintext.size() <= max_buffered_ || buffered_bytes_ == 0;
    });
    if (error_) {
      std::rethrow_exception(error_);
    }

    chunks_.emplace_back(plaintext);
    buffered_bytes_ += plaintext.size();
    data_ready_.notify_all();
  }

  void close() {
    std::lock_guard<std::mutex> lock(mutex_);
    done_ = true;
    data_ready_.notify_all();
  }

  void fail(std::exception_ptr error) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!error_) {
      error_ = error;
    }
    done_ = true;
    data_ready_.notify_all();
    space_ready_.notify_all();
  }

  auto get() -> std::optional<char> {
    std::unique_lock<std::mutex> lock(mutex_);
    data_ready_.wait(lock, [&]() { return error_ || !chunks_.empty() || done_; });
    if (error_) {
      std::rethrow_exception(error_);
    }
    if (chunks_.empty()) {
      return std::nullopt;
    }

    return pop_front(lock);
  }

  auto read_until(char delimiter, std::string& out) -> bool {
    out.clear();
    while (std::optional<char> c = get()) {
      if (*c == delimiter) {
        return true;
      }
      out += *c;
    }
    return false;
  }

  auto read_bytes(size_t size, std::string& out) -> bool {
    out.clear();
    out.reserve(size);

    size_t remaining = size;
    while (remaining != 0) {
      std::unique_lock<std::mutex> lock(mutex_);
      data_ready_.wait(lock, [&]() { return error_ || !chunks_.empty() || done_; });
      if (error_) {
        std::rethrow_exception(error_);
      }
      if (chunks_.empty()) {
        return false;
      }

      std::string& front = chunks_.front();
      const size_t available = front.size() - front_offset_;
      const size_t take = std::min(remaining, available);
      out.append(front.data() + front_offset_, take);
      front_offset_ += take;
      buffered_bytes_ -= take;
      remaining -= take;

      if (front_offset_ == front.size()) {
        chunks_.pop_front();
        front_offset_ = 0;
      }

      lock.unlock();
      space_ready_.notify_all();
    }
    return true;
  }

 private:
  auto pop_front(std::unique_lock<std::mutex>& lock) -> char {
    std::string& front = chunks_.front();
    const char c = front[front_offset_++];
    --buffered_bytes_;

    if (front_offset_ == front.size()) {
      chunks_.pop_front();
      front_offset_ = 0;
    }

    lock.unlock();
    space_ready_.notify_all();
    return c;
  }

  const size_t max_buffered_;
  std::mutex mutex_;
  std::condition_variable data_ready_;
  std::condition_variable space_ready_;
  std::deque<std::string> chunks_;
  std::exception_ptr error_;
  size_t buffered_bytes_ = 0;
  size_t front_offset_ = 0;
  bool done_ = false;
};

}  // namespace cryfa

#endif  // CRYFA_PLAINTEXT_STREAM_HPP

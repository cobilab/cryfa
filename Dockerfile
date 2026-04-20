# ── Stage 1: builder ─────────────────────────────────────────────────────────-
FROM ubuntu:22.04 AS builder
LABEL maintainer="Morteza Hosseini"

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
 && apt-get install -y --no-install-recommends build-essential git python3-pip \
 && pip3 install cmake \
 && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . .
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release \
 && cmake --build build --parallel "$(nproc)" --config Release

# ── Stage 2: runtime ─────────────────────────────────────────────────────────-
FROM debian:bookworm-slim

COPY --from=builder /src/build/cryfa  /usr/local/bin/cryfa
COPY --from=builder /src/build/keygen /usr/local/bin/keygen

ENTRYPOINT ["cryfa"]

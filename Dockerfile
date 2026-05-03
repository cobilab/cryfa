FROM ubuntu:24.04 AS builder
LABEL maintainer="Morteza Hosseini"

ARG CRYFA_VERSION=0.0.0-dev

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
    && apt-get install -y --no-install-recommends build-essential git python3-pip \
    && pip3 install --break-system-packages cmake \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY . .
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release -DCRYFA_VERSION_OVERRIDE="$CRYFA_VERSION" \
    && cmake --build build --parallel "$(nproc)" --config Release

FROM ubuntu:24.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
    && apt-get install -y --no-install-recommends libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /src/build/cryfa  /usr/local/bin/cryfa
COPY --from=builder /src/build/keygen /usr/local/bin/keygen

ENTRYPOINT ["cryfa"]

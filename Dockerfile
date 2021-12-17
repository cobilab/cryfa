FROM ubuntu:20.04
LABEL maintainer "Morteza Hosseini"

ENV DEBIAN_FRONTEND=noninteractive
RUN apt update && apt install -y cmake g++

COPY . /cryfa
WORKDIR /cryfa
RUN bash install.sh
# ENTRYPOINT ["./cryfa"]

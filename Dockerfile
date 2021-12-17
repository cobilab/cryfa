FROM ubuntu:20.04
LABEL maintainer "Morteza Hosseini"

ENV DEBIAN_FRONTEND=noninteractive
RUN apt update && apt install -y cmake g++ wget unzip

RUN wget https://github.com/smortezah/cryfa/archive/refs/heads/master.zip
RUN unzip master.zip && rm -f master.zip
RUN mv cryfa-master cryfa

WORKDIR /cryfa
RUN bash install.sh
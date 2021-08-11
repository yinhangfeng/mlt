FROM ubuntu:20.04 AS base

ENV DEBIAN_FRONTEND noninteractive

RUN apt update -qq && apt install -yqq clang pkg-config cmake libavformat-dev libswscale-dev libavutil-dev

COPY . /src

RUN cd /tmp && cmake /src && cmake --build . && cmake --install .

FROM ubuntu:20.04

ENV DEBIAN_FRONTEND noninteractive

RUN apt update -qq && apt install -yqq libavformat-dev libswscale-dev libavutil-dev

COPY --from=base /usr/local/ /usr/local/

ENV LD_LIBRARY_PATH /usr/local/lib

ENTRYPOINT ["/usr/local/bin/melt"]

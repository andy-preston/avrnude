FROM ubuntu:focal
ENV TERM=linux
ENV DEBIAN_FRONTEND=noninteractive
ENV HOME=/var/tmp
WORKDIR /usr/local/src

RUN apt-get update && apt-get -y upgrade

RUN apt-get -y --no-install-recommends install \
        make gcc libtool \
        automake autoconf \
        libusb-dev libusb-1.0-0-dev \
        libudev-dev


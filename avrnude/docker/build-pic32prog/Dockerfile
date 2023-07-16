FROM ubuntu:focal
ENV TERM=linux
ENV DEBIAN_FRONTEND=noninteractive
ENV HOME=/var/tmp
WORKDIR /usr/local/src

RUN apt-get update --fix-missing && apt-get -y upgrade

RUN apt-get -y --no-install-recommends install \
        make gcc g++ gobjc libtool \
        automake autoconf \
        libusb-dev libusb-1.0-0-dev \
        libudev-dev libfox-1.6-dev

FROM ubuntu:focal
ENV TERM=linux
ENV DEBIAN_FRONTEND=noninteractive
ENV HOME=/var/tmp
WORKDIR /usr/local/src

RUN apt-get update --fix-missing && apt-get -y upgrade

RUN apt-get -y --no-install-recommends install \
    gcc-avr binutils-avr

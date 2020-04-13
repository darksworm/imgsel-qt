FROM ubuntu:bionic

RUN apt-get update
RUN DEBIAN_FRONTEND="noninteractive" apt-get -y install tzdata
RUN apt-get install -y \
    qt5-default \
    libqt5x11extras5 \
    cmake \
    libx11-dev \
    xclip \ 
    g++ \
    tk-dev \
    libqt5x11extras5-dev

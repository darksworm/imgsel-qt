#!/usr/bin/env bash

cd $(dirname "$0")
docker build -t imgsel-builder .

cd ..
docker run --rm -it -v $(pwd):/app imgsel-builder:latest /bin/bash -c '\
cd /app && \
rm -rf build; \
mkdir build && \
cd build && \
i686-w64-mingw32.static-cmake .. \
make BUILD=i686-pc-linux-gnu MXE_TARGETS=i686-w64-mingw32.static -j 4

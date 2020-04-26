#!/usr/bin/env bash

cd $(dirname "$0")

cd ..
docker run --rm -it -v $(pwd):/app emojigun-builder:latest /bin/bash -c '\
cd /app/src && \
mkdir win-debug-build ; \
cd win-debug-build && \
i686-w64-mingw32.static-cmake .. && \
make CROSS=i686-w64-mingw32.static -j8'

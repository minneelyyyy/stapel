#!/bin/sh

rm -rf build/
cmake -S . -B build -G Ninja -DCMAKE_INSTALL_PREFIX=build/out/StapelSDK

#!/bin/sh

rm -rf build/
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=build/out/StapelSDK

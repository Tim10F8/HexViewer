#!/bin/bash


echo "=== HexViewer Build Script ==="

if [ "$1" == "clean" ]; then
    rm -rf build
    echo "Clean complete."
    exit 0
fi

if ! command -v ninja >/dev/null 2>&1; then
    echo "ERROR: Ninja is not installed."
    exit 1
fi

mkdir -p build
cd build

cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release ..
if [ $? -ne 0 ]; then exit 1; fi

ninja -j"$(nproc)"
if [ $? -ne 0 ]; then exit 1; fi

echo "Build completed successfully!"
echo "Output: build/HexViewer"

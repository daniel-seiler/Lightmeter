#!/bin/sh

cd build
cmake -D CMAKE_TOOLCHAIN_FILE=../libs/Arduino-CMake-Toolchain/Arduino-toolchain.cmake ..
make upload-lightmeter SERIAL_PORT=/dev/ttyACM1


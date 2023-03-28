#!/bin/bash
set -e
if [ ! -d build ]; then
    cmake -DCMAKE_BUILD_TYPE=Debug -DVENDOR=st -DBOARD=stm32l475_discovery -DCOMPILER=arm-gcc -S . -B build -G Ninja
fi
cmake --build build --parallel 8
arm-none-eabi-objcopy -O binary build/aws_demos.elf build/aws_demos.bin
JLink.exe con command.jlink
# sudo ~/JLink_Linux_V780b_x86_64/JLinkExe -NoGui 1 -CommandFile command.jlink

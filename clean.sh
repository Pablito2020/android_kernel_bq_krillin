#!/bin/bash

export CROSS_COMPILE=../toolchain/linaro-4.9/bin/arm-linux-androideabi-
make clean
make mrproper

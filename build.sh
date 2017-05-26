#!/bin/bash

CFLAGS="-O2 -Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes -Werror=implicit-function-declaration -Werror=missing-prototypes"

cd src
gcc $CFLAGS rpiz.c board_rpi.c cpu_arm.c util.c -o ../rpiz `pkg-config --cflags --libs gtk+-2.0`
cd ..

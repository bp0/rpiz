#!/bin/bash

# -O2
CFLAGS="-Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes -Werror=implicit-function-declaration -Werror=missing-prototypes"

cd src
rm -f ../armcpu ../arpiztest
gcc $CFLAGS $@ cpu_arm.c util.c -DDEBUG_ARMCPU -o ../armcpu
gcc $CFLAGS $@ test.c board_rpi.c cpu_arm.c util.c -o ../arpiztest
cd ..

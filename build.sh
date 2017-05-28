#!/bin/bash

CFLAGS="-O2 -Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes -Werror=implicit-function-declaration -Werror=missing-prototypes"

cd src
rm -f ../rpiz-gtk ../rpiz-cli
gcc $CFLAGS $@ rpiz-gtk.c board_rpi.c cpu_arm.c arm_data.c util.c -o ../rpiz-gtk `pkg-config --cflags --libs gtk+-2.0`
gcc $CFLAGS $@ rpiz-cli.c board_rpi.c cpu_arm.c arm_data.c util.c -o ../rpiz-cli
cd ..

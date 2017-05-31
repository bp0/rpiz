#!/bin/bash

CFLAGS="-O2 -Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes -Werror=implicit-function-declaration -Werror=missing-prototypes"
SRCC="board.c board_rpi.c board_dt.c board_dmi.c cpu_arm.c arm_data.c fields.c util.c"

cd src
rm -f ../rpiz-gtk ../rpiz-cli
gcc $CFLAGS $@ rpiz-gtk.c $SRCC -o ../rpiz-gtk `pkg-config --cflags --libs gtk+-2.0`
gcc $CFLAGS $@ rpiz-cli.c $SRCC -o ../rpiz-cli
cd ..

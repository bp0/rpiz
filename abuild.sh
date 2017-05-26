#!/bin/bash

# -O2
CFLAGS="-Wall -Wextra -Wmissing-prototypes -Wstrict-prototypes -Werror=implicit-function-declaration -Werror=missing-prototypes"

cd src
gcc $CFLAGS cpu_arm.c util.c -DDEBUG_ARMCPU -o ../armcpu
cd ..

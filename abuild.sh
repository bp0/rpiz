#!/bin/bash

cd src
gcc cpu_arm.c util.c -DDEBUG_ARMCPU -o ../armcpu
cd ..

#!/bin/sh
cc ./MultChoice.c -O3 -march=native -mtune=native -pipe -o MultChoice
#-fdump-rtl-expand => call graph

#!/bin/sh
gcc ./MultChoice.c -g -Og -march=native -mtune=native -pipe -o ~/.local/bin/MultChoice
#-fdump-rtl-expand => call graph

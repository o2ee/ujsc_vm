#!/bin/bash

gcc -m32 -g -O0 -Wall -W -fPIC  -W -D_REENTRANT -D__UNIX__ *.c inet/*.c -I./inet -I. -o uolvm

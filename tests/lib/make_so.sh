#!/bin/bash

clang -fPIC -c foo.c -o foo.o
clang -shared -o libfoo.so foo.o

exit 0
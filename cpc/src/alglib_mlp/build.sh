#!/usr/bin/env sh
ALGLIB_DIR=$HOME/install
g++ -Wall \
-I$ALGLIB_DIR/include/alglib -I.. \
main.cc \
../table.cc \
-L$ALGLIB_DIR/lib -lalglib

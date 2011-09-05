#!/usr/bin/env sh

RJ_DIR=$HOME/install

g++ -Wall \
-I$RJ_DIR/include/randomjungle \
-I$RJ_DIR/include/randomjungle/lr \
-I$RJ_DIR/include/randomjungle/library \
main.cc \
-L$RJ_DIR/lib -lrandomjungle -lgsl -lblas -lxml2 -lz

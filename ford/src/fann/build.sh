#!/usr/bin/env sh
FANN_DIR=$HOME/install
g++ \
    -Wall -O3 -march=native \
    -I. -I$FANN_DIR/include \
    main.cc table.cc nn.cc  \
    -L$FANN_DIR/lib -lfann

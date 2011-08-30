#!/usr/bin/env sh
g++ -I../../../ford/src/fann -I.. main.cc ../gini.cc ../../../ford/src/fann/nn.cc ../../../ford/src/fann/table.cc -lfann

#!/usr/bin/env sh

FANN_DIR=$HOME/install
KMEANS_DIR=../kmeans/kmlocal-1.7.2

g++ \
    -Wall -O3 -march=native \
    -I. \
    -I$FANN_DIR/include \
    -I../kmeans/kmlocal-1.7.2/src \
    main.cc table.cc \
    nn.cc \
    ../auc/auc.c \
    ../kmeans/kmeans/kmeans.cc \
    ../kmeans/kmlocal-1.7.2/src/KM_ANN.cpp \
    ../kmeans/kmlocal-1.7.2/src/KMeans.cpp \
    ../kmeans/kmlocal-1.7.2/src/KCutil.cpp \
    ../kmeans/kmlocal-1.7.2/src/KCtree.cpp \
    ../kmeans/kmlocal-1.7.2/src/KMdata.cpp \
    ../kmeans/kmlocal-1.7.2/src/KMcenters.cpp \
    ../kmeans/kmlocal-1.7.2/src/KMfilterCenters.cpp \
    ../kmeans/kmlocal-1.7.2/src/KMlocal.cpp \
    ../kmeans/kmlocal-1.7.2/src/KMterm.cpp \
    ../kmeans/kmlocal-1.7.2/src/KMrand.cpp \
    -L$FANN_DIR/lib -lfann \
    -lm

#!/usr/bin/env sh

g++ -Wall -O3 -march=native \
-I../kmlocal-1.7.2/src \
-I../../fann \
kmeans.cc \
../../fann/table.cc \
../kmlocal-1.7.2/src/KM_ANN.cpp \
../kmlocal-1.7.2/src/KMeans.cpp \
../kmlocal-1.7.2/src/KCutil.cpp \
../kmlocal-1.7.2/src/KCtree.cpp \
../kmlocal-1.7.2/src/KMdata.cpp \
../kmlocal-1.7.2/src/KMcenters.cpp \
../kmlocal-1.7.2/src/KMfilterCenters.cpp \
../kmlocal-1.7.2/src/KMlocal.cpp \
../kmlocal-1.7.2/src/KMterm.cpp \
../kmlocal-1.7.2/src/KMrand.cpp \
-lm
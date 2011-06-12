#!/usr/bin/env sh

# keep rows where cols[4] < 1

./a.out filter \
    ../../data/fordTrain_sliced.csv \
    ../../data/fordTrain_sliced_filtered.csv \
    3 '<' 0.39 ;

./a.out filter \
    ../../data/fordTrain_sliced_filtered.csv \
    ../../data/fordTrain_sliced_filtered.csv \
    6 '<' 430 ;

./a.out filter \
    ../../data/fordTrain_sliced_filtered.csv \
    ../../data/fordTrain_sliced_filtered.csv \
    10 '<' 101 ;

./a.out filter \
    ../../data/fordTrain_sliced_filtered.csv \
    ../../data/fordTrain_sliced_filtered.csv \
    13 '<' 43 ;

./a.out filter \
    ../../data/fordTrain_sliced_filtered.csv \
    ../../data/fordTrain_sliced_filtered.csv \
    14 '<' 23 ;

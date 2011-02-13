#!/usr/bin/env sh
# ./a.out slice ../../data/fordTrain.csv ../../data/fordTrainSliced.csv rows 0 12100
# ./a.out slice ../../data/fordTrain.csv ../../data/fordTrainSlicedCols.csv cols 3 4 5 6 8 9 10 11 12 14 21 23 24 25 27 28 30 31
# ./a.out slice ../../data/fordTrain_train_balanced.csv ../../data/fordTrain_train_balanced_sliced.csv  cols 3 4 5 6 8 9 10 11 12 14 21 23 24 25 27 28 30 31

# ./a.out slice ../../data/fordTrain_test_averaged_quantized.csv \
# ../../data/fordTrain_test_averaged_quantized_sliced.csv \
# cols 3 4 5 6 8 9 10 11 12 14 21 23 24 25 27 28 30 31

./a.out slice ../../data/fordTest_averaged_quantized.csv \
../../data/fordTest_averaged_quantized_sliced.csv \
cols 3 4 5 6 8 9 10 11 12 14 21 23 24 25 27 28 30 31

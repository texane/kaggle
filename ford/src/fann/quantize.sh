#!/usr/bin/env sh
# ./a.out quantize ../../data/fordTrain.csv ../../data/fordTrainQuantized.csv 
# ./a.out quantize ../../data/fordTestAveraged.csv ../../data/fordTestAveragedQuantized.csv 
# ./a.out quantize ../../data/fordDerivTrainAveraged.csv ../../data/fordDerivTrainAveragedQuantized.csv
# ./a.out quantize ../../data/fordDerivTestAveraged.csv ../../data/fordDerivTestAveragedQuantized.csv
# ./a.out quantize ../../data/fordTrainSlicedColsAveraged.csv ../../data/fordTrainSlicedColsAveragedQuantized.csv
# ./a.out quantize ../../data/fordTestSlicedColsAveraged.csv ../../data/fordTestSlicedColsAveragedQuantized.csv
# ./a.out quantize ../../data/fordTrain_train_averaged.csv ../../data/fordTrain_train_averaged_quantized.csv

./a.out quantize \
../../data/fordTrain_averaged.csv \
../../data/fordTrain_averaged_quantized.csv
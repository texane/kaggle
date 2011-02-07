#!/usr/bin/env sh
# ./a.out quantize ../../data/fordTrain.csv ../../data/fordTrainQuantized.csv 
# ./a.out quantize ../../data/fordTestAveraged.csv ../../data/fordTestAveragedQuantized.csv 
# ./a.out quantize ../../data/fordDerivTrainAveraged.csv ../../data/fordDerivTrainAveragedQuantized.csv
# ./a.out quantize ../../data/fordDerivTestAveraged.csv ../../data/fordDerivTestAveragedQuantized.csv
# ./a.out quantize ../../data/fordTrainSlicedColsAveraged.csv ../../data/fordTrainSlicedColsAveragedQuantized.csv 10
./a.out quantize ../../data/fordTestSlicedColsAveraged.csv ../../data/fordTestSlicedColsAveragedQuantized.csv 10

#!/usr/bin/env sh
#./a.out submit fanns/nn_0.fann ../../data/fordTest.csv /tmp/fu.csv
# ./a.out submit /tmp/nn.fann ../../data/fordTestAveragedQuantized.csv /tmp/fu.csv
# ./a.out submit /tmp/nn.fann ../../data/fordTestSlicedCols.csv /tmp/fu.csv
# ./a.out submit /tmp/nn.fann ../../data/fordTestSlicedColsAveragedQuantized.csv /tmp/fu.csv
# ./a.out submit fanns/fordTrainSlicedColsAveragedQuantized.fann ../../data/fordTestSlicedColsAveragedQuantized.csv /tmp/fu.csv
# ./a.out submit fanns/fordTrainSlicedColsAveraged_2x15.fann ../../data/fordTestSlicedColsAveraged.csv /tmp/fu.csv 
# ./a.out submit fanns/fordTrainSlicedColsAveragedQuantized_2x15.fann ../../data/fordTestSlicedColsAveragedQuantized.csv /tmp/fu.csv 
# ./a.out submit fanns/fordTrainSlicedColsAveragedQuantized.fann ../../data/fordTestSlicedColsAveragedQuantized.csv /tmp/fu.csv

./a.out submit \
/tmp/nn.fann \
../../data/fordTest_averaged_quantized_sliced.csv \
/tmp/fu.csv

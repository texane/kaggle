#!/usr/bin/env sh

./a.out score \
/tmp/nn.fann \
../../data/fordTrain_train_310_tids.csv

# ./a.out score \
# /tmp/nn.fann \
# ../../data/fordTrain_test_averaged_quantized_sliced.csv

# ./a.out score /tmp/nn.fann ../../data/fordTrain_test_averaged.csv
# ./a.out score /tmp/nn.fann ../../data/fordTrain_test.csv
# ./a.out score /tmp/nn.fann ../../data/fordTrainSlicedColsAveragedQuantized.csv
# ./a.out score fanns/fordTrainSlicedColsAveragedQuantized.fann ../../data/fordTrainSlicedColsAveragedQuantized.csv
# ./a.out score fanns/fordTrainSlicedColsAveragedQuantized_2x15.fann ../../data/fordTrainSlicedColsAveragedQuantized.csv
# ./a.out score /tmp/nn.fann ../../data/fordTrainSlicedColsAveraged.csv
# ./a.out score fanns/fordTrainSlicedColsAveragedQuantized.fann ../../data/fordTrainSlicedColsAveragedQuantized.csv
# ./a.out score /tmp/nn.fann ../../data/fordTrainSlicedColsAveragedQuantizedDerived.csv
# ./a.out score /tmp/nn.fann ../../data/fordTrainSlicedCols.csv
# ./a.out score /tmp/nn.fann ../../data/fordDerivonlyTrainAveragedQuantized.csv
# ./a.out score /tmp/nn.fann ../../data/fordTrainAveragedQuantized.csv
# ./a.out score /tmp/nn.fann ../../data/fordTrain.csv
# ./a.out score fanns/nn_0.fann ../../data/fordTrain.csv
# ./a.out score /tmp/nn.fann ../../data/fordTrain.csv
# ./a.out score /tmp/nn.deriv.fann ../../data/fordDerivTrain.csv
# ./a.out score /tmp/nn.derivonly.fann ../../data/fordDerivonlyTrain.csv

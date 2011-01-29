#include <stdio.h>
#include <vector>
#include "table.hh"
#include "nn.hh"


// main

int main(int ac, char** av)
{
  // in, out
  table train_tables[2];
  table test_tables[3];

  std::vector<unsigned int> cols;
  fann* nn;

  table_read_csv_file(train_tables[0], "../../data/fordTrainSmall.csv");

  // train on the first 10 rows, test on remaining ones
  table_split_at_row(test_tables[0], train_tables[0], 10);

  // xxx_tables[0] has inputs, [1] has the output
  table_split_at_col(train_tables[1], train_tables[0], 3);
  cols.resize(2); cols[0] = 0; cols[1] = 1;
  table_delete_cols(train_tables[1], cols);

  table_split_at_col(test_tables[1], test_tables[0], 3);
  cols.resize(2); cols[0] = 0; cols[1] = 1;
  table_delete_cols(test_tables[1], cols);

  // train and eval the network
  nn_create_and_train(&nn, train_tables[1], train_tables[0]);
  nn_eval(nn, test_tables[1], test_tables[2]);
  nn_destroy(nn);

  return 0;
}

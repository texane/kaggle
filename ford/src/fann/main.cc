#include <stdio.h>
#include <vector>
#include "table.hh"
#include "nn.hh"


// main

int main(int ac, char** av)
{
  table data_table, in_table, out_table;
  std::vector<unsigned int> cols;
  fann* nn;

  table_read_csv_file(data_table, "../../data/fordTrainSmall.csv");

  cols.resize(1); cols[0] = 2;
  table_extract_cols(out_table, data_table, cols);

  cols.resize(3);
  cols[0] = 0; cols[1] = 1; cols[2] = 2;
  table_delete_cols(data_table, cols);

  nn_create_and_train(&nn, data_table, out_table);
  table_destroy(data_table);
  table_destroy(out_table);

  table_read_csv_file(data_table, "../../data/fordTrainSmall.csv");
  table_delete_cols(data_table, cols);
  nn_eval(nn, data_table, out_table);
  nn_destroy(nn);

  table_print(out_table);

  return 0;
}

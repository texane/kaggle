#include <stdio.h>
#include <string>
#include <vector>
#include <map>
#include "table.hh"

int main(int ac, char** av)
{
  table table;

  if (table_create(table))
  {
    printf("[!] create\n");
    return -1;
  }

  std::vector<table::col_type> col_types;

#if 0 // fu.csv test

  col_types.push_back(table::REAL);
  col_types.push_back(table::REAL);
  col_types.push_back(table::STRING);
  col_types.push_back(table::REAL);

#else

  col_types.push_back(table::REAL);
  col_types.push_back(table::REAL);
  col_types.push_back(table::REAL);
  col_types.push_back(table::REAL);
  col_types.push_back(table::REAL);

  col_types.push_back(table::STRING);
  col_types.push_back(table::STRING);
  col_types.push_back(table::STRING);
  col_types.push_back(table::STRING);
  col_types.push_back(table::STRING);
  col_types.push_back(table::STRING);
  col_types.push_back(table::STRING);
  col_types.push_back(table::STRING);
  col_types.push_back(table::STRING);
  col_types.push_back(table::STRING);
  col_types.push_back(table::STRING);
  col_types.push_back(table::STRING);
  col_types.push_back(table::STRING);
  col_types.push_back(table::STRING);
  col_types.push_back(table::STRING);

  col_types.push_back(table::REAL);
  col_types.push_back(table::REAL);
  col_types.push_back(table::REAL);
  col_types.push_back(table::REAL);
  col_types.push_back(table::REAL);
  col_types.push_back(table::REAL);
  col_types.push_back(table::REAL);
  col_types.push_back(table::REAL);
  col_types.push_back(table::REAL);

  col_types.push_back(table::STRING);

  col_types.push_back(table::REAL);
  col_types.push_back(table::REAL);
  col_types.push_back(table::REAL);
  col_types.push_back(table::REAL);
  col_types.push_back(table::REAL);

#endif

  table_set_column_types(table, col_types);

  if (table_read_csv_file(table, av[1], true))
  {
    printf("[!] read\n");
    // return -1;
  }

  // table_print(table);
  table_write_csv_file(table, "../data/train_set_reals.csv");
  table_write_bin_file(table, "../data/train_set_reals.bin");

  table_destroy(table);

  return 0;
}

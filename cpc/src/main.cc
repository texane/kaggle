#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <map>
#include "table.hh"

#define CONFIG_TRAIN_SET 0

static double map_real(const char* s)
{
  return strtod(s, NULL);
}

static double map_capital(const char* s)
{
  return (double)(*s - 'A');
}

static double map_dots(const char* s)
{
  // expect dots format, ie.: AA.NN.MM
  // with NN and MM eventually missing.
  // where AA a string of capital letters
  // NN and MM a string of digits
  // the resulting value: (AA << 18) | (NN << 9) | (MM << 0)
  // or: (AA * 512 * 512) + (NN * 512) + MM

  unsigned int aa = 0;
  unsigned int nn = 0;
  unsigned int mm = 0;
  unsigned int i;
  unsigned int x;
  char* endptr;

  // convert AA.. style string to numeric
  // note that 'A' is mapped on 1, 'Z' on 27 so that
  // there is no 0 and 'A.0.0' != 'AA.0.0'
  for (x = 1; *s && (*s != '.'); ++i, ++s, x *= ('Z' - 'A' + 2))
    aa += (unsigned int)(*s - 'A' + 1) * x;

  // return or skip the dot
  if (*s == 0) goto return_value;
  ++s;

  // nn
  nn = strtoul(s, &endptr, 10);
  s = endptr + 1;

  if (*endptr == 0) goto return_value;

  // mm
  mm = strtoul(s, &endptr, 10);

 return_value:
  return (double)((aa << 18) | (nn << 9) | (mm << 0));
}

int main(int ac, char** av)
{
  table table;

  if (table_create(table))
  {
    printf("[!] create\n");
    return -1;
  }

  std::vector<table::col_type> col_types;
  std::vector<table::map_func_type> map_funcs;

  col_types.push_back(table::REAL);
  map_funcs.push_back(map_real);
  col_types.push_back(table::REAL);
  map_funcs.push_back(map_real);
  col_types.push_back(table::REAL);
  map_funcs.push_back(map_real);
  col_types.push_back(table::REAL);
  map_funcs.push_back(map_real);
  col_types.push_back(table::REAL);
  map_funcs.push_back(map_real);

  col_types.push_back(table::STRING);
  map_funcs.push_back(map_dots);
  col_types.push_back(table::STRING);
  map_funcs.push_back(map_dots);
  col_types.push_back(table::STRING);
  map_funcs.push_back(map_dots);
  col_types.push_back(table::STRING);
  map_funcs.push_back(map_capital);
  col_types.push_back(table::STRING);
  map_funcs.push_back(map_capital);
  col_types.push_back(table::STRING);
  map_funcs.push_back(map_capital);
  col_types.push_back(table::STRING);
  map_funcs.push_back(map_capital);
  col_types.push_back(table::STRING);
  map_funcs.push_back(map_capital);
  col_types.push_back(table::STRING);
  map_funcs.push_back(map_capital);
  col_types.push_back(table::STRING);
  map_funcs.push_back(map_capital);
  col_types.push_back(table::STRING);
  map_funcs.push_back(map_capital);
  col_types.push_back(table::STRING);
  map_funcs.push_back(map_capital);
  col_types.push_back(table::STRING);
  map_funcs.push_back(map_capital);
  col_types.push_back(table::STRING);
  map_funcs.push_back(map_capital);
  col_types.push_back(table::STRING);
  map_funcs.push_back(map_capital);

  col_types.push_back(table::REAL);
  map_funcs.push_back(map_real);
  col_types.push_back(table::REAL);
  map_funcs.push_back(map_real);
  col_types.push_back(table::REAL);
  map_funcs.push_back(map_real);
  col_types.push_back(table::REAL);
  map_funcs.push_back(map_real);
  col_types.push_back(table::REAL);
  map_funcs.push_back(map_real);
  col_types.push_back(table::REAL);
  map_funcs.push_back(map_real);
  col_types.push_back(table::REAL);
  map_funcs.push_back(map_real);
  col_types.push_back(table::REAL);
  map_funcs.push_back(map_real);
  col_types.push_back(table::REAL);
  map_funcs.push_back(map_real);

  col_types.push_back(table::STRING);
  map_funcs.push_back(map_capital);

  col_types.push_back(table::REAL);
  map_funcs.push_back(map_real);
  col_types.push_back(table::REAL);
  map_funcs.push_back(map_real);
  col_types.push_back(table::REAL);
  map_funcs.push_back(map_real);
  col_types.push_back(table::REAL);
  map_funcs.push_back(map_real);

#if CONFIG_TRAIN_SET
  col_types.push_back(table::REAL);
  map_funcs.push_back(map_real);
#endif

  table_set_column_types(table, col_types);
  table_set_map_funcs(table, map_funcs);

  if (table_read_csv_file(table, av[1], true))
  {
    printf("[!] read\n");
    // return -1;
  }

  // table_print(table);
#if CONFIG_TRAIN_SET
  table_write_csv_file(table, "../data/train_set_reals.csv");
  table_write_bin_file(table, "../data/train_set_reals.bin");
#else
  table_write_csv_file(table, "../data/test_set_reals.csv");
  table_write_bin_file(table, "../data/test_set_reals.bin");
#endif
  table_destroy(table);

  return 0;
}

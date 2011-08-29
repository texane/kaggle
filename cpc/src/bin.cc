#include <stdio.h>
#include "table.hh"

int main(int ac, char** av)
{
  static const unsigned int ncols = 35;
  unsigned int nrows;

  double* data;

  static const char* const path = "../data/train_set_reals.bin";
  if (table_read_bin_file(data, path, ncols, nrows) == -1)
  {
    printf("[!] read\n");
    return -1;
  }

  printf("(%u, %u)\n", nrows, ncols);

  return 0;
}

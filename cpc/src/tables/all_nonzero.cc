#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include "table.hh"

int table_write_csv_file
(const double* data, unsigned int nrow, unsigned int ncol, const char* path)
{
  // write only the valid values

  unsigned int i, j;

  char buf[1024];

  const int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
  if (fd == -1) return -1;

  // write names
  static const char names[] =
    "Row_ID,Household_ID,Vehicle,Calendar_Year,Model_Year,"
    "Blind_Make,Blind_Model,Blind_Submodel,"
    "Cat1,Cat2,Cat3,Cat4,Cat5,Cat6,Cat7,Cat8,Cat9,Cat10,Cat11,Cat12,OrdCat,"
    "Var1,Var2,Var3,Var4,Var5,Var6,Var7,Var8,NVCat,NVVar1,NVVar2,NVVar3,NVVar4,"
    "Claim_Amount\n";
  write(fd, names, sizeof(names) - 1);

  for (i = 0; i < nrow; ++i)
  {
    if (data[i * ncol + ncol - 1] == 0) continue;

    size_t len = 0;
    for (j = 0; j < ncol; ++j)
    {
      const double value = data[i * ncol + j];
      len += sprintf(buf + len, "%lf,", value);
    }

    buf[len - 1] = '\n';
    write(fd, buf, len);
  }

  close(fd);
  return 0;
}

int main(int ac, char** av)
{
  static const unsigned int ncols = 35;
  unsigned int nrows;

  double* data;

  static const char* const path = "../../data/train_set_reals.bin";
  if (table_read_bin_file(data, path, ncols, nrows) == -1)
  {
    printf("[!] read\n");
    return -1;
  }

  table_write_csv_file(data, nrows, ncols, "../../data/all_nonzero.csv");

  return 0;
}

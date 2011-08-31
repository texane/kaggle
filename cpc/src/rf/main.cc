#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include "linalg.h"
#include "statistics.h"
#include "dataanalysis.h"
#include "table.hh"


int main(int ac, char** av)
{
  // load the csv training file

  table table;
  table_create(table);
  table_read_csv_file(table, av[1], true);

  // transform the claim amount
  for (unsigned int i = 0; i < table.row_count; ++i)
    table.rows[i][17] = ::floor(table.rows[i][17] / 10);

  static unsigned int train_row_count = 10000;

  // turn into real_2d_array whose format is
  // var0, var1 .. varN, class

  alglib::real_2d_array x;
  x.setlength(train_row_count, table.col_count);
  for (unsigned int i = 0; i < train_row_count; ++i)
    for (unsigned int j = 0; j < table.col_count; ++j)
      x(i, j) = table.rows[i][j];

  // train a rdf

  const alglib::ae_int_t npoints = train_row_count;
  const alglib::ae_int_t nvars = table.col_count - 1;
  const alglib::ae_int_t nclasses = 2000 / 10;

  const alglib::ae_int_t ntrees = 75; // recommended value
  const double r = 0.5; // recommended value
  alglib::ae_int_t info;
  alglib::decisionforest df;
  alglib::dfreport report;

  alglib::dfbuildrandomdecisionforest
    (x, npoints, nvars, nclasses, ntrees, r, info, df, report);

  // build and classify the input set
  alglib::real_1d_array input;
  for (unsigned int i = train_row_count; i < table.row_count; ++i)
  {
    const table::row_type& row = table.rows[i];

    input.setcontent(nvars, row.data());

    alglib::real_1d_array estims;
    estims.setlength(nclasses);

    alglib::dfprocess(df, input, estims);

    // get the most likely class
    unsigned int k = 0;
    for (unsigned int j = 0; j < (unsigned int)nclasses; ++j)
      if (estims[j] > estims[k]) k = j;

    // print the class
    printf("%lf %u\n", row[row.size() - 1], k);
  }

  table_destroy(table);

  return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include "linalg.h"
#include "statistics.h"
#include "dataanalysis.h"
#include "table.hh"

static double max(const table& t, unsigned int col)
{
  double max = table::min_inf;
  for (unsigned int i = 0; i < t.row_count; ++i)
    if (t.rows[i][col] > max)
      max = t.rows[i][col];
  return max;
}

static void filter_categorical_cols(table& table)
{
  // keep only categorical columns and claimed_amount

  static const char* names[] =
  {
    "Blind_Submodel",
    "Cat1",
    "Cat2",
    "Cat3",
    "Cat4",
    "Cat5",
    "Cat6",
    "Cat7",
    "Cat8",
    "Cat9",
    "Cat10",
    "Cat11",
    "Cat12",
    "OrdCat",
    "NVCat",
    "NVVar1",
    "NVVar2",
    "NVVar3",
    "NVVar4",
    "Claim_Amount"
  };

  static const unsigned int count = sizeof(names) / sizeof(names[0]);

  std::vector<unsigned int> cols;

  for (unsigned int i = 0; i < table.col_names.size(); ++i)
  {
    bool is_found = false;

    for (unsigned int j = 0; j < count; ++j)
      if (table.col_names[i] == names[j])
      {
	is_found = true;
	break ;
      }

    if (is_found == false) cols.push_back(i);
  }

  table_delete_cols(table, cols);
}

int main(int ac, char** av)
{
  // load the csv training file

  table table;
  table_create(table);
  table_read_csv_file(table, av[1], true);

  filter_categorical_cols(table);

  // transform the claim amount
  const unsigned int output_index = table.col_count - 1;
  for (unsigned int i = 0; i < table.row_count; ++i)
  {
    if (table.rows[i][output_index] > 2000)
      table.rows[i][output_index] = 2000;

    table.rows[i][output_index] = ::round(table.rows[i][output_index]);
  }

  static unsigned int train_row_count = 5000;

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
  const alglib::ae_int_t nclasses = max(table, output_index) + 1;

//   const alglib::ae_int_t ntrees = 75; // recommended value
  const alglib::ae_int_t ntrees = 50;
  const double r = 0.4; // recommended value
  alglib::ae_int_t info;
  alglib::decisionforest df;
  alglib::dfreport report;

  alglib::dfbuildrandomdecisionforest
    (x, npoints, nvars, nclasses, ntrees, r, info, df, report);

  // build and classify the input set
  alglib::real_1d_array input;
  for (unsigned int i = 0; i < table.row_count; ++i)
  {
    const table::row_type& row = table.rows[i];

    input.setcontent(nvars, row.data());

    alglib::real_1d_array estims;
    estims.setlength(nclasses);

    alglib::dfprocess(df, input, estims);

    const unsigned int klass = (unsigned int)row[row.size() - 1];

    // get the most likely class
    unsigned int rank = 0;
    unsigned int k = 0;
    for (unsigned int j = 0; j < (unsigned int)nclasses; ++j)
    {
      if (estims[j] > estims[k]) k = j;
      if (estims[j] >= estims[klass]) ++rank;
    }

    // print the class
    printf("%u %u %u %lf\n", klass, k, rank, estims[k]);
  }

  table_destroy(table);

  return 0;
}

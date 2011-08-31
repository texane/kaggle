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
    table.rows[i][17] = ::floor(table.rows[i][17]);

  unsigned int train_row_count = 5000;
  if (train_row_count > table.row_count)
    train_row_count = table.row_count;

  // turn into real_2d_array whose format is
  // var0, var1 .. varN, class

  alglib::real_2d_array x;
  x.setlength(train_row_count, table.col_count);
  for (unsigned int i = 0; i < train_row_count; ++i)
    for (unsigned int j = 0; j < table.col_count; ++j)
      x(i, j) = table.rows[i][j];

  // create nn

  static const unsigned int nclasses = 2000;

  const alglib::ae_int_t nin = table.col_count - 1;
  const alglib::ae_int_t nhid1 = 50;
  const alglib::ae_int_t nout = nclasses;

  alglib::multilayerperceptron net;

#define CONFIG_TWO_LAYERS 1
#if CONFIG_TWO_LAYERS
  const alglib::ae_int_t nhid2 = 50;
  alglib::mlpcreatec2(nin, nhid1, nhid2, nout, net);
#else
  alglib::mlpcreatec1(nin, nhid1, nout, net);
#endif

  // set input scaling
  // for each column, compute mean and std dev
  alglib::real_1d_array samples;
  samples.setlength(train_row_count);
  for (unsigned int i = 0; i < table.col_count - 1; ++i)
  {
    // build samples
    for (unsigned j = 0; j < train_row_count; ++j)
      samples(j) = table.rows[j][i];

    double mean, var, skew, kurt;
    alglib::samplemoments(samples, mean, var, skew, kurt);

    // sigma is the stddev
    mlpsetinputscaling(net, i, mean, ::sqrt(var));
  }

  // output[i] = output[i] * sigma + mean
  // mlpsetoutputscaling(net, i, 0, 1);

  // train the network

#if 0

  alglib::ae_int_t _nin;
  alglib::ae_int_t _nout;
  alglib::ae_int_t wcount;
  alglib::mlpproperties(net, _nin, _nout, wcount);

  alglib::real_1d_array grad;
  grad.setlength(wcount);

  alglib::mlprandomizefull(net);

  double e;
  for (unsigned int epoch = 0; epoch < 100; ++epoch)
  {
    alglib::mlpgradnbatch(net, x, train_row_count, e, grad);
    printf("e == %lf\n", e);
  }

#else

  static const double decay = 0.001;
  static const alglib::ae_int_t restarts = 2;
  static const double wsteps = 0.00;
  static const alglib::ae_int_t maxits = 10;

  const alglib::ae_int_t npoints = train_row_count;

  alglib::ae_int_t info;
  alglib::mlpreport report;

  alglib::mlprandomizefull(net);

  alglib::mlptrainlbfgs
    (net, x, npoints, decay, restarts, wsteps, maxits, info, report);

  if (info != 2)
  {
    printf("[!] info: %d\n", info);
    exit(-1);
  }

#endif

  // build and classify the test set
  alglib::real_1d_array input;

  alglib::real_1d_array estims;
  estims.setlength(nclasses);

  for (unsigned int i = train_row_count; i < table.row_count; ++i)
  {
    const table::row_type& row = table.rows[i];

    input.setcontent(nin, row.data());

    alglib::mlpprocess(net, input, estims);

    // get the most likely class
    unsigned int k = 0;
    for (unsigned int j = 0; j < nclasses; ++j)
      if (estims[j] > estims[k]) k = j;

    // print the class
    printf("%lf %u (estims[k] = %lf, estims[true] = %lf)\n", row[row.size() - 1], k, estims[k], estims[(unsigned int)row[row.size() - 1]]);
  }

  table_destroy(table);

  return 0;
}

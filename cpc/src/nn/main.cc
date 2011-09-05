#include <stdio.h>
#include <vector>
#include <string>
#include <sys/types.h>
#include <sys/time.h>
#include "table.hh"
#include "nn.hh"
#include "gini.hh"

static inline double diff
(const struct timeval& start, const struct timeval& stop)
{
  struct timeval res;
  timersub(&stop, &start, &res);
  return (double)(res.tv_usec + res.tv_sec * 1E6) / 1000;
}

static void normalize_input(table& table)
{
  std::vector<double> means;
  means.resize(table.col_count);
  for (unsigned int i = 0; i < means.size(); ++i) means[i] = 0;

  for (unsigned int i = 0; i < table.row_count; ++i)
    for (unsigned int j = 0; j < table.col_count; ++j)
      means[j] += table.rows[i][j];
  for (unsigned int i = 0; i < table.col_count; ++i)
    means[i] /= table.row_count;

  std::vector<double> sigmas;
  sigmas.resize(table.col_count);
  for (unsigned int i = 0; i < sigmas.size(); ++i)  sigmas[i] = 0;

  for (unsigned int i = 0; i < table.row_count; ++i)
    for (unsigned int j = 0; j < table.col_count; ++j)
      sigmas[j] += pow(table.rows[i][j] - means[j], 2);

  for (unsigned int i = 0; i < sigmas.size(); ++i)
    sigmas[i] = sqrt(sigmas[i] / table.row_count);

  for (unsigned int i = 0; i < table.row_count; ++i)
    for (unsigned int j = 0; j < table.col_count; ++j)
      table.rows[i][j] = (table.rows[i][j] - means[j]) / sigmas[j];
}

static void normalize_output(table& table, double& max)
{
  for (unsigned int i = 0; i < table.row_count; ++i)
    table.rows[i][0] = round(table.rows[i][0]);

  max = 0;
  for (unsigned int i = 0; i < table.row_count; ++i)
    if (table.rows[i][0] > max)
      max = table.rows[i][0];

//   if (max <= 0.0001) return ;
//   for (unsigned int i = 0; i < table.row_count; ++i)
//     table.rows[i][0] /= max;
}

static void normalize_output(table& table)
{
  double max;
  normalize_output(table, max);
}

// static void hist(const table& table, unsigned int max)
// {
//   std::vector<unsigned int> hist;
//   hist.resize(max);
//   for (unsigned int i = 0; i < max; ++i) hist[i] = 0;
//   for (unsigned int i = 0; i < table.row_count; ++i)
//     ++hist[(unsigned int)table.rows[i][0]];
// }

static void train(int ac, char** av, bool retrain = false)
{
  const char* const train_path = av[0];
  const char* const sav_path = av[1];

  struct timeval start, stop;

  table train_tables[2];

  gettimeofday(&start, NULL);

  table_create(train_tables[1]);
  table_read_csv_file(train_tables[1], train_path);

  // xxx_tables[1] has inputs, [0] has the output
  table_split_at_col(train_tables[0], train_tables[1], 17);

  normalize_input(train_tables[1]);

  double max;
  normalize_output(train_tables[0], max);

  // create the output table
  unsigned int klass_count = (unsigned int)max + 1;
  table output;
  table_create(output);
  output.row_count = train_tables[0].row_count;
  output.rows.resize(output.row_count);
  output.col_count = klass_count;
  for (unsigned int i = 0; i < train_tables[0].row_count; ++i)
  {
    // zero all and set the corresponding klass proba to 1
    output.rows[i].resize(klass_count);
    for (unsigned int j = 0; j < klass_count; ++j)
      output.rows[i][j] = 0;
    output.rows[i][(unsigned int)train_tables[0].rows[i][0]] = 1;
  }

  gettimeofday(&stop, NULL);
  printf("reading_formating: %lf ms\n", diff(start, stop));

  // train the network
  gettimeofday(&start, NULL);
  struct fann* nn;
  if (retrain == false)
  {
    nn_create_and_train(&nn, train_tables[1], output);
  }
  else
  {
    nn_load(&nn, sav_path);
    nn_train(nn, train_tables[1], output);
  }
  gettimeofday(&stop, NULL);
  printf("create_and_trainting: %lf ms\n", diff(start, stop));

  nn_save(nn, sav_path);
  nn_destroy(nn);

} // train


static void eval(int ac, char** av)
{
  const char* const nn_path = av[0];
  const char* const test_path = av[1];

  struct fann* nn;
  nn_load(&nn, nn_path);

  table test_tables[2];
  table_create(test_tables[0]);
  table_read_csv_file(test_tables[0], test_path);
  table_split_at_col(test_tables[1], test_tables[0], 17);
  normalize_input(test_tables[0]);
  normalize_output(test_tables[1]);

  table output;
  nn_eval(nn, test_tables[0], output);

  // actual, prediction

  std::vector<double> a;
  std::vector<double> p;

  a.resize(output.row_count);
  p.resize(output.row_count);
  for (unsigned int i = 0; i < test_tables[2].row_count; ++i)
  {
    a[i] = test_tables[1].rows[i][0];

    unsigned int k = 0;
    for (unsigned int j = 0; j < output.col_count; ++j)
      if (output.rows[i][j] > output.rows[i][k])
	k = j;
    p[i] = (double)k;

    printf("%lf %lf(%lf, %lf)\n", a[i], p[i], output.rows[i][k], output.rows[i][(unsigned int)a[i]]);
  }
 
  const double coeff = GiniNormalized(a, p);
  printf("eval %lf\n", coeff);
} // eval


int main(int ac, char** av)
{
  if (std::string(av[1]) == "train")
    train(ac - 2, av + 2);
  else if (std::string(av[1]) == "eval")
    eval(ac - 2, av + 2);
  return 0;
}

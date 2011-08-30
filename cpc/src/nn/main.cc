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

static void train(int ac, char** av, bool retrain = false)
{
  const char* const train_path = av[0];
  const char* const sav_path = av[1];

  struct timeval start, stop;

  table train_tables[2];

  gettimeofday(&start, NULL);

  table_read_csv_file(train_tables[0], train_path);

  // xxx_tables[1] has inputs, [0] has the output
  table_split_at_col(train_tables[0], train_tables[1], 17);

  gettimeofday(&stop, NULL);
  printf("reading_formating: %lf ms\n", diff(start, stop));

  // train the network
  gettimeofday(&start, NULL);
  struct fann* nn;
  if (retrain == false)
  {
    nn_create_and_train(&nn, train_tables[1], train_tables[0]);
  }
  else
  {
    nn_load(&nn, sav_path);
    nn_train(nn, train_tables[1], train_tables[0]);
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

  table test_tables[3];
  table_read_csv_file(test_tables[0], test_path);
  table_split_at_col(test_tables[1], test_tables[0], 17);
  nn_eval(nn, test_tables[0], test_tables[2]);

  // actual, prediction

  std::vector<double> a;
  std::vector<double> p;

  a.resize(test_tables[2].row_count);
  p.resize(test_tables[2].row_count);
  for (unsigned int i = 0; i < test_tables[2].row_count; ++i)
  {
    a[i] = test_tables[1].rows[i][0];
    p[i] = test_tables[2].rows[i][0];
    printf("%lf %lf %lf\n", test_tables[0].rows[i][3], a[i], p[i]);
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

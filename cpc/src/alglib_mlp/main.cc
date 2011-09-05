#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <vector>
#include <string>
#include <algorithm>
#include "linalg.h"
#include "statistics.h"
#include "dataanalysis.h"
#include "table.hh"


static int load_mlp
(alglib::multilayerperceptron& net, const std::string& filename)
{
  const int fd = open(filename.c_str(), O_RDONLY);
  if (fd == -1) return -1;

  struct stat st;
  if (fstat(fd, &st) == -1)
  {
    close(fd);
    return -1;
  }

  std::string buf;
  buf.resize(st.st_size);

  const ssize_t nread = read(fd, (void*)buf.data(), st.st_size);
  close(fd);

  if (nread != (ssize_t)st.st_size) return -1;

  alglib::mlpunserialize(buf, net);

  return 0;
}

static int save_mlp
(alglib::multilayerperceptron& net, const std::string& filename)
{
  std::string buf;
  alglib::mlpserialize(net, buf);

  const int fd = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0600);
  if (fd == -1) return -1;
  write(fd, buf.data(), buf.size());
  close(fd);

  return 0;
}


__attribute__((unused))
static bool comparison_function(unsigned int a, unsigned int b)
{
  return a < b;
}


typedef std::vector<unsigned int> col_set;

static inline double& at
(table& t, const col_set& cs, unsigned int i, unsigned int j)
{
  return t.rows[i][cs[j]];
}

// static inline const double* at(const table& t, unsigned int i)
// {
//   return t.rows[i].data();
// }

static inline unsigned int ncols(const table& t, const col_set& cs)
{
  return cs.size();
}

__attribute__((unused))
static double max(table& t, const col_set& cs, unsigned int col)
{
  double max = table::min_inf;
  for (unsigned int i = 0; i < t.row_count; ++i)
    if (at(t, cs, i, col) > max)
      max = at(t, cs, i, col);
  return max;
}

static double max(table& t, unsigned int col)
{
  double max = table::min_inf;
  for (unsigned int i = 0; i < t.row_count; ++i)
    if (t.rows[i][col] > max)
      max = t.rows[i][col];
  return max;
}

static col_set filter_cols(const table& table)
{
  // columns [0 1 2] are for identification

  col_set cols;

  for (unsigned int i = 3; i < table.col_count - 1; ++i)
    cols.push_back(i);

#if 0 // shuffle
  srand(time(NULL) * getpid());
  for (unsigned int i = 0; i < cols.size(); ++i)
  {
    const unsigned int j = rand() % cols.size();
    std::swap(cols[i], cols[j]);
  }
  cols.resize(1 + rand() % cols.size());
  std::sort(cols.begin(), cols.end(), comparison_function);
#endif

  return cols;
}


__attribute__((unused))
static col_set get_col_from_baz(table& _table)
{
  table baz;
  table_create(baz);
  table_read_csv_file(baz, "../../data/baz.csv", true);

  col_set cols;
  for (unsigned int i = 0; i < _table.col_names.size(); ++i)
  {
    if (_table.col_names[i] == "Claim_Amount") continue ;

    for (unsigned int j = 0; j < baz.col_names.size(); ++j)
      if (_table.col_names[i] == baz.col_names[j])
      {
	cols.push_back(i);
	break ;
      }
  }

  return cols;
}

__attribute__((unused))
static col_set filter_continuous_cols(table& table)
{
  // keep only categorical columns and claimed_amount

  static const char* names[] =
  {
    "Blind_Submodel",

    "Cat1",
    "Cat1",
    "Cat4",
    "OrdCat",

    "NVCat",
    "NVVar1",
    "NVVar2",

//     "Var1",
//     "Var2",
//     "Var3",
//     "Var4",
//     "Var5",
    "Var6",
    "Var7",
    "Var8",

    "Claim_Amount",
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

  col_set cs;
  cs.resize(table.col_count);
  for (unsigned int i = 0; i < cs.size(); ++i) cs[i] = i;
  return cs;
}

static col_set get_rand_cols(table& table)
{
  // shuffle cols, not including the 7 first and last ones
  srand(getpid() * time(0));
  std::vector<unsigned int> cols;
  cols.resize(table.col_count - 1 - 7);
  for (unsigned int i = 0; i < cols.size(); ++i)
    cols[i] = i + 7;
  for (unsigned int i = 0; i < cols.size(); ++i)
  {
    const unsigned int j = rand() % cols.size();
    std::swap(cols[i], cols[j]);
  }

  // keep only the first n, with n >= 3
  // add last col (output always present)
  const unsigned int n = 3 + rand() % (cols.size() - 3);
  std::sort(cols.begin(), cols.begin() + n);
  col_set cs;
  cs.resize(n + 1);
  for (unsigned int i = 0; i < n; ++i) cs[i] = cols[i];
  cs[n] = table.col_count - 1;
  return cs;
}


static void train(int ac, char** av)
{
  // load the csv training file

  table table;
  table_create(table);
  table_read_csv_file(table, av[1], true);

  // prune the table
//   const col_set cols = filter_cols(table);
//   const col_set cols = get_col_from_baz(table);
//   const col_set cols = filter_continuous_cols(table);
  const col_set cols = get_rand_cols(table);

  for (unsigned int i = 0; i < cols.size(); ++i)
    printf("%s\n", table.col_names[cols[i]].c_str());

#define OUTPUT_RATIO 5
#define CONFIG_TRAIN_ROW_COUNT 1000

  // transform the claim amount
  const unsigned int output_index = cols.size() - 1;
  for (unsigned int i = 0; i < table.row_count; ++i)
  {
    double& value = at(table, cols, i, output_index);
    if (value > 2000) value = 2001;
    value = ::round(value / OUTPUT_RATIO);
  }

  unsigned int train_row_count = CONFIG_TRAIN_ROW_COUNT;
  if (train_row_count > table.row_count)
    train_row_count = table.row_count;

  // turn into real_2d_array whose format is
  // var0, var1 .. varN, class

  alglib::real_2d_array x;
  x.setlength(train_row_count, ncols(table, cols));

  for (unsigned int i = 0; i < train_row_count; ++i)
  {
    for (unsigned int j = 0; j < ncols(table, cols); ++j)
      x(i, j) = at(table, cols, i, j);
  }

  // create nn

  const unsigned int nclasses = max(table, cols, output_index) + 1;

  const alglib::ae_int_t nin = ncols(table, cols) - 1; // output not part
  const alglib::ae_int_t nhid1 = 20;
  const alglib::ae_int_t nout = nclasses;

  alglib::multilayerperceptron net;

#define CONFIG_TWO_LAYERS 0
#if CONFIG_TWO_LAYERS
  const alglib::ae_int_t nhid2 = 10;
  alglib::mlpcreatec2(nin, nhid1, nhid2, nout, net);
#else
  alglib::mlpcreatec1(nin, nhid1, nout, net);
#endif

  // set input scaling
  // for each column, compute mean and std dev
  alglib::real_1d_array samples;
  samples.setlength(train_row_count);
  for (unsigned int i = 0; i < ncols(table, cols) - 1; ++i)
  {
    // build samples
    for (unsigned int j = 0; j < train_row_count; ++j)
      samples(j) = at(table, cols, j, i);

    double mean, var, skew, kurt;
    alglib::samplemoments(samples, mean, var, skew, kurt);

    // sigma is the stddev
    mlpsetinputscaling(net, i, mean, ::sqrt(var));
  }

  // train the network

  static const double decay = 0.001;
  static const alglib::ae_int_t restarts = 2;
  static const double wsteps = 0.01;
  static const alglib::ae_int_t maxits = 100;

  const alglib::ae_int_t npoints = train_row_count;

  alglib::ae_int_t info;
  alglib::mlpreport report;

  alglib::mlptrainlbfgs
    (net, x, npoints, decay, restarts, wsteps, maxits, info, report);

  if (info != 2)
  {
    printf("[!] info: %d\n", info);
    return ;
  }

  save_mlp(net, av[0]);
}


static unsigned int find_col(const table& table, const std::string& name)
{
  for (unsigned int i = 0; i < table.col_count; ++i)
    if (table.col_names[i] == name) return i;
  return (unsigned int)-1;
}

#include <fstream>

static col_set read_col_file(table& table, const std::string& filename)
{
  std::ifstream ifs;
  ifs.open(filename.c_str(), std::ifstream::in);
  if (ifs.is_open() == false)
  {
    printf("[!] ifs.open()\n");
    exit(-1);
  }

  col_set cs;

  while (1)
  {
    std::string line;
    std::getline(ifs, line);
    if (line.size() == 0) break ;
    cs.push_back(find_col(table, line));
  }

  return cs;
}

static void eval(int ac, char** av)
{
  alglib::multilayerperceptron net;
  load_mlp(net, av[0]);

  table table;
  table_create(table);
  table_read_csv_file(table, av[1], true);

  col_set cols;
  if (ac > 2)
  {
    // read the column file
    cols = read_col_file(table, av[2]);
  }
  else
  {
    // static column allocation

    // const col_set cols = filter_cols(table);
    // const col_set cols = get_col_from_baz(table);
    col_set cols = filter_continuous_cols(table);
  }

  // transform the claim amount
  const unsigned int output_index = cols.size() - 1;
  for (unsigned int i = 0; i < table.row_count; ++i)
  {
    double& value = at(table, cols, i, output_index);
    if (value > 2000) value = 2001;
    value = ::round(value / OUTPUT_RATIO);
  }

  unsigned int train_row_count = CONFIG_TRAIN_ROW_COUNT;
  if (train_row_count > table.row_count)
    train_row_count = table.row_count;

  // classify

  alglib::real_1d_array estims;
  const unsigned int nclasses = max(table, output_index) + 1;
  estims.setlength(nclasses);

  alglib::real_1d_array input;
  const unsigned int nin = ncols(table, cols) - 1;
  input.setlength(nin);

  unsigned int sum = 0;
  for (unsigned int i = 0; i < train_row_count; ++i)
//   for (unsigned int i = train_row_count; i < train_row_count + 100; ++i)
//   for (unsigned int i = 0; i < table.row_count; ++i)
//   for (unsigned int i = 0; i < train_row_count + 1000; ++i)
//   for (unsigned int i = 0; i < train_row_count + 1000; ++i)
//   for (unsigned int i = train_row_count; i < train_row_count + 5000; ++i)
  {
    const unsigned int klass = (unsigned int)at(table, cols, i, output_index);

    for (unsigned int j = 0; j < (unsigned int)nin; ++j)
      input(j) = at(table, cols, i, j);

    alglib::mlpprocess(net, input, estims);

    // get the most likely class
    unsigned int rank = 0;
    unsigned int k = 0;
    for (unsigned int j = 0; j < nclasses; ++j)
    {
      if (estims[j] > estims[k]) k = j;
      if (estims[j] >= estims[klass]) ++rank;
    }

#if 0
    if (k == klass) ++hits;
    printf("%u %u (estims[k] = %lf, estims[true] = %lf) rank = %u\n",
	   klass, k, estims[k], estims[klass], rank);
#endif

    sum += abs((int)k - (int)klass);
  }

#if 0
  printf("\n");
  printf("hits: %u\n", hits);
  for (unsigned int i = 0; i < ncols(table, cols); ++i)
    printf(" %u", cols[i]);
  printf("\n");
#endif

  printf("%u\n", sum);

  table_destroy(table);
}


static void hist(int ac, char** av)
{
  table table;
  table_create(table);
  table_read_csv_file(table, av[0], true);

  const unsigned int output_index = table.col_count - 1;

  // filter rows
  std::vector<unsigned int> rows;
  for (unsigned int i = 0; i < table.row_count; ++i)
    if (table.rows[i][output_index] > 2000)
      rows.push_back(i);
  table_delete_rows(table, rows);

  // transform the claim amount
  for (unsigned int i = 0; i < table.row_count; ++i)
    table.rows[i][output_index] = ::round(table.rows[i][output_index] / OUTPUT_RATIO);

  unsigned int train_row_count = CONFIG_TRAIN_ROW_COUNT;
  if (train_row_count > table.row_count)
    train_row_count = table.row_count;

  const unsigned int _max = max(table, output_index);

  std::vector<unsigned int> hist(_max + 1, 0);

  for (unsigned int i = 0; i < train_row_count; ++i)
    ++hist[(unsigned int)table.rows[i][output_index]];

  for (unsigned int i = 0; i < hist.size(); ++i)
    if (hist[i]) printf("%u: %u\n", i, hist[i]);
}


// network ensemble

static int load_mlpe
(alglib::mlpensemble& mlpe, const std::string& filename)
{
  const int fd = open(filename.c_str(), O_RDONLY);
  if (fd == -1) return -1;

  struct stat st;
  if (fstat(fd, &st) == -1)
  {
    close(fd);
    return -1;
  }

  // load ra ae_vector
  alglib::ae_int_t rlen = st.st_size / sizeof(double);
  alglib_impl::ae_vector ra;
  alglib_impl::ae_state state;
  alglib_impl::ae_vector_init(&ra, rlen, alglib_impl::DT_REAL, &state, 0);

  const ssize_t nread = read(fd, (void*)ra.ptr.p_double, st.st_size);
  close(fd);

  if (nread != (ssize_t)st.st_size) return -1;

  alglib_impl::mlpeunserialize(&ra, mlpe.c_ptr(), &state);

  return 0;
}

static int save_mlpe
(alglib::mlpensemble& mlpe, const std::string& filename)
{
  alglib_impl::ae_vector ra;
  alglib::ae_int_t rlen;
  alglib_impl::ae_state state;

  alglib_impl::ae_vector_init(&ra, 0, alglib_impl::DT_REAL, &state, 0);
  alglib_impl::mlpeserialize(mlpe.c_ptr(), &ra, &rlen, &state);

  const int fd = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0600);
  if (fd == -1) return -1;
  write(fd, (const void*)ra.ptr.p_double, rlen * sizeof(double));
  close(fd);

  return 0;
}


static int load_table(table& table, const std::string& filename, col_set& cols)
{
  table_create(table);
  table_read_csv_file(table, filename.c_str(), true);

  // prune the table
  cols = filter_cols(table);
  // const col_set cols = get_col_from_baz(table);

  const unsigned int output_index = table.col_count - 1;

  // filter rows
  std::vector<unsigned int> rows;
  for (unsigned int i = 0; i < table.row_count; ++i)
    if (table.rows[i][output_index] > 2000)
      rows.push_back(i);
  table_delete_rows(table, rows);

  // transform the claim amount
  for (unsigned int i = 0; i < table.row_count; ++i)
    table.rows[i][output_index] = ::round(table.rows[i][output_index] / OUTPUT_RATIO);

  return 0;
}


static void moments
(
 table& table, const col_set& cols, unsigned int nrows,
 std::vector<double>& means, std::vector<double>& vars
)
{
  const unsigned int _ncols = ncols(table, cols);

  means.resize(_ncols);

  for (unsigned int i = 0; i < nrows; ++i)
    for (unsigned int j = 0; j < _ncols; ++j)
      means[j] += at(table, cols, i, j);

  for (unsigned int j = 0; j < _ncols; ++j) means[j] /= nrows;

  vars.resize(ncols(table, cols), 0);

  for (unsigned int i = 0; i < nrows; ++i)
    for (unsigned int j = 0; j < _ncols; ++j)
      vars[j] += pow(at(table, cols, i, j) - means[j], 2);

  for (unsigned int j = 0; j < _ncols; ++j) vars[j] /= nrows;
}

static void train_ensemble(int ac, char** av)
{
  // train a network ensemble

  const std::string mlpe_filename = av[0];
  const std::string csv_filename = av[1];

  table table;
  col_set cols;
  load_table(table, csv_filename, cols);

  const unsigned int output_index = table.col_count - 1;

  unsigned int train_row_count = 1000;
  if (train_row_count > table.row_count)
    train_row_count = table.row_count;

  // turn into real_2d_array whose format is
  // var0, var1 .. varN, class
  // varX are scaled with (x - mean) / stdev

  alglib::real_2d_array x;
  x.setlength(train_row_count, ncols(table, cols) + 1);

  std::vector<double> means, vars;
  moments(table, cols, train_row_count, means, vars);

  for (unsigned int i = 0; i < train_row_count; ++i)
  {
    for (unsigned int j = 0; j < ncols(table, cols); ++j)
      x(i, j) = (at(table, cols, i, j) - means[j]) / sqrt(vars[j]);
    x(i, ncols(table, cols)) = table.rows[i][output_index];
  }

  // create ensemble

  const unsigned int nclasses = max(table, output_index);

  const alglib::ae_int_t nin = ncols(table, cols);
  const alglib::ae_int_t nhid1 = 50;
  const alglib::ae_int_t nout = nclasses;

  const alglib::ae_int_t ensemblesize = 1;

  alglib::mlpensemble mlpe;

#define CONFIG_TWO_LAYERS 0
#if CONFIG_TWO_LAYERS
  const alglib::ae_int_t nhid2 = 10;
  alglib::mlpecreatec2(nin, nhid1, nhid2, nout, ensemblesize, mlpe);
#else
  alglib::mlpecreatec1(nin, nhid1, nout, ensemblesize, mlpe);
#endif

  // train the ensemble

  static const double decay = 0.001;
  static const alglib::ae_int_t restarts = 2;
  static const double wsteps = 0.01;
  static const alglib::ae_int_t maxits = 100;

  const alglib::ae_int_t npoints = train_row_count;

  alglib::ae_int_t info;
  alglib::mlpreport rep;
  alglib::mlpcvreport errs;

  alglib::mlpebagginglbfgs
    (mlpe, x, npoints, decay, restarts, wsteps, maxits, info, rep, errs);

  if (info != 2)
  {
    printf("[!] info: %d\n", info);
    return ;
  }

  save_mlpe(mlpe, mlpe_filename);
}


static void eval_ensemble(int ac, char** av)
{
  const std::string mlpe_filename = av[0];
  const std::string csv_filename = av[1];

  alglib::mlpensemble mlpe;
  load_mlpe(mlpe, mlpe_filename);

  table table;
  col_set cols;
  load_table(table, csv_filename, cols);

  const unsigned int output_index = table.col_count - 1;

  unsigned int train_row_count = 1000;
  if (train_row_count > table.row_count)
    train_row_count = table.row_count;

  std::vector<double> means, vars;
  moments(table, cols, train_row_count, means, vars);

  // classify

  alglib::real_1d_array estims;
  const unsigned int nclasses = max(table, output_index);
  estims.setlength(nclasses);

  alglib::real_1d_array input;
  const unsigned int nin = ncols(table, cols);
  input.setlength(nin);

  unsigned int hits = 0;
//   for (unsigned int i = train_row_count; i < train_row_count + 100; ++i)
//   for (unsigned int i = train_row_count; i < table.row_count; ++i)
//   for (unsigned int i = 0; i < train_row_count + 1000; ++i)
  for (unsigned int i = 0; i < train_row_count + 100; ++i)
//   for (unsigned int i = train_row_count; i < train_row_count + 5000; ++i)
  {
    const unsigned int klass = (unsigned int)table.rows[i][output_index];

    // build scaled input
    for (unsigned int j = 0; j < (unsigned int)nin; ++j)
      input(j) = (at(table, cols, i, j)) - means[j] / sqrt(vars[j]);

    alglib::mlpeprocess(mlpe, input, estims);

    // get the most likely class
    unsigned int rank = 0;
    unsigned int k = 0;
    for (unsigned int j = 0; j < nclasses; ++j)
    {
      if (estims[j] > estims[k]) k = j;
      if (estims[j] >= estims[klass]) ++rank;
    }

    if (k == klass) ++hits;

    // print the class
    printf("%u %u (estims[k] = %lf, estims[true] = %lf) rank = %u\n",
	   klass, k, estims[k], estims[klass], rank);
  }

  printf("\n");
  printf("hits: %u\n", hits);
  for (unsigned int i = 0; i < ncols(table, cols); ++i)
    printf(" %u", cols[i]);
  printf("\n");
}


struct functor
{
  const alglib::real_1d_array& _estims;

  functor(const alglib::real_1d_array& estims) : _estims(estims) {}

  bool operator()(unsigned int a, unsigned int b) const
  {
    return _estims[a] < _estims[b];
  }
};

static void sort(const alglib::real_1d_array& estims, std::vector<unsigned int>& keys)
{
  // initialize the keys
  for (unsigned int i = 0; i < (unsigned int)estims.length(); ++i) keys[i] = i;
  std::sort(keys.begin(), keys.end(), functor(estims));
}

static void eval_ensemble_fu(int ac, char** av)
{
  // foreach network, eval

  std::vector<alglib::multilayerperceptron> mlps;
  mlps.resize(ac - 1);
  for (unsigned int i = 0; i < (unsigned int)ac - 1; ++i)
    load_mlp(mlps[i], av[i]);

  table table;
  table_create(table);
  table_read_csv_file(table, av[ac - 1], true);

  // prune the table
  const col_set cols = filter_cols(table);
  // const col_set cols = get_col_from_baz(table);

  const unsigned int output_index = table.col_count - 1;

  // filter rows
  std::vector<unsigned int> rows;
  for (unsigned int i = 0; i < table.row_count; ++i)
    if (table.rows[i][output_index] > 2000)
      rows.push_back(i);
  table_delete_rows(table, rows);

  // transform the claim amount
  for (unsigned int i = 0; i < table.row_count; ++i)
    table.rows[i][output_index] = ::round(table.rows[i][output_index] / OUTPUT_RATIO);

  unsigned int train_row_count = 10000;
  if (train_row_count > table.row_count)
    train_row_count = table.row_count;

  // classify

  alglib::real_1d_array estims;
  const unsigned int nclasses = max(table, output_index) + 1;
  estims.setlength(nclasses);

  std::vector<unsigned int> keys;
  keys.resize(nclasses);

  alglib::real_1d_array input;
  const unsigned int nin = ncols(table, cols);
  input.setlength(nin);

  unsigned int hits = 0;
//   for (unsigned int i = train_row_count; i < train_row_count + 100; ++i)
//   for (unsigned int i = train_row_count; i < table.row_count; ++i)
//   for (unsigned int i = 0; i < train_row_count + 1000; ++i)
  for (unsigned int i = 0; i < train_row_count + 100; ++i)
//   for (unsigned int i = train_row_count; i < train_row_count + 5000; ++i)
  {
    const unsigned int klass = (unsigned int)table.rows[i][output_index];

    for (unsigned int j = 0; j < (unsigned int)nin; ++j)
      input(j) = at(table, cols, i, j);

    std::vector<unsigned int> ranks(nclasses, 0);

    for (unsigned int j = 0; j < mlps.size(); ++j)
    {
      alglib::mlpprocess(mlps[j], input, estims);

      // sort by decreasing order
      sort(estims, keys);

      for (unsigned int k = 0; k < keys.size(); ++k)
	ranks[keys[k]] += k;
    }

    // find the minimum rank
    unsigned int min = 0;
    for (unsigned int j = 0; j < ranks.size(); ++j)
      if (ranks[j] < ranks[min]) min = j;
    if (min == klass) ++hits;
  }

  printf("hits: %u\n", hits);

  table_destroy(table);
}

static void submit(int ac, char** av)
{
  table table;
  table_create(table);
  table_read_csv_file(table, "../../data/test_set.csv", true);
}


int main(int ac, char** av)
{
  const std::string what = av[1];
  if (what == "train") train(ac - 2, av + 2);
  else if (what == "eval") eval(ac - 2, av + 2);
  else if (what == "hist") hist(ac - 2, av + 2);
  else if (what == "submit") submit(ac - 2, av + 2);
  else if (what == "train_ensemble") train_ensemble(ac - 2, av + 2);
  else if (what == "eval_ensemble") eval_ensemble(ac - 2, av + 2);
  else if (what == "eval_ensemble_fu") eval_ensemble_fu(ac - 2, av + 2);

  return 0;
}

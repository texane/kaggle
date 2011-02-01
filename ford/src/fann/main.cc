#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <vector>
#include "table.hh"
#include "nn.hh"


using std::vector;


// find rows whose tid is less or equal tid

struct leq
{
  table::data_type _value;

  leq(const table::data_type& value) : _value(value) {};

  inline bool operator()(const table::data_type& value)
  { return value <= _value; }
};

static inline void get_tids_leq
(table& new_table, const table& table, table::data_type tid)
{
  vector<unsigned int> rows;
#define TID_COLUMN 0
  table_find_rows(rows, table, TID_COLUMN, leq(tid));
  table_extract_rows(new_table, table, rows);
}

// find rows whose tid is greater or equal tid

struct greater
{
  table::data_type _value;

  greater(const table::data_type& value) : _value(value) {};

  inline bool operator()(const table::data_type& value)
  { return value > _value; }
};

static inline void get_tids_greater
(table& new_table, const table& table, table::data_type tid)
{
  vector<unsigned int> rows;
  table_find_rows(rows, table, TID_COLUMN, greater(tid));
  table_extract_rows(new_table, table, rows);
}

// find rows whose tid is equal to tid

struct eq
{
  vector<table::data_type> _values;

  eq() {}

  eq(const vector<table::data_type>& values) : _values(values) {};

  eq(const table::data_type& value)
  { _values.resize(1); _values[0] = value; }

  inline bool operator()(const table::data_type& value)
  {
    for (size_t i = 0; i < _values.size(); ++i)
      if (_values[i] == value) return true;
    return false;
  }
};

static inline void get_tids_eq
(table& new_table, const table& table, table::data_type tid)
{
  vector<unsigned int> rows;
  table_find_rows(rows, table, TID_COLUMN, eq(tid));
  table_extract_rows(new_table, table, rows);
}

static inline void get_tids_eq
(
 table& new_table,
 const table& table,
 vector<table::data_type>::const_iterator beg,
 vector<table::data_type>::const_iterator end
)
{
  eq eq;
  eq._values.resize(end - beg);
  for (size_t i = 0; beg != end; ++beg, ++i) eq._values[i] = *beg;

  vector<unsigned int> rows;
  table_find_rows(rows, table, TID_COLUMN, eq);
  table_extract_rows(new_table, table, rows);
}


#if 0 // generate a random permutation

static inline void init_rand_once()
{
  static bool isinit = false;

  if (isinit == false)
  {
    srand(getpid() * time(NULL));
    isinit = true;
  }
}

static void shuffle(vector<table::data_type>& perm)
{
  init_rand_once();

  for (size_t i = 0; i < (perm.size() - 1); ++i)
  {
    size_t r = i + (rand() % (perm.size() - i));
    std::swap(perm[r], perm[i]);
  }

  // print permutation
  vector<table::data_type>::const_iterator pos = perm.begin();
  for (; pos != perm.end(); ++pos)
    printf("%u, ", (unsigned int)*pos);
  printf("\n");
}

#else // use a previously generated permutation

#include "static_perm.hh"

static void shuffle(vector<table::data_type>& perm)
{
  vector<table::data_type>::iterator pos = perm.begin();
  for (size_t i = 0; i < sizeof(static_perm) / sizeof(static_perm[0]); ++pos, ++i)
    *pos = static_perm[i];
}

#endif // shuffling

static inline void gen_tids_rand
(table& first_table, table& second_table, const table& table, size_t first_count)
{
  // generate a permuation of [0, 511[
#define CONFIG_MAX_TID 510
  vector<table::data_type> perm;
  perm.resize(CONFIG_MAX_TID + 1);
  for (size_t i = 0; i <= CONFIG_MAX_TID; ++i) perm[i] = (table::data_type)i;
  shuffle(perm);

  // exclusive sets
  get_tids_eq(first_table, table, perm.begin(), perm.begin() + first_count);
  get_tids_eq(second_table, table, perm.begin() + first_count, perm.end());
}


static inline double diff
(const struct timeval& start, const struct timeval& stop)
{
  struct timeval res;
  timersub(&stop, &start, &res);
  return (double)(res.tv_usec + res.tv_sec * 1E6) / 1000;
}


// subprograms

static void submit(int ac, char** av)
{
  const char* const nn_path = av[0];
  const char* const input_path = av[1];
  const char* const output_path = av[2];

  table input_tables[2], output_table;

  table_read_csv_file(input_tables[0], input_path);
  table_split_at_col(input_tables[1], input_tables[0], 3);

  struct fann* nn;
  nn_load(&nn, nn_path);
  nn_eval(nn, input_tables[1], output_table);
  nn_destroy(nn);

  for (size_t i = 0; i < output_table.row_count; ++i)
  {
    const double value = output_table.rows[i][0] < 0.5 ? 0.f : 1.f;
    input_tables[0].rows[i][2] = value;
  }

  table_write_csv_file(input_tables[0], output_path);

} // submission


static void delete_cols(table& table)
{
  // delete those cols
  // static const size_t todel[] = { 10, 13, 17, 22, 26, 27, 28, 30, 31, 32 };
  // static const size_t todel_count = sizeof(todel) / sizeof(todel[0]);

#if 0
  const size_t todel_count = 33 - 20;
  unsigned int todel[todel_count];
  for (size_t i = 0; i < todel_count; ++i)
    todel[i] = 20 + i;
#elif 0
  static const unsigned int todel[] =
    { 3, 4, 5, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32 };
  static const size_t todel_count = sizeof(todel) / sizeof(todel[0]);
#else
  static const size_t todel_count = 0;
  const unsigned int todel[] = {};
#endif

  vector<unsigned int> cols;
  cols.resize(todel_count);
  for (size_t i = 0; i < todel_count; ++i)
    cols[i] = todel[i];
  table_delete_cols(table, cols);
}


static void train(int ac, char** av, bool retrain = false)
{
  const char* const train_path = av[0];
  const char* const sav_path = av[1];

  struct timeval start, stop;

  table data_table;
  table train_tables[2];
  table test_tables[3];

  vector<unsigned int> cols;

  gettimeofday(&start, NULL);

  table_read_csv_file(data_table, train_path);

  delete_cols(data_table);

  // get 2 mutually exclusive tid sets
  gen_tids_rand(train_tables[0], test_tables[0], data_table, 500);
  table_destroy(data_table);

  // xxx_tables[1] has inputs, [0] has the output
  table_split_at_col(train_tables[1], train_tables[0], 3);
  cols.resize(2); cols[0] = 0; cols[1] = 1;
  table_delete_cols(train_tables[0], cols);

  table_split_at_col(test_tables[1], test_tables[0], 3);
  cols.resize(2); cols[0] = 0; cols[1] = 1;
  table_delete_cols(test_tables[0], cols);

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


static void score(int ac, char** av)
{
  const char* const nn_path = av[0];
  const char* const test_path = av[1];

  struct fann* nn;
  nn_load(&nn, nn_path);

  table data_table;
  table_read_csv_file(data_table, test_path);

  delete_cols(data_table);

  table dummy_table, test_tables[3];
  gen_tids_rand(dummy_table, test_tables[0], data_table, 100);
  table_destroy(data_table);

  table_split_at_col(test_tables[1], test_tables[0], 3);

  vector<unsigned int> cols;
  cols.resize(2); cols[0] = 0; cols[1] = 1;
  table_delete_cols(test_tables[0], cols);

  nn_eval(nn, test_tables[1], test_tables[2]);

  // eval the score
  table& real_table = test_tables[0];
  table& eval_table = test_tables[2];
  unsigned int sum = 0.f;
  for (size_t i = 0; i < eval_table.row_count; ++i)
  {
    for (size_t j = 0; j < eval_table.col_count; ++j)
    {
      const double value = eval_table.rows[i][j] < 0.5 ? 0.f : 1.f;
      if (real_table.rows[i][j] == value) ++sum;
    }
  }

  const unsigned int total_count = eval_table.row_count * eval_table.col_count;
  printf("score: %lf (%u)\n", (double)sum / (double)total_count, total_count);

} // score


static void deriv(int ac, char** av)
{
  // compute the derivative of the table 

  const char* const input_path = av[0];
  const char* const output_path = av[1];

  unsigned int prev_tid = (unsigned int)-1;
  table table;

  table_read_csv_file(table, input_path);

  const size_t new_col_count = 3 + (table.col_count - 3) * 2;

  for (size_t i = 0; i < table.row_count; ++i)
  {
    table.rows[i].resize(new_col_count);

    // new signal
    if (prev_tid != table.rows[i][TID_COLUMN])
    {
      for (size_t j = table.col_count; j < new_col_count; ++j)
	table.rows[i][j] = 0;
      prev_tid = table.rows[i][TID_COLUMN];
    }
    else // compute derivative
    {
      size_t j = 3, k = table.col_count;
      for ( ; j < table.col_count; ++j, ++k)
      {
	const table::data_type value = table.rows[i][j] - table.rows[i - 1][j];
	int deriv = 0;
	if (value < -0.001) deriv = -1;
	else if (value > 0.001) deriv = 1;
	table.rows[i][k] = (table::data_type)deriv;
      }
    }
  }

  table.col_count = new_col_count;

  table_write_csv_file(table, output_path);
  
} // deriv

static void derivonly(int ac, char** av)
{
  // compute the derivative of the table 

  const char* const input_path = av[0];
  const char* const output_path = av[1];

  unsigned int prev_tid = (unsigned int)-1;
  table input_table, output_table;

  table_read_csv_file(input_table, input_path);

  output_table.col_count = input_table.col_count;
  output_table.row_count = input_table.row_count;
  output_table.rows.resize(output_table.row_count);

  for (size_t i = 0; i < input_table.row_count; ++i)
  {
    output_table.rows[i].resize(output_table.col_count);

    // copy first 3 cols
    for (size_t j = 0; j < 3; ++j)
      output_table.rows[i][j] = input_table.rows[i][j];

    // new signal
    if (prev_tid != input_table.rows[i][TID_COLUMN])
    {
      for (size_t j = 3; j < output_table.col_count; ++j)
	output_table.rows[i][j] = 0;
      prev_tid = output_table.rows[i][TID_COLUMN];
    }
    else // compute derivative
    {
      for (size_t j = 3; j < output_table.col_count; ++j)
      {
	const table::data_type value = input_table.rows[i][j] - input_table.rows[i - 1][j];
	int deriv = 0;
	if (value < -0.001) deriv = -1;
	else if (value > 0.001) deriv = 1;
	output_table.rows[i][j] = (table::data_type)deriv;
      }
    }
  }

  table_write_csv_file(output_table, output_path);
  
} // derivonly


// main

int main(int ac, char** av)
{
  if (ac <= 2) printf("invalid command line\n");
  else if (strcmp(av[1], "train") == 0) train(ac - 2, av + 2);
  else if (strcmp(av[1], "retrain") == 0) train(ac - 2, av + 2, true);
  else if (strcmp(av[1], "submit") == 0) submit(ac - 2, av + 2);
  else if (strcmp(av[1], "score") == 0) score(ac - 2, av + 2);
  else if (strcmp(av[1], "deriv") == 0) deriv(ac - 2, av + 2);
  else if (strcmp(av[1], "derivonly") == 0) derivonly(ac - 2, av + 2);
  return 0;
}

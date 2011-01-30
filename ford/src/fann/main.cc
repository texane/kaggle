#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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


// generate a random permutation

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
}

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


// main

int main(int ac, char** av)
{
  struct timeval start, stop;

  // in, out
  table data_table;
  table train_tables[2];
  table test_tables[3];

  std::vector<unsigned int> cols;
  fann* nn;

  gettimeofday(&start, NULL);

  table_read_csv_file(data_table, "../../data/fordTrain.csv");
  // table_read_csv_file(data_table, "../../data/fordTrainSmall.csv");
  // table_print(data_table);

  // get 2 mutually exclusive tid sets
  gen_tids_rand(train_tables[0], test_tables[0], data_table, 50);

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
  nn_create_and_train(&nn, train_tables[1], train_tables[0]);
  gettimeofday(&stop, NULL);
  printf("create_and_trainting: %lf ms\n", diff(start, stop));

  // eval the network
  gettimeofday(&start, NULL);
  nn_eval(nn, test_tables[1], test_tables[2]);
  gettimeofday(&stop, NULL);
  printf("eval: %lf ms\n", diff(start, stop));

  nn_destroy(nn);

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

  return 0;
}

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
  for (; pos != perm.begin() + first_count; ++pos)
    printf("%u, ", (unsigned int)*pos);
  printf("\n");
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

  table input_table, output_tables[2];

  table_read_csv_file(input_table, input_path);

  vector<unsigned int> cols;
  cols.resize(3);
  cols[0] = 0; cols[1] = 1; cols[2] = 2;
  table_delete_cols(input_table, cols);

  struct fann* nn;
  nn_load(&nn, nn_path);
  nn_eval(nn, input_table, output_tables[0]);

  output_tables[1].col_count = 3;
  output_tables[1].row_count = output_tables[0].row_count;
  output_tables[1].rows.resize(output_tables[1].row_count);

  for (size_t i = 0; i < output_tables[0].row_count; ++i)
  {
    const double value = output_tables[0].rows[i][0] < 0.5 ? 0.f : 1.f;

    output_tables[1].rows[i].resize(3);
    output_tables[1].rows[i][0] = input_table.rows[i][0];
    output_tables[1].rows[i][1] = input_table.rows[i][1];
    output_tables[1].rows[i][2] = value;
  }

  table_write_csv_file(output_tables[1], output_path);

} // submission


static void train(int ac, char** av)
{
  const char* const train_path = av[0];
  const char* const sav_path = av[1];

  struct timeval start, stop;

  table data_table;
  table train_tables[2];
  table test_tables[3];

  gettimeofday(&start, NULL);

  table_read_csv_file(data_table, train_path);

  // get 2 mutually exclusive tid sets
  gen_tids_rand(train_tables[0], test_tables[0], data_table, 100);
  table_destroy(data_table);

  // xxx_tables[1] has inputs, [0] has the output
  table_split_at_col(train_tables[1], train_tables[0], 3);
  vector<unsigned int> cols;
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
  nn_create_and_train(&nn, train_tables[1], train_tables[0]);
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

  table dummy_table, test_tables[3];
  gen_tids_rand(dummy_table, test_tables[0], data_table, 100);
  table_destroy(data_table);

  vector<unsigned int> cols;
  table_split_at_col(test_tables[1], test_tables[0], 3);
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


// main

int main(int ac, char** av)
{
  if (ac <= 2) printf("invalid command line\n");
  else if (strcmp(av[1], "train") == 0) train(ac - 2, av + 2);
  else if (strcmp(av[1], "submit") == 0) submit(ac - 2, av + 2);
  else if (strcmp(av[1], "score") == 0) score(ac - 2, av + 2);
  return 0;
}

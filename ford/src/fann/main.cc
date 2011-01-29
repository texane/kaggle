#include <stdio.h>
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


// main

int main(int ac, char** av)
{
  // in, out
  table data_table;
  table train_tables[2];
  table test_tables[3];

  std::vector<unsigned int> cols;
  fann* nn;

  table_read_csv_file(data_table, "../../data/fordTrain.csv");

  printf("foo\n");
  
  // train on the first 10 , test on remaining ones
  get_tids_leq(train_tables[0], data_table, 100);

  printf("bar\n");

  get_tids_greater(test_tables[0], data_table, 100);

  printf("baz\n");

  table_destroy(data_table);

  // xxx_tables[1] has inputs, [0] has the output
  table_split_at_col(train_tables[1], train_tables[0], 3);
  cols.resize(2); cols[0] = 0; cols[1] = 1;
  table_delete_cols(train_tables[0], cols);

  table_split_at_col(test_tables[1], test_tables[0], 3);
  cols.resize(2); cols[0] = 0; cols[1] = 1;
  table_delete_cols(test_tables[0], cols);

  // train and eval the network
  nn_create_and_train(&nn, train_tables[1], train_tables[0]);
  nn_eval(nn, test_tables[1], test_tables[2]);
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

#ifndef TABLE_HH_INCLUDED
# define TABLE_HH_INCLUDED


#include <vector>


using std::vector;


// data table

typedef struct table
{
  // types
  typedef double data_type;
  typedef vector<data_type> row_type;

  // data rows
  vector<row_type> rows;

  // cached counts
  size_t row_count, col_count;

} table;


int table_create(table&);
void table_destroy(table&);
int table_read_csv_file(table&, const char*);
int table_write_csv_file(const table&, const char*);
void table_extract_cols(table&, const table&, const vector<unsigned int>&);
void table_extract_rows(table&, const table&, const vector<unsigned int>&);
void table_delete_cols(table&, const vector<unsigned int>&);
void table_split_at_col(table&, table&, unsigned int);
void table_split_at_row(table&, table&, unsigned int);
void table_print(const table&);


// inlined functions

template<typename functor_type>
void table_find_rows
(vector<unsigned int>& rows, const table& table, unsigned int col, functor_type func)
{
  size_t row_count = 0;

  rows.resize(table.row_count);

  for (size_t i = 0; i < table.row_count; ++i)
  {
    if (func(table.rows[i][col]))
    {
      // found
      rows[row_count++] = i;
    }
  }

  rows.resize(row_count);
}


#endif // ! TABLE_HH_INCLUDED

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
void table_extract_cols(table&, const table&, const vector<unsigned int>&);
void table_extract_rows(table&, const table&, const vector<unsigned int>&);
void table_delete_cols(table&, const vector<unsigned int>&);
void table_split_at_col(table&, table&, unsigned int);
void table_split_at_row(table&, table&, unsigned int);
void table_print(const table&);


#endif // ! TABLE_HH_INCLUDED

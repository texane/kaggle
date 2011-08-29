#ifndef TABLE_HH_INCLUDED
# define TABLE_HH_INCLUDED


#include <vector>
#include <string>
#include <map>
#include <sys/types.h>


// data table

typedef struct table
{
  enum col_type
  {
    REAL = 0,
    STRING,
    INVALID
  };

  // types
  typedef double data_type;
  typedef std::vector<data_type> row_type;
  static const data_type invalid_value = 24242424242;

  // data rows
  std::vector<row_type> rows;

  // cached counts
  size_t row_count, col_count;

  // column names, types
  std::vector<std::string> col_names;
  std::vector<col_type> col_types;

  // column map
  std::vector< std::map<data_type, std::string> > col_maps;

} table;


int table_create(table&);
void table_destroy(table&);
void table_set_column_types(table&, const std::vector<table::col_type>&);
void table_set_column_names(table&, const std::vector<std::string>&);
int table_map_value(table&, unsigned int, const table::data_type&, std::string& );
int table_read_csv_file(table&, const char*, bool = false);
int table_write_csv_file(const table&, const char*);
void table_extract_cols(table&, const table&, const std::vector<unsigned int>&);
void table_extract_rows(table&, const table&, const std::vector<unsigned int>&);
void table_delete_cols(table&, const std::vector<unsigned int>&);
void table_delete_rows(table&, const std::vector<unsigned int>&);
void table_split_at_col(table&, table&, unsigned int);
void table_split_at_row(table&, table&, unsigned int);
void table_print(const table&);


// inlined functions

template<typename functor_type>
void table_find_rows
(
 std::vector<unsigned int>& rows,
 const table& table,
 unsigned int col,
 functor_type func
)
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "table.hh"



int table_create(table& table)
{
  table.row_count = 0;
  table.col_count = 0;
  return 0;
}

void table_destroy(table& table)
{
  table.rows.clear();
  table.row_count = 0;
  table.col_count = 0;
}


// read a csv file

typedef struct mapped_file
{
  unsigned char* base;
  size_t off;
  size_t len;
} mapped_file_t;

typedef mapped_file_t mapped_line_t;

static int map_file(mapped_file_t* mf, const char* path)
{
  int error = -1;
  struct stat st;

  const int fd = open(path, O_RDONLY);
  if (fd == -1)
    return -1;

  if (fstat(fd, &st) == -1)
    goto on_error;

  mf->base = (unsigned char*)
    mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
  if (mf->base == MAP_FAILED)
    goto on_error;

  mf->off = 0;
  mf->len = st.st_size;

  /* success */
  error = 0;

 on_error:
  close(fd);

  return error;
}

static void unmap_file(mapped_file_t* mf)
{
  munmap((void*)mf->base, mf->len);
  mf->base = (unsigned char*)MAP_FAILED;
  mf->len = 0;
}

static int next_line(mapped_file_t* mf, mapped_line_t* ml)
{
  const unsigned char* end = mf->base + mf->len;
  const unsigned char* p;
  size_t skipnl = 0;

  ml->off = 0;
  ml->len = 0;
  ml->base = mf->base + mf->off;

  for (p = ml->base; p != end; ++p, ++ml->len)
  {
    if (*p == '\n')
    {
      skipnl = 1;
      break;
    }
  }

  if (p == ml->base) return -1;

  /* update offset */
  mf->off += (p - ml->base) + skipnl;

  return 0;
}

static int next_col(mapped_line_t* ml, unsigned char* buf)
{
  if (ml->off == ml->len) return -1;

  for (; ml->off < ml->len; ++ml->off, ++buf)
  {
    if (ml->base[ml->off] == ',') break;
    *buf = ml->base[ml->off];
  }

  *buf = 0;

  // skip comma
  if (ml->off != ml->len) ++ml->off;

  return 0;
}

static size_t get_col_count(mapped_file_t* mf)
{
  mapped_file_t tmp_mf = *mf;
  mapped_line_t ml;

  if (next_line(&tmp_mf, &ml) == -1) return 0;

  unsigned char buf[256];
  size_t col_count;
  for (col_count = 0; next_col(&ml, buf) != -1; ++col_count) ;

  return col_count;
}

static inline bool is_digit(unsigned char c)
{
  return (c >= '0' && c <= '9') || (c == '.') || (c == '-');
}

static void skip_first_line(mapped_file_t* mf)
{
  mapped_file_t tmp_mf = *mf;
  mapped_line_t ml;

  if (next_line(&tmp_mf, &ml) == -1) return ;

  unsigned char buf[256];
  if (next_col(&ml, buf) == -1) return ;

  for (size_t i = 0; buf[i]; ++i)
  {
    // skip the line if non digit token found
    if (is_digit(buf[i]) == false)
    {
      *mf = tmp_mf;
      break;
    }
  }
}

static inline int next_value
(mapped_file_t& mf, table::data_type& value)
{
  char* endptr;

  if (mf.off >= mf.len) return -1;

  value = 0;
  if (mf.base[mf.off] == '?')
    endptr = (char*)mf.base + mf.off + 1;
  else
    value = strtod((char*)mf.base + mf.off, &endptr);

  // endptr points to the next caracter
  mf.off = endptr - (char*)mf.base + 1;

  return 0;
}

int table_read_csv_file(table& table, const char* path)
{
  // table_create not assumed

  int error = -1;

  if (table_create(table) == -1) return -1;

  mapped_file_t mf;
  if (map_file(&mf, path) == -1) return -1;

  // skip first line if needed (before counting cols)
  skip_first_line(&mf);

  // get the column count
  table.col_count = get_col_count(&mf);

  vector<table::data_type> row;
  row.resize(table.col_count);

  table::data_type value;
  while (1)
  {
    // no more value
    if (next_value(mf, value) == -1) break ;

    size_t col_pos = 0;
    goto add_first_value;
    for (; col_pos < table.col_count; ++col_pos)
    {
      if (next_value(mf, value) == -1) break ;
    add_first_value:
      row[col_pos] = value;
    }

    if (col_pos != table.col_count) goto on_error;

    table.rows.push_back(row);
    ++table.row_count;
  }

  // success
  error = 0;

 on_error:
  unmap_file(&mf);
  return error;
}

int table_write_csv_file(const table& table, const char* path)
{
  char buf[1024];

  // prepend TrialID,ObsNum,Prediction
  const int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
  if (fd == -1) return -1;

  static const char* header_string = "TrialID,ObsNum,Prediction\n";
  write(fd, header_string, strlen(header_string));

  for (size_t i = 0; i < table.row_count; ++i)
  {
    size_t len = 0;
    for (size_t j = 0; j < table.col_count; ++j)
      len += sprintf(buf + len, "%lf,", table.rows[i][j]);
    buf[len - 1] = '\n';
    write(fd, buf, len);
  }

  close(fd);
  return 0;
}

void table_extract_cols
(table& new_table, const table& table, const vector<unsigned int>& cols)
{
  new_table.row_count = table.rows.size();
  new_table.col_count = cols.size();

  new_table.rows.resize(new_table.row_count);
  for (size_t i = 0; i < table.row_count; ++i)
  {
    new_table.rows[i].resize(cols.size());
    for (size_t j = 0; j < cols.size(); ++j)
      new_table.rows[i][j] = table.rows[i][cols[j]];
  }
}

void table_delete_cols(table& table, const vector<unsigned int>& cols)
{
  // assume cols sorted in ascending order

  for (size_t i = 0; i < table.row_count; ++i)
  {
    unsigned int count = 0;
    for (size_t j = 0; j < cols.size(); ++j, ++count)
    {
      table::row_type::iterator pos = table.rows[i].begin() + (cols[j] - count);
      table.rows[i].erase(pos);
    }
  }

  table.col_count -= cols.size();
}

void table_delete_rows(table& table, const vector<unsigned int>& rows)
{
  // assume rows sorted in ascending order

  for (size_t i = 0; i < rows.size(); ++i)
    table.rows.erase(table.rows.begin() + (rows[i] - i));
  table.row_count -= rows.size();
}

void table_extract_rows
(table& new_table, const table& table, const vector<unsigned int>& rows)
{
  new_table.row_count = rows.size();
  new_table.col_count = table.col_count;

  new_table.rows.resize(new_table.row_count);
  for (size_t i = 0; i < rows.size(); ++i)
    new_table.rows[i] = table.rows[rows[i]];
}

void table_split_at_col
(table& new_table, table& table, unsigned int pivot)
{
  // new_table gets cols[pivot, col_count[

  new_table.col_count = table.col_count - pivot;
  new_table.row_count = table.row_count;
  new_table.rows.resize(new_table.row_count);

  for (size_t i = 0; i < table.row_count; ++i)
  {
    new_table.rows[i].resize(new_table.col_count);

    // copy cols from table to new_table
    table::row_type::iterator pos = new_table.rows[i].begin();
    for (size_t j = pivot; j < table.col_count; ++j, ++pos)
      *pos = table.rows[i][j];

    // erase cols from table
    table.rows[i].erase(table.rows[i].begin() + pivot, table.rows[i].end());
  }

  table.col_count = pivot;
}

void table_split_at_row
(table& new_table, table& table, unsigned int pivot)
{
  // new_table gets rows[pivot, row_count[

  // assume pivot <= row_count
  new_table.row_count = table.row_count - pivot;
  new_table.rows.resize(new_table.row_count);
  new_table.col_count = table.col_count;

  // copy rows from table to new_table
  vector<table::row_type>::iterator pos = table.rows.begin() + pivot;
  for (size_t i = 0; i < new_table.row_count; ++i, ++pos)
    new_table.rows[i] = *pos;

  // erase from table
  table.rows.erase(table.rows.begin() + pivot, table.rows.end());
  table.row_count = pivot;
}


#if 0 // helper routines

static void append_derivatives(table& table)
{
  // append the derivative to end of rows
}

#endif


void table_print(const table& table)
{
  for (size_t i = 0; i < table.row_count; ++i)
  {
    for (size_t j = 0; j < table.col_count; ++j)
      printf("%lf,", table.rows[i][j]);
    printf("\n");
  }
  printf("\n");
}

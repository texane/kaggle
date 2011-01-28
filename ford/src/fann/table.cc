#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
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

static table::data_type buf_to_value(const unsigned char* buf)
{
  return strtod((const char*)buf, NULL);
}

static size_t get_col_count(mapped_file_t* mf)
{
  mapped_file_t tmp_mf = *mf;
  mapped_line_t ml;

  if (next_line(&tmp_mf, &ml) == -1) return 0;

  unsigned char buf[256];
  size_t col_count = 0;
  while (next_col(&ml, buf) != -1) ++col_count;

  return col_count;
}

static inline bool is_digit(unsigned char c)
{
  return (c >= '0' && c <= '9') || c == '.';
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

int table_read_csv_file(table& table, const char* path)
{
  // table_create not assumed

  if (table_create(table) == -1) return -1;

  mapped_file_t mf;
  if (map_file(&mf, path) == -1) return -1;

  // get the column count
  table.col_count = get_col_count(&mf);

  // skip first line if needed
  skip_first_line(&mf);

  table::row_type row;
  row.resize(table.col_count);

  unsigned char buf[256];
  mapped_line_t ml;
  while (next_line(&mf, &ml) != -1)
  {
    size_t col_pos = 0;
    while (next_col(&ml, buf) != -1)
      row[col_pos++] = buf_to_value(buf);

    table.rows.push_back(row);
    ++table.row_count;
  }

  unmap_file(&mf);
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
      printf("del %u\n", cols[j] - count);
      table::row_type::iterator pos = table.rows[i].begin() + (cols[j] - count);
      table.rows[i].erase(pos);
    }
  }

  table.col_count -= cols.size();
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

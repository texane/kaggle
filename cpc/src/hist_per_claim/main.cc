#define CONFIG_USE_BMP 0
#define CONFIG_CLAIM_RATIO 1

#include <stdio.h>
#include <math.h>
#include <vector>
#include <map>
#include <algorithm>
#include "table.hh"

#if CONFIG_USE_BMP
#include "bmp.h"
#endif

typedef struct compare_functor
{
  const double* _vals;

  compare_functor(const double* vals) : _vals(vals) {}

  bool operator()(unsigned int i, unsigned int j)
  {
    return _vals[i] > _vals[j];
  }
} compare_functor;

typedef struct hist_info
{
  // per column histogram
  unsigned int nrows;
  unsigned int ncols;

  // hist[i * ncols + j] where i the column
  // value, j the claimed amount
  std::vector<double> hist;

  std::vector<double> means;
} hist_info;

static inline double& at(hist_info& hi, unsigned int i, unsigned int j)
{
  return hi.hist[i * hi.ncols + j];
}

__attribute__((unused))
static double inverse
(std::map<double, unsigned int>& mappings, unsigned int val)
{
  std::map<double, unsigned int>::iterator pos = mappings.begin();
  std::map<double, unsigned int>::iterator end = mappings.end();

  for (; pos != end; ++pos)
    if (pos->second == val)
      return pos->first;

  return -1;
}

static void compute
(
 hist_info& hi,
 table& table,
 unsigned int col,
 unsigned int nclaims,
 std::map<double, unsigned int>& mappings
)
{
  std::vector<double> keys;

  for (unsigned int i = 0; i < table.row_count; ++i)
  {
    const double value = table.rows[i][col];
    std::map<double, unsigned int>::iterator pos = mappings.find(value);
    if (pos == mappings.end())
    {
      mappings.insert(std::pair<double, unsigned int>(value, 0));
      keys.push_back(value);
    }
  }

  std::sort(keys.begin(), keys.end());
  for (unsigned int i = 0; i < keys.size(); ++i)
    mappings[keys[i]] = i;

  const unsigned int nrows = keys.size();
  const unsigned int ncols = nclaims;
  hi.hist.resize(nrows * ncols, 0);
  hi.nrows = nrows;
  hi.ncols = ncols;

  for (unsigned int i = 0; i < table.row_count; ++i)
  {
    const unsigned int value = mappings[table.rows[i][col]];
    const unsigned int claimed = (unsigned int)
      table.rows[i][table.col_count - 1];
    ++at(hi, value, claimed);
  }
}

__attribute__((unused))
static void normalize(hist_info& hi)
{
  double min = table::plus_inf;
  double max = table::min_inf;

  for (unsigned int i = 0; i < hi.nrows; ++i)
    for (unsigned int j = 0; j < hi.ncols; ++j)
    {
      const double& val = at(hi, i, j);
      if (val > max) max = val;
      if (val < min) min = val;
    }

  for (unsigned int i = 0; i < hi.nrows; ++i)
    for (unsigned int j = 0; j < hi.ncols; ++j)
    {
      double& val = at(hi, i, j);
      val = (val - min) / (max - min);
    }
}

static unsigned int find_nearest
(std::map<double, unsigned int>& mappings, double val)
{
  std::map<double, unsigned int>::iterator pos = mappings.begin();
  std::map<double, unsigned int>::iterator end = mappings.end();

  double best_dist = fabs(pos->first - val);
  unsigned int best_second = pos->second;

  for (; pos != end; ++pos)
  {
    const double dist = fabs(pos->first - val);
    if (dist < best_dist)
    {
      best_dist = dist;
      best_second = pos->second;
    }
  }

  return best_second;
}

__attribute__((unused))
static double predict
(
 std::vector<hist_info>& his,
 const double* row,
 std::vector< std::map<double, unsigned int> >& mappings
)
{
  double pred = 0;

  // assume size(row) == his.size()
  for (unsigned int i = 3; i < his.size() - 1; ++i)
  {
    std::map<double, unsigned int>& m = mappings[i];
    const unsigned int j = find_nearest(m, row[i]);

    hist_info& hi = his[i];
    pred += hi.means[j];
  }

  return pred;
}

int main(int ac, char** av)
{
  // build the model

  table table, new_table;
  table_create(table);
  table_read_csv_file(table, "../../data/all_valid_nonzero.csv", true);

  double max_claim = table::min_inf;
  for (unsigned int i = 0; i < table.row_count; ++i)
  {
    double val = table.rows[i][table.col_count - 1];
    if (val > 2000) val = 2000;
    val = round(table.rows[i][table.col_count - 1] / CONFIG_CLAIM_RATIO);
    table.rows[i][table.col_count - 1] = val;
    if (val > max_claim) max_claim = val;
  }

  std::vector< std::map<double, unsigned int> > mappings(table.col_count);

  std::vector<hist_info> his(table.col_count);
  for (unsigned int j = 3; j < table.col_count - 1; ++j)
  {
    hist_info& hi = his[j];
    compute(hi, table, j, (unsigned int)(max_claim + 1), mappings[j]);
    // normalize(hi);
  }

#if CONFIG_USE_BMP
  for (unsigned int j = 3; j < table.col_count - 1; ++j)
  {
    hist_info& hi = his[j];

    struct bmp* image = bmp_create();

    bmp_set_format(image, 24, hi.ncols, hi.nrows);

    for (unsigned int y = 0; y < hi.nrows; ++y)
      for (unsigned int x = 0; x < hi.ncols; ++x)
      {
	const double value = at(hi, y, x);
	uint8_t color = (uint8_t)((double)0xff * value);
	const uint8_t colors[3] = { color, color, color };
	bmp_put_pixel(image, x, y, colors);
      }

    char filename[256];
    sprintf(filename, "/tmp/bmp/%u.bmp", j);
    bmp_store_file(image, filename);
    bmp_destroy(image);
  }
#endif

  // foreach column histogram
  for (unsigned int i = 3; i < table.col_count - 1; ++i)
  {
    hist_info& hi = his[i];
    hi.means.resize(hi.nrows);

    printf("\n");

    // foreach value, sort by cumulated value
    for (unsigned int j = 0; j < hi.nrows; ++j)
    {
      std::vector<unsigned int> keys(hi.ncols);
      for (unsigned int k = 0; k < keys.size(); ++k) keys[k] = k;
      const double* const row = hi.hist.data() + j * hi.ncols;
      sort(keys.begin(), keys.end(), compare_functor(row));

#if 0
      // print the first N ones
#define N 5
      printf("table$%02u$%03u:", i, j);
      unsigned int size = keys.size();
      if (size > N) size = N;
      for (unsigned int k = 0; k < size; ++k)
	printf(" %05u(%05u) ", keys[k], (unsigned int)row[keys[k]]);
	// printf(" %05u(%lf) ", keys[k], row[keys[k]]);
	// printf(" %lf", row[keys[k]]);
      printf("\n");
#endif

      // print stat infos: for a given histogram value, gives
      // min/max claimed amount
      // mean claimed amount
      // median claimed amount
      unsigned int min = +10000;
      unsigned int max = 0;
      unsigned int claim_sum = 0;
      unsigned int pop_sum = 0;
      std::vector<unsigned int> claims;
      for (unsigned int k = 0; k < keys.size(); ++k)
      {
	const unsigned int claim_val = keys[k];
	const unsigned int pop_val = (unsigned int)row[claim_val];

	if (pop_val == 0) continue ;

	pop_sum += pop_val;

	claim_sum += claim_val;
	claims.push_back(claim_val);
	if (claim_val > max) max = claim_val;
	if (claim_val < min) min = claim_val;
      }
      std::sort(claims.begin(), claims.end());

      const double mean = (double)claim_sum / (double)pop_sum;
      hi.means[j] = mean;

#if 0
      const double mapped = inverse(mappings[i], j);
      const unsigned int med = claims[claims.size() / 2];
      printf("table$%02u$%lf: [%05u - %05u] (mean=%lf, med=%u) %u\n",
	     i, mapped, min, max, mean, med, pop_sum);
#endif
    }
  }

  table_destroy(table);

  // predict
  table_create(new_table);
  table_read_csv_file(new_table, "../../data/test_set_reals.csv", true);
  for (unsigned int i = 0; i < table.row_count; ++i)
  {
    const double* const row = new_table.rows[i].data();
    const double pred = predict(his, row, mappings);
    printf("%lf\n", pred); fflush(stdout);
  }
  table_destroy(new_table);

  return 0;
}

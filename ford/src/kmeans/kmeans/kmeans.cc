#include <vector>
#include "table.hh"
#include "KMeans.h"
#include "KMdata.h"
#include "KMcenters.h"
#include "KMfilterCenters.h"
#include "KMterm.h"
#include "KMlocal.h"


using std::vector;


int kmeans(table& table, vector< vector<double> >& centers)
{
  const size_t center_count = centers.size();

  // generate the data points

  const size_t point_count = table.row_count;
  const size_t point_dim = table.col_count;

  KMdata points(point_dim, point_count);
  for (size_t row = 0; row < table.row_count; ++row)
    for (size_t col = 3; col < table.col_count; ++col)
      points[row][col] = table.rows[row][col];

  points.buildKcTree();

  // build the center set

  KMfilterCenters _centers(center_count, points);

  // execute the algorithm

  KMterm term(100, 0, 0, 0, .1, .1, 3, .5, 10, .95);
  KMlocalHybrid algo(_centers, term);
  _centers = algo.execute();

  // get the centers back

  for (size_t i = 0; i < center_count; ++i)
  {
    centers[i].resize(point_dim);
    for (size_t j = 0; j < point_dim; ++j)
      centers[i][j] = _centers[i][j];
  }

  return 0;
}


#if 0 // unit

int main(int ac, char** av)
{
  // generate a table
  table table;
  table.col_count = 5;
  table.row_count = 10;
  table.rows.resize(table.row_count);
  for (size_t row = 0; row < table.row_count; ++row)
  {
    table.rows[row].resize(table.row_count);
    for (size_t col = 0; col < table.col_count; ++col)
      table.rows[row][col] = (table::data_type)(row);
  }

  // clusterize
  vector< vector<double> > centers(2);
  kmeans(table, centers);

  // print centers
  for (size_t i = 0; i < centers.size(); ++i)
  {
    for (size_t j = 0; j < centers[j].size(); ++j)
      printf("%lf,", centers[i][j]);
    printf("\n");
  }

  return 0;
}

#endif // unit

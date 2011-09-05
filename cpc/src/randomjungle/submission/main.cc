#include <stdio.h>
#include <vector>
#include <algorithm>
#include "readlines.hh"

struct fu
{
  std::vector<unsigned int>& _amounts;

  fu(std::vector<unsigned int>& amounts) : _amounts(amounts) {}

  bool operator()(unsigned int i, unsigned int j)
  {
    return _amounts[i] > _amounts[j];
  }
};

int main(int ac, char** av)
{
  static const unsigned int first_row = 13184291;

  std::vector<unsigned int> amounts;
  readlines("amount.csv", amounts);

  std::vector<unsigned int> rows;
  rows.resize(amounts.size());
  for (unsigned int i = 0; i < amounts.size(); ++i) rows[i] = i;

  std::sort(rows.begin(), rows.end(), fu(amounts));
  for (unsigned int i = 0; i < rows.size(); ++i)
    printf("%u\n", first_row + rows[i]);

  return 0;
}

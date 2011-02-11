static inline unsigned int double_to_uint(double d)
{
  return (unsigned int)d;
}

double compute_auc(const double* sub, const double* sol, unsigned int size)
{
  double auc = 0.f;

  unsigned int is_next_same;
  unsigned int i;

  double totals[2] = { 0, 0 };
  double this_percents[2] = { 0, 0 };
  double last_percents[2] = { 0, 0 };
  double counts[2] = { 0, 0 };

  /* foreach solution, count 0 and 1s */
  for (i = 0; i < size; ++i)
    ++totals[double_to_uint(sol[i])];

  /* foreach submission */
  is_next_same = 0;
  for (i = 0; i < size; ++i)
  {
    if (is_next_same == 0)
    {
      last_percents[0] = this_percents[0];
      last_percents[1] = this_percents[1];
    }

    ++counts[double_to_uint(sol[i])];

    if ((i < (size - 1)) && (sub[i] == sub[i + 1]))
    {
      is_next_same = 1;
    }
    else /* next not the same */
    {
      double dif, tri, rec;

      is_next_same = 0;

      this_percents[0] = counts[0] / totals[0];
      this_percents[1] = counts[1] / totals[1];

      dif = this_percents[0] - last_percents[0];
      tri = dif * (this_percents[1] - last_percents[1]) * .5;
      rec = dif * last_percents[1];

      auc += rec + tri;
    }
  }

  return auc;
}


#if 0 /* unit */

#include <stdio.h>

int main(int ac, char** av)
{
  static const double sub[] = { 1, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0 };
  static const double sol[] = { 1, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0 };

  static const unsigned int size = sizeof(sub) / sizeof(sub[0]);

  printf("%lf\n", compute_auc(sub, sol, size));

  return 0;
}

#endif /* unit */

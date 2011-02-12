#if 0

static inline void swap_doubles(double* a, double* b)
{
  const double tmp = *a;
  *a = *b;
  *b = tmp;
}

static void multisort_desc(double* a, double* b, size_t size)
{
  size_t i, j, k;

  if (size == 1) return ;

  for (i = 0; i < (size - 1); ++i)
  {
    double hi = a[i];
    k = i;

    /* look for the highest one in ]i, size[ */
    for (j = i + 1; j < size; ++j)
      if (hi < a[j])
      {
	hi = a[j];
	k = j;
      }

    /* swap in a, b */
    if (k != i)
    {
      swap_doubles(a + i, a + k);
      swap_doubles(b + i, b + k);
    }
  }
}

#else /* use qsort, not thread safe */

#include <stdlib.h>
#include <string.h>

static const double* aptr;

static int cmpdesc_func(const void *i, const void *j)
{
  const double a = aptr[*(const size_t*)i];
  const double b = aptr[*(const size_t*)j];
  return (int)-(a - b);
}

static int multisort_desc(double* a, double* b, size_t size)
{
  int error = -1;

  size_t* keys = NULL;
  double* atmp = NULL;
  double* btmp = NULL;

  size_t i;

  keys = (size_t*)malloc(size * sizeof(size_t));
  if (keys == NULL) goto on_error;
  for (i = 0; i < size; ++i) keys[i] = i;

  atmp = (double*)malloc(size * sizeof(double));
  if (atmp == NULL) goto on_error;
  memcpy(atmp, a, size * sizeof(double));

  btmp = (double*)malloc(size * sizeof(double));
  if (btmp == NULL) goto on_error;
  memcpy(btmp, b, size * sizeof(double));

  aptr = a;
  qsort(keys, size, sizeof(size_t), cmpdesc_func);

  for (i = 0; i < size; ++i)
  {
    const size_t k = keys[i];
    a[i] = atmp[k];
    b[i] = btmp[k];
  }

  if (atmp != NULL) free(atmp);
  if (btmp != NULL) free(btmp);
  if (keys != NULL) free(keys);

  error = 0;

 on_error:
  return error;
}

#endif

static inline unsigned int double_to_uint(double d)
{
  return (unsigned int)d;
}

double compute_auc(double* sub, double* sol, unsigned int size)
{
  double auc = 0.f;

  unsigned int is_next_same;
  unsigned int i;

  double totals[2] = { 0, 0 };
  double this_percents[2] = { 0, 0 };
  double last_percents[2] = { 0, 0 };
  double counts[2] = { 0, 0 };

  /* sort the solutions */
  multisort_desc(sub, sol, size);

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
      tri = dif * (this_percents[1] - last_percents[1]) / 2.;
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
  static double sub[] = { 1, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0 };
  static double sol[] = { 1, 0, 1, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0 };

  static const unsigned int size = sizeof(sub) / sizeof(sub[0]);

  printf("%lf\n", compute_auc(sub, sol, size));

  return 0;
}

#endif /* unit */

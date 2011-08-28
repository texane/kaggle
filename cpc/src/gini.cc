#include <vector>
#include <algorithm>

// http://www.kaggle.com/c/ClaimPredictionChallenge/Details/Evaluation
// When you submit an entry, the observations are sorted from "largest prediction"
// to "smallest prediction". This is the only step where your predictions come into
// play, so only the order determined by your predictions matters. Visualize the
// observations arranged from left to right, with the largest predictions on the
// left. We then move from left to right, asking "In the leftmost x% of the data,
// how much of the actual observed loss have you accumulated?" With no model, you
// can expect to accumulate 10% of the loss in 10% of the predictions, so no model
// (or a "null" model) achieves a straight line. We call the area between your curve
// and this straight line the Gini coefficient.
// There is a maximum achievable area for a "perfect" model. We will use the normalized
// Gini coefficient by dividing the Gini coefficient of your model by the Gini coefficient
// of the perfect model.

struct K {double a, p;};

static inline bool compare_function(const K& a, const K& b)
{
  return a.p > b.p;
}

double Gini(const std::vector<double>& a, const std::vector<double>& p)
{
  // k[i] = { a[i],  p[i] };
  std::vector<K> k; k.resize(a.size());
  for (unsigned int i = 0; i < a.size(); ++i)
  { k[i].a = a[i]; k[i].p = p[i]; }

  // sort(k) by descending p
  std::stable_sort(k.begin(), k.end(), compare_function);

  double giniSum = 0;
  double totalSum = 0;

#if 1
  for (unsigned int fu = 0; fu < a.size(); ++fu)
  {
    const struct K& i = k[fu];
    totalSum += i.a;
    giniSum += totalSum;
  }
#else
  for (unsigned int i = 0; i < a.size(); ++i)
  {
    // can be written as giniSum = 1 * a0 + 2 * a1 + .. + n * an;
    totalSum += i.a;
    giniSum += (a.size() - i) * k[i].a;
  }
#endif

  // divide by the sum and substract the null model
  giniSum = giniSum / totalSum - (a.size() + 1.0) / 2;

  return giniSum / a.size();
}

double GiniNormalized(const std::vector<double> &a, const std::vector<double>& p)
{
  return Gini(a, p)/Gini(a, a);
}

#if 0 // unit

#include <stdio.h>

void GiniTest() {
  auto T = [](const std::vector<double> &a, decltype(a) p, double g, double n) {
    auto E = [](double a, double b, double e=1e-6) {return fabs(a-b) <= e;};
    if (! ( (E(Gini(a, p), g) && E(GiniNormalized(a, p), n)) ) ) printf("INVALID\n");
  };

  Gini({1, 2, 4, 3}, {0, 0, 0, 0});
  Gini({1, 2, 4, 3}, {1, 2, 4, 3});

#if 1
  T({1, 2, 3}, {10, 20, 30}, 0.111111, 1);
  T({1, 2, 3}, {30, 20, 10}, -0.111111, -1);
  T({1, 2, 3}, {0, 0, 0}, -0.111111, -1);
  T({3, 2, 1}, {0, 0, 0}, 0.111111, 1);
  T({1, 2, 4, 3}, {0, 0, 0, 0}, -0.1, -0.8);
  T({2, 1, 4, 3}, {0, 0, 2, 1}, 0.125, 1);
  T({0, 20, 40, 0, 10}, {40, 40, 10, 5, 5}, 0, 0);
  T({40, 0, 20, 0, 10}, {1000000, 40, 40, 5, 5}, 0.171428, 0.6);
  T({40, 20, 10, 0, 0}, {40, 20, 10, 0, 0}, 0.285714, 1);
  T({1, 1, 0, 1}, {0.86, 0.26, 0.52, 0.32}, -0.041666, -0.333333);
#endif
}

int main(int ac, char** av)
{
  GiniTest();
}

#endif // unit

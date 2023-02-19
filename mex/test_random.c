#include <stdio.h>
#include <stdlib.h>

#include "se_random.h"

static int generate_random_numbers(const int seed, const int n_samples,
                                   int *res)
{
  se_rng_init(seed);
  for (int i = 0; i < n_samples; i++) {
    res[i] = se_get_random_int(1, 100);
  }

  return EXIT_SUCCESS;
}

// Compare to results from matlab
static void test_reproducible(int n_trials, int n_samples)
{
  printf("Test against matlab\n");

  int samples[n_samples];
  for (int i = 1; i < (n_trials + 1); i++) {
    generate_random_numbers(i, n_samples, samples);
    printf("Results seed %d:\n", i);
    for (int j = 0; j < n_samples; j++) {
      printf("%d\t", samples[j]);
    }
    printf("\n\n");
  }
}

static void test_rngs_openmp_thread_local(int n_trials, int n_samples)
{
  printf("\n\nTest OPENMP private (Compare across runs)\n");

  int n_repeats = 3;
  int samples[n_repeats][n_trials][n_samples];

  for (int k = 0; k < n_repeats; k++) {
    #pragma omp parallel for
    for (int i = 1; i < (n_trials + 1); i++) {
      printf("thread %d\n", i);
      generate_random_numbers(i, n_samples, samples[k][i - 1]);
    }

  }

  for (int i = 1; i < (n_trials + 1); i++) {
    printf("Results seed %d:\n", i);
    for (int k = 0; k < n_repeats; k++) {
      for (int j = 0; j < n_samples; j++) {
        printf("%d\t", samples[k][i - 1][j]);
      }
      printf("\n");
    }
    printf("\n\n");
  }
}

int main()
{
  int n_trials = 10;
  int n_samples = 10;

  test_reproducible(n_trials, n_samples);
  test_rngs_openmp_thread_local(n_trials, n_samples);
  return EXIT_SUCCESS;
}

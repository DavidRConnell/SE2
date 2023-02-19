#include "se_random.h"
#include <igraph_random.h>

/* Initializes default igraph random number generator to use twister method */
void se_rng_init(const int seed)
{
  igraph_rng_t rng;
  igraph_error_t err = igraph_rng_init(&rng, &igraph_rngtype_mt19937);

  igraph_rng_set_default(&rng);
  igraph_rng_seed(igraph_rng_default(), seed);
}

/* HACK: MATLABs rng consistently skips a value produced by igraph's. So if
RNG_INTEGER is run twice for every value we want, we get reproducible results
with matlab. */
int se_get_random_int(const int l, const int h)
{
  int res = RNG_INTEGER(l, h);
  RNG_INTEGER(l, h);
  return res;
}

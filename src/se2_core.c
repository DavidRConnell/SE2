#include <igraph_error.h>
#include <igraph_random.h>
#include <igraph_interface.h>
#include <igraph_structural.h>
#include <omp.h>

#include "speak_easy_2.h"

#define SE2_SET_OPTION(opts, field, default) \
    (opts->field) = (opts->field) ? (opts->field) : (default)

static size_t default_target_clusters()
{
  // TODO: Placeholder set real defaults later
  return 20;
}

static size_t default_max_threads()
{
  size_t n_threads = 0;
  // Hack since omp_get_num_threads returns 1 outside of a parallel block
  #pragma omp parallel
  {
    #pragma omp single
    n_threads = omp_get_num_threads();
  }
  return n_threads;
}

static void se2_set_defaults(options *opts)
{
  SE2_SET_OPTION(opts, independent_runs, 10);
  SE2_SET_OPTION(opts, subcluster, 1);
  SE2_SET_OPTION(opts, multicommunity, 1);
  SE2_SET_OPTION(opts, target_partitions, 5);
  SE2_SET_OPTION(opts, target_clusters, default_target_clusters());
  SE2_SET_OPTION(opts, minclust, 5);
  SE2_SET_OPTION(opts, discard_transient, 3);
  SE2_SET_OPTION(opts, random_seed, RNG_INTEGER(1, 9999));
  SE2_SET_OPTION(opts, max_threads, default_max_threads());
  SE2_SET_OPTION(opts, node_confidence, false);
}

int speak_easy_2(igraph_t *graph, igraph_vector_t const *weights,
                 igraph_bool_t const directed, options *opts, outputs *res)
{
  printf("\ncalling main routine at level 1\n");
  se2_set_defaults(opts);

  /* se2_bootstrap(graph, weights, directed, 0, opts, res); */

  if (opts->node_confidence) {
    // pass;
  }

  for (size_t i = 1; i < opts->subcluster; i++) {
    // pass;
  }

  return EXIT_SUCCESS;
}

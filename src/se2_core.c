#include <igraph_error.h>
#include <igraph_structural.h>
#include <omp.h>

#include "speak_easy_2.h"
#include "se2_seeding.h"
#include "se2_random.h"


#define SE2_SET_OPTION(opts, field, default) \
    (opts->field) = (opts->field) ? (opts->field) : (default)

static void se2_core(igraph_t *graph, igraph_vector_t const ic_store,
                     size_t const subcluster_iter, options const *opts, outputs *res);

static void se2_bootstrap(igraph_t *graph, igraph_vector_t const *weights,
                          igraph_bool_t const directed, size_t const subcluster_iter,
                          options const *opts, outputs *res)
{
  igraph_integer_t n_nodes = igraph_vcount(graph);
  igraph_vector_t kin;

  igraph_vector_init(&kin, n_nodes);
  igraph_strength(graph, &kin, igraph_vss_all(), IGRAPH_IN, IGRAPH_NO_LOOPS,
                  weights);

  if ((!subcluster_iter) && (opts->max_threads > 1)) {
    puts("starting level 1 clustering; independent runs might not be displayed in order - that is okay");
  }

  if ((!subcluster_iter) && (opts->multicommunity > 1)) {
    puts("attempting overlapping clustering");
  }

  #pragma omp parallel for
  for (igraph_integer_t run_i = 0; run_i < opts->independent_runs; run_i++) {
    igraph_vector_int_t ic_store;
    igraph_vector_int_init(&ic_store, n_nodes);

    se2_rng_init(run_i + opts->random_seed);
    size_t n_unique = se2_seeding(graph, weights, directed, &kin, opts,
                                  &ic_store);

    if ((!subcluster_iter) && (run_i == 0)) {
      printf("produced about %zu seed labels, while goal was %zu\n", n_unique,
             opts->target_clusters);
      puts("completed generating initial labels");
    }

    if (!subcluster_iter) {
      printf("\nstarting independent run # %zu of %zu\n", run_i + 1,
             opts->independent_runs);
    }

    /* se2_core(graph, ic_store, subcluster_iter, kin, opts, res); */
    igraph_vector_int_destroy(&ic_store);
  }

  igraph_vector_destroy(&kin);
}

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
  SE2_SET_OPTION(opts, random_seed, (size_t)RNG_INTEGER(1, 9999));
  SE2_SET_OPTION(opts, max_threads, default_max_threads());
  SE2_SET_OPTION(opts, node_confidence, false);
}

int speak_easy_2(igraph_t *graph, igraph_vector_t const *weights,
                 igraph_bool_t const directed, options *opts, outputs *res)
{
  printf("\ncalling main routine at level 1\n");
  se2_set_defaults(opts);

  se2_bootstrap(graph, weights, directed, 0, opts, res);

  if (opts->node_confidence) {
    // pass;
  }

  for (size_t i = 1; i < opts->subcluster; i++) {
    // pass;
  }

  return EXIT_SUCCESS;
}

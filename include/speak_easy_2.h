#ifndef SPEAK_EASY_H
#define SPEAK_EASY_H

#include <igraph_datatype.h>

typedef struct {
  double *partition_ids;
  double *secondary_labels_scores;
  double *secondary_labels_ids;
  double max_labels_output;
} outputs;

typedef struct {
  size_t independent_runs;  // Number of independent runs to perform.
  size_t subcluster;        // Depth of clustering.
  size_t multicommunity;    // Max number of communities a node can be a member of.
  size_t target_partitions; // Number of partitions to find per independent run.
  size_t target_clusters;   // Expected number of clusters to find.
  size_t minclust;          // Minimum cluster size to subclustering.
  size_t discard_transient; // How many initial partitions to discard before recording.
  size_t random_seed;       // Seed for reproducing results.
  size_t max_threads;       // Number of threads to use.
  bool node_confidence;
} options;

int speak_easy_2(igraph_t *graph, igraph_vector_t *weights,
                 options *opts, igraph_vector_int_t *res);

#endif

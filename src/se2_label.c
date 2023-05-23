#include <igraph_structural.h>
#include "se2_label.h"

static inline void global_label_proportions(igraph_t const *graph,
    igraph_vector_t const *weights, se2_partition const *partition,
    igraph_vector_t *labels_heard, igraph_integer_t const n_labels)
{
  igraph_vector_t kout;
  igraph_integer_t n_nodes = igraph_vcount(graph);
  igraph_integer_t acc = 0;

  igraph_vector_init(&kout, n_nodes);
  igraph_strength(graph, &kout, igraph_vss_all(), IGRAPH_OUT, IGRAPH_NO_LOOPS,
                  weights);

  for (igraph_integer_t i = 0; i < n_nodes; i++) {
    VECTOR(*labels_heard)[LABEL(*partition)[i]] += VECTOR(kout)[i];
  }
  igraph_vector_destroy(&kout);

  for (igraph_integer_t i = 0; i < n_labels; i++) {
    acc += VECTOR(*labels_heard)[i];
  }

  for (igraph_integer_t i = 0; i < n_labels; i++) {
    VECTOR(*labels_heard)[i] /= acc;
  }
}

static inline igraph_real_t edge_get_weight(igraph_t const *graph,
    igraph_vector_t const *weights, igraph_integer_t const from,
    igraph_integer_t const to, igraph_bool_t const directed)
{
  if (!weights) {
    return 1;
  }

  igraph_integer_t eid;
  igraph_get_eid(graph, &eid, from, to, directed, false);
  return VECTOR(*weights)[eid];
}

static inline void local_label_proportions(igraph_t const *graph,
    igraph_vector_t const *weights, se2_partition const *partition,
    igraph_integer_t const node_id, igraph_vector_t *labels_heard,
    igraph_real_t *kin, igraph_integer_t n_labels)
{
  igraph_vector_int_t neighbors;
  igraph_integer_t n_neighbors;
  igraph_integer_t neighbor;
  igraph_real_t weight;
  igraph_bool_t directed = igraph_is_directed(graph);

  igraph_vector_int_init(&neighbors, 0);
  igraph_neighbors(graph, &neighbors, node_id, IGRAPH_IN);
  n_neighbors = igraph_vector_int_size(&neighbors);
  for (igraph_integer_t i = 0; i < n_neighbors; i++) {
    neighbor = VECTOR(neighbors)[i];
    weight = edge_get_weight(graph, weights, neighbor, node_id, directed);
    VECTOR(*labels_heard)[LABEL(*partition)[neighbor]] += weight;
  }
  igraph_vector_int_destroy(&neighbors);

  *kin = 0;
  for (igraph_integer_t i = 0; i < n_labels; i++) {
    *kin += VECTOR(*labels_heard)[i];
  }
}

/* Scores labels based on the difference between the local and global
 frequencies.  Labels that are overrepresented locally are likely to be of
 importance in tagging a node. */
static void se2_find_most_specific_labels_i(igraph_t const *graph,
    igraph_vector_t const *weights,
    se2_partition *partition,
    se2_iterator *node_iter)
{
  igraph_integer_t max_label = se2_partition_max_label(partition);
  igraph_vector_t labels_expected;
  igraph_vector_t labels_observed;
  se2_iterator *label_iter = se2_iterator_random_label_init(partition, false);
  igraph_real_t node_kin = 0;
  igraph_real_t label_specificity = 0, best_label_specificity = 0;
  igraph_integer_t best_label = -1;

  igraph_vector_init(&labels_expected, max_label + 1);
  igraph_vector_init(&labels_observed, max_label + 1);

  global_label_proportions(graph, weights, partition, &labels_expected,
                           max_label + 1);

  igraph_integer_t node_id = 0, label_id = 0;
  while ((node_id = se2_iterator_next(node_iter)) != -1) {
    local_label_proportions(graph, weights, partition, node_id,
                            &labels_observed, &node_kin, max_label + 1);
    while ((label_id = se2_iterator_next(label_iter)) != -1) {
      label_specificity = VECTOR(labels_observed)[label_id] -
                          (node_kin * VECTOR(labels_expected)[label_id]);
      if ((best_label == -1) || (label_specificity >= best_label_specificity)) {
        best_label_specificity = label_specificity;
        best_label = label_id;
      }
    }
    se2_partition_add_to_stage(partition, node_id, best_label,
                               best_label_specificity);
    best_label = -1;
    se2_iterator_shuffle(label_iter);
  }
  se2_partition_commit_changes(partition);

  se2_iterator_destroy(label_iter);
  igraph_vector_destroy(&labels_expected);
  igraph_vector_destroy(&labels_observed);
}

void se2_find_most_specific_labels(igraph_t const *graph,
                                   igraph_vector_t const *weights,
                                   se2_partition *partition,
                                   igraph_real_t const fraction_nodes_to_label)
{
  se2_iterator *node_iter = se2_iterator_random_node_init(partition,
                            fraction_nodes_to_label);
  se2_find_most_specific_labels_i(graph, weights, partition, node_iter);
  se2_iterator_destroy(node_iter);
}

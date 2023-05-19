#ifndef SE2_PARTITIONS_H
#define SE2_PARTITIONS_H

#include <igraph_interface.h>
#include <igraph_vector_list.h>

// LABEL(partition)[node_id] gets the reference label for the node.
#define LABEL(partition) (VECTOR(*(partition).reference))

/* WARNING: Expose fields only so macros can be used for performance.  Ideally,
treated as opaque.

Basic idea is there is a reference membership vector, which stores the labels
of nodes from before the current time step, and a stage membership vector,
where proposed labels are set. At the end of a time step, the staged changes
are committed to the reference membership vectore. */
typedef struct se2_partition {
  igraph_vector_int_t *stage;           // Working membership
  igraph_vector_int_t *reference;       // Fixed previous membership
  igraph_vector_bool_t *label_mask;     // Mask of which labels are in use.
  igraph_vector_t *label_quality;       // Label quality for stage.
  // For node iterators, possibly break into own structure and create iterators
  // similar to igraph.
  igraph_integer_t n_nodes;
  igraph_integer_t n_nodes_to_update;
  igraph_integer_t node_pos;            // Node index for iterator
  igraph_vector_int_t *node_ids;        // Node indices for shuffling
  // For label iterators, same as with node iterator.
  igraph_integer_t n_labels;
  igraph_integer_t label_pos;
  igraph_integer_t max_label;
} se2_partition;

se2_partition *se2_partition_init(igraph_t const *graph,
                                  igraph_vector_int_t *initial_labels);
void se2_partition_destroy(se2_partition *partition);
void se2_partition_store(se2_partition const *working_partition,
                         igraph_vector_int_list_t *partition_store,
                         igraph_integer_t const index);
igraph_integer_t se2_next_node(se2_partition *partition);
igraph_integer_t se2_next_label(se2_partition *partition);
igraph_integer_t se2_partition_n_labels(se2_partition *partition);
igraph_integer_t se2_partition_max_label(se2_partition *partition);
void se2_partition_repack_labels(se2_partition *parititon);
void se2_partition_shuffle(se2_partition *partition);
void se2_partition_merge_labels(se2_partition *partition, igraph_integer_t c1,
                                igraph_integer_t c2);
void se2_partition_relabel_mask(se2_partition *partition,
                                igraph_vector_bool_t const *mask);
void se2_partition_add_to_stage(se2_partition *partition,
                                igraph_integer_t const node_id, igraph_integer_t const label,
                                igraph_real_t specificity);
void se2_partition_commit_changes(se2_partition *partition);

#endif

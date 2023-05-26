#include <string.h>
#include <stdlib.h>

#include "se2_partitions.h"
#include "se2_random.h"

struct se2_iterator {
  igraph_vector_int_t *ids;
  igraph_integer_t pos;
  igraph_integer_t n_total;
  igraph_integer_t n_iter;
  igraph_bool_t owns_ids;
};

static igraph_integer_t se2_detect_labels(igraph_vector_int_t *membership,
    igraph_vector_bool_t *mask)
{
  igraph_integer_t max_label = igraph_vector_int_max(membership);
  igraph_integer_t n_labels = 0;
  igraph_integer_t n_nodes = igraph_vector_int_size(membership);

  igraph_vector_bool_resize(mask, max_label + 1);
  igraph_vector_bool_null(mask);
  for (igraph_integer_t i = 0; i < n_nodes; i++) {
    VECTOR(*mask)[VECTOR(*membership)[i]] = true;
  }

  for (igraph_integer_t i = 0; i <= max_label; i++) {
    n_labels += VECTOR(*mask)[i];
  }

  return n_labels;
}

se2_partition *se2_partition_init(igraph_t const *graph,
                                  igraph_vector_int_t *initial_labels)
{
  se2_partition *partition = malloc(sizeof(*partition));
  igraph_integer_t n_nodes = igraph_vcount(graph);
  igraph_vector_t *specificity = malloc(sizeof(*specificity));
  igraph_vector_int_t *stage = malloc(sizeof(*specificity));
  igraph_vector_bool_t *label_mask = malloc(sizeof(*label_mask));
  igraph_integer_t n_labels = 0;

  igraph_vector_init(specificity, n_nodes);
  igraph_vector_int_init(stage, n_nodes);
  igraph_vector_bool_init(label_mask, 0);
  n_labels = se2_detect_labels(initial_labels, label_mask);

  se2_partition new_partition = {
    .n_nodes = n_nodes,
    .reference = initial_labels,
    .stage = stage,
    .label_quality = specificity,
    .label_mask = label_mask,
    .n_labels = n_labels,
    .max_label = igraph_vector_bool_size(label_mask) - 1,
  };

  memcpy(partition, &new_partition, sizeof(new_partition));
  return partition;
}

void se2_partition_destroy(se2_partition *partition)
{
  igraph_vector_int_destroy(partition->stage);
  igraph_vector_destroy(partition->label_quality);
  igraph_vector_bool_destroy(partition->label_mask);
  free(partition->stage);
  free(partition->label_quality);
  free(partition->label_mask);
  free(partition);
}

void se2_iterator_shuffle(se2_iterator *iterator)
{
  iterator->pos = 0;
  se2_randperm(iterator->ids, iterator->n_total,
               iterator->n_iter);
}

void se2_iterator_reset(se2_iterator *iterator)
{
  iterator->pos = 0;
}

// WARNING: Iterator does not take ownership of the id vector so it must still
// be cleaned up by the caller.
se2_iterator *se2_iterator_from_vector(igraph_vector_int_t *ids,
                                       igraph_integer_t const n_iter)
{
  igraph_integer_t n = igraph_vector_int_size(ids);
  se2_iterator *iterator = malloc(sizeof(*iterator));
  se2_iterator new_iterator = {
    .ids = ids,
    .n_total = n,
    .n_iter = n_iter,
    .pos = 0,
    .owns_ids = false
  };

  memcpy(iterator, &new_iterator, sizeof(new_iterator));
  return iterator;
}

se2_iterator *se2_iterator_random_node_init(se2_partition const *partition,
    igraph_real_t const proportion)
{
  igraph_integer_t n_total = partition->n_nodes;
  igraph_integer_t n_iter = n_total;
  igraph_vector_int_t *nodes = malloc(sizeof(*nodes));

  igraph_vector_int_init(nodes, n_total);
  for (igraph_integer_t i = 0; i < n_total; i++) {
    VECTOR(*nodes)[i] = i;
  }

  if (proportion) {
    n_iter = n_total * proportion;
  }

  se2_iterator *iterator = se2_iterator_from_vector(nodes, n_iter);
  iterator->owns_ids = true;
  se2_iterator_shuffle(iterator);

  return iterator;
}

se2_iterator *se2_iterator_random_label_init(se2_partition const *partition,
    igraph_real_t const proportion)
{
  igraph_integer_t n_total = partition->n_labels;
  igraph_integer_t n_iter = n_total;
  igraph_vector_int_t *labels = malloc(sizeof(*labels));

  igraph_vector_int_init(labels, n_total);
  for (igraph_integer_t i = 0, j = 0; i < n_total; j++) {
    if (VECTOR(*(partition->label_mask))[j]) {
      VECTOR(*labels)[i] = j;
      i++;
    }
  }

  if (proportion) {
    n_iter = n_total * proportion;
  }

  se2_iterator *iterator = se2_iterator_from_vector(labels, n_iter);
  iterator->owns_ids = true;
  se2_iterator_shuffle(iterator);

  return iterator;
}

static inline void k_smallest_swap_i(igraph_vector_int_t *idx,
                                     igraph_integer_t const i,
                                     igraph_integer_t j)
{
  igraph_integer_t swap = VECTOR(*idx)[i];
  VECTOR(*idx)[i] = VECTOR(*idx)[j];
  VECTOR(*idx)[j] = swap;
}

static void k_smallest_i(igraph_vector_t const *arr,
                         igraph_integer_t const n,
                         igraph_vector_int_t *idx,
                         igraph_integer_t const k)
{
  igraph_integer_t pivot = 0;
  igraph_integer_t max = VECTOR(*arr)[pivot];

  for (igraph_integer_t i = 0; i < k; i++) {
    if (VECTOR(*arr)[i] > max) {
      max = VECTOR(*arr)[i];
      pivot = i;
    }
  }
  k_smallest_swap_i(idx, k - 1, pivot);
  pivot = k - 1;

  for (igraph_integer_t i = k; i < n; i++) {
    if (VECTOR(*arr)[VECTOR(*idx)[pivot]] > VECTOR(*arr)[VECTOR(*idx)[i]]) {
      k_smallest_swap_i(idx, i, pivot);
      pivot = i;
    }
  }

  if (pivot != (k - 1)) {
    k_smallest_i(arr, pivot, idx, k);
  }
}

se2_iterator *se2_iterator_k_worst_fit_nodes_init(
  se2_partition const *partition, igraph_integer_t const k)
{
  igraph_vector_int_t *ids = malloc(sizeof(*ids));

  igraph_vector_int_init(ids, partition->n_nodes);
  for (igraph_integer_t i = 0; i < partition->n_nodes; i++) {
    VECTOR(*ids)[i] = i;
  }

  k_smallest_i(partition->label_quality, partition->n_nodes, ids, k);

  igraph_vector_int_resize(ids, k);

  se2_iterator *iterator = se2_iterator_from_vector(ids, k);
  iterator->owns_ids = true;
  se2_iterator_shuffle(iterator);

  return iterator;
}

void se2_iterator_destroy(se2_iterator *iterator)
{
  if (iterator->owns_ids) {
    igraph_vector_int_destroy(iterator->ids);
    free(iterator->ids);
  }
  free(iterator);
}

igraph_integer_t se2_iterator_next(se2_iterator *iterator)
{
  igraph_integer_t n = 0;
  if (iterator->pos == iterator->n_iter) {
    iterator->pos = 0;
    return -1;
  }

  n = VECTOR(*iterator->ids)[iterator->pos];
  iterator->pos++;

  return n;
}

igraph_integer_t se2_partition_n_nodes(se2_partition const *partition)
{
  return partition->n_nodes;
}

igraph_integer_t se2_partition_n_labels(se2_partition const *partition)
{
  return partition->n_labels;
}

igraph_integer_t se2_partition_max_label(se2_partition const *partition)
{
  return partition->max_label;
}

void se2_partition_add_to_stage(se2_partition *partition,
                                igraph_integer_t const node_id,
                                igraph_integer_t const label,
                                igraph_real_t specificity)
{
  VECTOR(*partition->stage)[node_id] = label;
  VECTOR(*partition->label_quality)[node_id] = specificity;
}

// Return an unused label.
static igraph_integer_t se2_new_label(se2_partition *partition)
{
  igraph_integer_t mask_length = igraph_vector_bool_size(partition->label_mask);
  igraph_integer_t next_label = 0;
  while ((partition->label_mask) && (next_label < mask_length)) {
    next_label++;
  }

  if (next_label == mask_length) {
    igraph_vector_bool_resize(partition->label_mask, 2 * mask_length);
  }

  if (next_label > partition->max_label) {
    partition->max_label = next_label;
  }

  partition->n_labels++;

  return next_label;
}

static inline igraph_integer_t se2_partition_community_size(
  se2_partition const *partition, igraph_integer_t const label)
{
  igraph_integer_t sz = 0;
  for (igraph_integer_t i = 0; i < partition->n_nodes; i++) {
    sz += VECTOR(*partition->reference)[i] == label;
  }

  return sz;
}

static inline void se2_partition_free_label(se2_partition *partition,
    igraph_integer_t const label)
{
  VECTOR(*partition->label_mask)[label] = false;
  if (label == partition->max_label) {
    while ((!VECTOR(*partition->label_mask)[partition->max_label]) &&
           (partition->max_label > 0)) {
      partition->max_label--;
    }
  }

  partition->n_labels--;
}

void se2_partition_merge_labels(se2_partition *partition, igraph_integer_t c1,
                                igraph_integer_t c2)
{
  // Ensure smaller community engulfed by larger community. Not necessary.
  if (se2_partition_community_size(partition, c2) >
      se2_partition_community_size(partition, c1)) {
    igraph_integer_t swp = c1;
    c1 = c2;
    c2 = swp;
  }

  for (igraph_integer_t i = 0; i < partition->n_nodes; i++) {
    if (VECTOR(*partition->stage)[i] == c2) {
      VECTOR(*partition->stage)[i] = c1;
    }
  }

  se2_partition_free_label(partition, c2);
}

// Move nodes in mask to new label.
void se2_partition_relabel_mask(se2_partition *partition,
                                igraph_vector_bool_t const *mask)
{
  igraph_integer_t label = se2_new_label(partition);
  for (igraph_integer_t i = 0; i < partition->n_nodes; i++) {
    if (VECTOR(*mask)[i]) {
      VECTOR(*partition->stage)[i] = label;
    }
  }
}

static void se2_partition_update_label_mask(se2_partition *partition)
{
  partition->n_labels = se2_detect_labels(partition->reference,
                                          partition->label_mask);
  partition->max_label = igraph_vector_bool_size(partition->label_mask) - 1;
}

void se2_partition_commit_changes(se2_partition *partition)
{
  igraph_vector_int_update(partition->reference, partition->stage);
  se2_partition_update_label_mask(partition);
}

/* Save the state of the current working partition's committed changes to the
partition store.

NOTE: This saves only the membership ids for each node so it goes from a
se2_partition to an igraph vector despite both arguments being
"partitions". */
void se2_partition_store(se2_partition const *working_partition,
                         igraph_vector_int_list_t *partition_store,
                         igraph_integer_t const idx)
{
  igraph_vector_int_t *partition_state = igraph_vector_int_list_get_ptr(
      partition_store, idx);
  igraph_vector_int_update(partition_state, working_partition->reference);
}

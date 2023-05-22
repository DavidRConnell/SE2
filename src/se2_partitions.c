#include <string.h>
#include <stdlib.h>

#include "se2_partitions.h"
#include "se2_random.h"

#define TO_UPDATE_FRAC 0.9

static void se2_collect_all_nodes(igraph_t const *graph,
                                  igraph_vector_int_t *nodes)
{
  igraph_vit_t vit;
  igraph_vit_create(graph, igraph_vss_all(), &vit);
  for (igraph_integer_t i = 0; !IGRAPH_VIT_END(vit);
       IGRAPH_VIT_NEXT(vit), i++) {
    VECTOR(*nodes)[i] = IGRAPH_VIT_GET(vit);
  }
  igraph_vit_destroy(&vit);
}

static igraph_integer_t se2_detect_labels(igraph_vector_int_t *labels,
    igraph_vector_bool_t *mask)
{
  igraph_integer_t max_label = igraph_vector_int_max(labels);
  igraph_integer_t n_labels = 0;
  igraph_integer_t n_nodes = igraph_vector_int_size(labels);

  igraph_vector_bool_resize(mask, max_label + 1);
  for (igraph_integer_t i = 0; i < n_nodes; i++) {
    VECTOR(*mask)[VECTOR(*labels)[i]] = true;
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
  igraph_vector_int_t *nodes = malloc(sizeof(*nodes));
  igraph_integer_t n_nodes = igraph_vcount(graph);
  igraph_vector_t *specificity = malloc(sizeof(*specificity));
  igraph_vector_int_t *stage = malloc(sizeof(*specificity));
  igraph_vector_bool_t *label_mask = malloc(sizeof(*label_mask));
  igraph_integer_t n_labels = 0;

  igraph_vector_int_init(nodes, n_nodes);
  se2_collect_all_nodes(graph, nodes);
  igraph_vector_init(specificity, n_nodes);
  igraph_vector_int_init(stage, n_nodes);
  igraph_vector_bool_init(label_mask, 0);
  n_labels = se2_detect_labels(initial_labels, label_mask);
  se2_partition new_partition = {
    .n_nodes = n_nodes,
    .n_nodes_to_update = ceil((double)(n_nodes * TO_UPDATE_FRAC)),
    .node_ids = nodes,
    .node_pos = 0,
    .reference = initial_labels,
    .stage = stage,
    .label_quality = specificity,
    .label_mask = label_mask,
    .n_labels = n_labels,
    .label_pos = 0,
    .max_label = igraph_vector_bool_size(label_mask),
  };

  memcpy(partition, &new_partition, sizeof(new_partition));
  return partition;
}

void se2_partition_destroy(se2_partition *partition)
{
  igraph_vector_int_destroy(partition->node_ids);
  igraph_vector_int_destroy(partition->stage);
  igraph_vector_destroy(partition->label_quality);
  igraph_vector_bool_destroy(partition->label_mask);
  free(partition->node_ids);
  free(partition->stage);
  free(partition->label_quality);
  free(partition->label_mask);
  free(partition);
}

igraph_integer_t se2_next_node(se2_partition *partition)
{
  igraph_integer_t n = 0;
  if (partition->node_pos == partition->n_nodes_to_update) {
    partition->node_pos = 0;
    return -1;
  }

  n = VECTOR(*partition->node_ids)[partition->node_pos];
  partition->node_pos++;

  return n;
}

igraph_integer_t se2_next_label(se2_partition *partition)
{
  igraph_integer_t l = 0;
  while ((partition->label_pos <= partition->max_label) &&
         !(l = VECTOR(*partition->label_mask)[partition->label_pos])) {
    partition->label_pos++;
  }

  if (partition->label_pos > partition->max_label) {
    partition->label_pos = 0;
    return -1;
  }

  l = partition->label_pos;
  partition->label_pos++;

  return l;
}

igraph_integer_t se2_partition_n_labels(se2_partition *partition)
{
  return partition->n_labels;
}

igraph_integer_t se2_partition_max_label(se2_partition *partition)
{
  return partition->max_label;
}

// WARNING: Not tested.
void se2_partition_repack_labels(se2_partition *partition)
{
  igraph_integer_t biggest_label = igraph_vector_int_max(partition->reference);
  igraph_vector_int_t tag_shift;

  igraph_vector_int_init(&tag_shift, biggest_label + 1);
  igraph_vector_bool_resize(partition->label_mask, biggest_label + 1);
  igraph_vector_bool_null(partition->label_mask);

  for (igraph_integer_t i = 0; i < partition->n_nodes; i++) {
    VECTOR(*partition->label_mask)[VECTOR(*partition->reference)[i]] = true;
  }

  partition->n_labels = 0;
  for (igraph_integer_t i = 0; i <= biggest_label; i++) {
    partition->n_labels += VECTOR(*partition->label_mask)[i];
  }

  igraph_integer_t n_empty_tags = 0;
  for (igraph_integer_t i = 0; i <= biggest_label; i++) {
    if (!VECTOR(*partition->label_mask)[i]) {
      n_empty_tags++;
    } else {
      VECTOR(tag_shift)[i] = n_empty_tags;
    }
  }

  partition->max_label = biggest_label - VECTOR(tag_shift)[biggest_label] + 1;
  igraph_integer_t label;
  for (igraph_integer_t i = 0; i < partition->n_nodes; i++) {
    label = VECTOR(*partition->reference)[i];
    VECTOR(*partition->reference)[i] -= VECTOR(tag_shift)[label];
  }

  for (igraph_integer_t i = 0; i <= biggest_label; i++) {
    VECTOR(*partition->label_mask)[i] = i < partition->n_labels;
  }

  igraph_vector_int_destroy(&tag_shift);
}

void se2_partition_add_to_stage(se2_partition *partition,
                                igraph_integer_t const node_id, igraph_integer_t const label,
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

void se2_partition_shuffle(se2_partition *partition)
{
  partition->node_pos = 0;
  partition->label_pos = 0;
  se2_randperm(partition->node_ids, partition->n_nodes,
               partition->n_nodes_to_update);
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

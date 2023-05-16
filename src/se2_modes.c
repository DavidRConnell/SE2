#include <stdlib.h>
#include <string.h>

#include "se2_modes.h"
#include "se2_random.h"

#define TO_UPDATE_FRAC 0.9

typedef enum {
  SE2_TYPICAL = 0,
  SE2_BUBBLE,
  SE2_MERGE,
  SE2_NURTURE,
  SE2_NUM_MODES
} se2_mode;

struct se2_partition {
  igraph_integer_t n_nodes;
  igraph_integer_t n_nodes_to_update;
  igraph_vector_int_t *node_ids;
  igraph_vector_int_t *membership;
};

struct se2_tracker {
  se2_mode mode;
  igraph_bool_t allowed_to_merge;
  igraph_integer_t *time_since_last;
  igraph_integer_t *n_times;
  igraph_integer_t n_clusters_merged;
  igraph_integer_t postintervention_count;
  igraph_integer_t n_partitions;
  igraph_bool_t intervention_event;
};

se2_tracker *se2_tracker_init(options const *opts)
{
  se2_tracker *tracker = malloc(sizeof(*tracker));

  igraph_integer_t *time_since_mode_tracker = calloc(SE2_NUM_MODES,
      sizeof(*time_since_mode_tracker));
  igraph_integer_t *n_times_mode_ran_tracker = calloc(SE2_NUM_MODES,
      sizeof(*time_since_mode_tracker));

  se2_tracker new_tracker = {
    .mode = SE2_TYPICAL,
    .allowed_to_merge = false,
    .time_since_last = time_since_mode_tracker,
    .n_times = n_times_mode_ran_tracker,
    .n_clusters_merged = 0,
    .postintervention_count = -(opts->discard_transient),
    .n_partitions = opts->target_partitions - 1,
    .intervention_event = false,
  };

  memcpy(tracker, &new_tracker, sizeof(new_tracker));

  return tracker;
}

void se2_tracker_destroy(se2_tracker *tracker)
{
  free(tracker->n_times);
  free(tracker->time_since_last);
  free(tracker);
}

static void se2_collect_all_nodes(igraph_t const *graph,
                                  igraph_vector_int_t *nodes,
                                  igraph_integer_t const n_nodes)
{
  igraph_vs_t vs;
  igraph_vit_t vit;
  igraph_vs_all(&vs);
  igraph_vit_create(graph, vs, &vit);
  for (igraph_integer_t i = 0; !IGRAPH_VIT_END(vit);
       IGRAPH_VIT_NEXT(vit), i++) {
    VECTOR(*nodes)[i] = IGRAPH_VIT_GET(vit);
  }
  igraph_vs_destroy(&vs);
  igraph_vit_destroy(&vit);
}

se2_partition *se2_partition_init(igraph_t const *graph,
                                  igraph_vector_int_t *initial_labels)
{
  se2_partition *partition = malloc(sizeof(*partition));

  igraph_vector_int_t *nodes = malloc(sizeof(*nodes));
  igraph_integer_t n_nodes = igraph_vcount(graph);
  igraph_vector_int_init(nodes, n_nodes);
  se2_collect_all_nodes(graph, nodes, n_nodes);
  se2_partition new_partition = {
    .n_nodes = n_nodes,
    .n_nodes_to_update = ceil((double)(n_nodes * TO_UPDATE_FRAC)),
    .node_ids = nodes,
    .membership = initial_labels,
  };

  memcpy(partition, &new_partition, sizeof(new_partition));
  return partition;
}

void se2_partition_destroy(se2_partition *partition)
{
  igraph_vector_int_destroy(partition->node_ids);
  free(partition->node_ids);
  free(partition);
}

/* Save the state of the current working partition to the partition store.

   NOTE: This saves only the membership ids for each node so it goes from a
   se2_partition to an igraph vector despite both arguments being
   "partitions". */
void se2_store_partition(se2_partition const *working_partition,
                         igraph_vector_int_list_t *partition_store,
                         igraph_integer_t const idx)
{
  igraph_vector_int_t *partition_state = igraph_vector_int_list_get_ptr(
      partition_store, idx);
  igraph_vector_int_update(partition_state, working_partition->membership);
}

igraph_bool_t se2_do_terminate(se2_tracker *tracker)
{
  // TODO Temporary early termination
  if (tracker->n_times[SE2_TYPICAL] == 20) {
    return true;
  }

  return tracker->postintervention_count >= tracker->n_partitions;
}

igraph_bool_t se2_do_save_partition(se2_tracker *tracker)
{
  return tracker->intervention_event;
}

static void se2_select_mode(igraph_integer_t const time, se2_tracker *tracker)
{
  if (time < 20) {
    tracker->mode = SE2_TYPICAL;
    return;
  }

  if ((tracker->allowed_to_merge) &&
      (tracker->time_since_last[SE2_MERGE] > 2) &&
      (tracker->time_since_last[SE2_BUBBLE] > 3)) {
    tracker->mode = SE2_MERGE;
    return;
  }

  if (!tracker->allowed_to_merge) {
    if ((tracker->time_since_last[SE2_MERGE] > 2) &&
        (tracker->time_since_last[SE2_BUBBLE] > 15)) {
      tracker->mode = SE2_BUBBLE;
      return;
    }
    if ((tracker->time_since_last[SE2_MERGE] >= 2) &&
        (tracker->time_since_last[SE2_BUBBLE] <= 4)) {
      tracker->mode = SE2_NURTURE;
      return;
    }
  }

  // Fallback
  tracker->mode = SE2_TYPICAL;
}

static void se2_shuffle_nodes(se2_partition const *partition)
{
  se2_randperm(partition->node_ids, partition->n_nodes,
               partition->n_nodes_to_update);
}

static void se2_premode_hook(se2_partition *partition, se2_tracker *tracker)
{
  se2_shuffle_nodes(partition);
  tracker->intervention_event = false;
  tracker->n_clusters_merged = 0;
}

static void se2_postmode_hook(se2_tracker *tracker)
{
  if (tracker->mode == SE2_NUM_MODES) {
    return;
  }

  tracker->n_times[tracker->mode]++;
  tracker->time_since_last[tracker->mode] = 0;
  for (size_t i = 0; i < SE2_NUM_MODES; i++) {
    tracker->time_since_last[i]++;
  }

  if (tracker->allowed_to_merge) {
    if ((tracker->mode == SE2_MERGE) && (tracker->n_clusters_merged == 0)) {
      tracker->allowed_to_merge = false;
      tracker->postintervention_count++;
      if (tracker->postintervention_count >= 0) {
        tracker->intervention_event = true;
      }
    }
  } else {
    if ((tracker->mode == SE2_BUBBLE) && (tracker->n_times[SE2_BUBBLE] > 2)) {
      tracker->allowed_to_merge = true;
    }
  }
}

static void se2_typical_mode(igraph_t const *graph, se2_partition *partition,
                             se2_tracker *tracker, igraph_integer_t time)
{
  puts("typical mode");
}

static void se2_bubble_mode(igraph_t const *graph, se2_partition *partition,
                            se2_tracker *tracker, igraph_integer_t time)
{
  puts("bubble_mode");
}

static void se2_merge_mode(igraph_t const *graph, se2_partition *partition,
                           se2_tracker *tracker, igraph_integer_t time)
{
  puts("merge_mode");
}
static void se2_nurture_mode(igraph_t const *graph, se2_partition *partition,
                             se2_tracker *tracker, igraph_integer_t time)
{
  puts("nurture_mode");
}

void se2_run_mode(igraph_t const *graph, se2_partition *partition,
                  se2_tracker *tracker,
                  igraph_integer_t const time)
{
  se2_premode_hook(partition, tracker);
  se2_select_mode(time, tracker);

  switch (tracker->mode) {
  case SE2_TYPICAL:
    se2_typical_mode(graph, partition, tracker, time);
    break;
  case SE2_BUBBLE:
    se2_bubble_mode(graph, partition, tracker, time);
    break;
  case SE2_MERGE:
    se2_merge_mode(graph, partition, tracker, time);
    break;
  case SE2_NURTURE:
    se2_nurture_mode(graph, partition, tracker, time);
    break;
  case SE2_NUM_MODES:
    // Never occurs.
    break;
  }

  se2_postmode_hook(tracker);
}

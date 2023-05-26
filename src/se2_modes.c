#include <stdlib.h>
#include <string.h>

#include "se2_modes.h"
#include "se2_label.h"

#define FRACTION_UNSTABLE_LABELS 0.9
#define FRACTION_NODES_TO_UPDATE 0.9

typedef enum {
  SE2_TYPICAL = 0,
  SE2_BUBBLE,
  SE2_MERGE,
  SE2_NURTURE,
  SE2_NUM_MODES
} se2_mode;

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

igraph_integer_t se2_tracker_mode(se2_tracker const *tracker)
{
  return tracker->mode;
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
  tracker->mode = SE2_TYPICAL; // Default

  if (time < 20) {
    return;
  }

  if (tracker->allowed_to_merge) {
    if ((tracker->time_since_last[SE2_MERGE] > 1) &&
        (tracker->time_since_last[SE2_BUBBLE] > 3)) {
      tracker->mode = SE2_MERGE;
      return;
    }
  } else {
    if ((tracker->time_since_last[SE2_MERGE] > 2) &&
        (tracker->time_since_last[SE2_BUBBLE] > 14)) {
      tracker->mode = SE2_BUBBLE;
      return;
    }

    if ((tracker->time_since_last[SE2_MERGE] > 1) &&
        (tracker->time_since_last[SE2_BUBBLE] < 5)) {
      tracker->mode = SE2_NURTURE;
      return;
    }
  }
}

static void se2_pre_step_hook(se2_tracker *tracker)
{
  tracker->intervention_event = false;
  tracker->n_clusters_merged = 0;
}

static void se2_post_step_hook(se2_tracker *tracker)
{
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

static void se2_typical_mode(igraph_t const *graph,
                             igraph_vector_t const *weights,
                             se2_partition *partition)
{
  puts("typical mode");
  se2_find_most_specific_labels(graph, weights, partition,
                                FRACTION_NODES_TO_UPDATE);
}

static void se2_bubble_mode(igraph_t const *graph, se2_partition *partition,
                            se2_tracker *tracker)
{
  puts("bubble_mode");
}

static void se2_merge_mode(igraph_t const *graph, se2_partition *partition,
                           se2_tracker *tracker)
{
  puts("merge_mode");
}

static void se2_nurture_mode(igraph_t const *graph,
                             igraph_vector_t const *weights,
                             se2_partition *partition)
{
  puts("nurture_mode");
  se2_relabel_worst_nodes(graph, weights, partition, FRACTION_UNSTABLE_LABELS);
}

void se2_mode_run_step(igraph_t const *graph, igraph_vector_t const *weights,
                       se2_partition *partition, se2_tracker *tracker, igraph_integer_t const time)
{
  se2_pre_step_hook(tracker);
  se2_select_mode(time, tracker);

  switch (tracker->mode) {
  case SE2_TYPICAL:
    se2_typical_mode(graph, weights, partition);
    break;
  case SE2_BUBBLE:
    se2_bubble_mode(graph, weights, partition);
    break;
  case SE2_MERGE:
    se2_merge_mode(graph, weights, partition);
    break;
  case SE2_NURTURE:
    se2_nurture_mode(graph, weights, partition);
    break;
  case SE2_NUM_MODES:
    // Never occurs.
    break;
  }

  se2_post_step_hook(tracker);
}

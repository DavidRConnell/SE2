#ifndef SE2_MODES_H
#define SE2_MODES_H

#include <igraph_interface.h>
#include <igraph_vector_list.h>
#include "speak_easy_2.h"

typedef struct se2_tracker se2_tracker;
typedef struct se2_partition se2_partition;

se2_tracker *se2_tracker_init(options const *opts);
void se2_tracker_destroy(se2_tracker *tracker);
se2_partition *se2_partition_init(igraph_t const *graph,
                                  igraph_vector_int_t *initial_labels);
void se2_partition_destroy(se2_partition *partition);
void se2_store_partition(se2_partition const *working_partition,
                         igraph_vector_int_list_t *partition_store,
                         igraph_integer_t const index);
igraph_bool_t se2_do_terminate(se2_tracker *tracker);
igraph_bool_t se2_do_save_partition(se2_tracker *tracker);
void se2_run_mode(igraph_t const *graph, se2_partition *partition,
                  se2_tracker *tracker, igraph_integer_t const time);

#endif

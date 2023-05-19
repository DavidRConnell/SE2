#ifndef SE2_LABEL_H
#define SE2_LABEL_H

#include "se2_partitions.h"

void se2_find_most_specific_labels(igraph_t const *graph,
                                   igraph_vector_t const *weights,
                                   se2_partition *partition);
void find_unstable_nodes();

#endif

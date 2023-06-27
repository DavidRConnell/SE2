#ifndef MEX_IGRAPH_H
#define MEX_IGRAPH_H

#include "mex.h"
#include <igraph_vector.h>
#include <igraph_datatype.h>

igraph_bool_t mxIgraphIsWeighted(const mxArray *p);
igraph_bool_t mxIgraphIsSymmetric(const mxArray *p);
igraph_bool_t mxIgraphIsDirected(const mxArray *p);
igraph_integer_t mxIgraphVCount(const mxArray *p);
igraph_integer_t mxIgraphECount(const mxArray *p);
igraph_integer_t mxIgraphVectorLength(const mxArray *p);

int mxIgraphGetVectorInt(igraph_vector_int_t *vec, const mxArray *p);
int mxIgraphArrayToPartition(igraph_vector_int_t *membership,
                             const mxArray *p);
int mxIgraphArrayToGraph(igraph_t *graph, const mxArray *p,
                         const igraph_bool_t directed);
int mxIgraphArrayToWeights(igraph_vector_t *weights, const mxArray *p,
                           const igraph_bool_t directed);

mxArray *mxIgraphCreateFullAdj(igraph_t const *graph,
                               igraph_vector_t const *weights);
mxArray *mxIgraphCreateSparseAdj(igraph_t const *graph,
                                 igraph_vector_t const *weights);
mxArray *mxIgraphCreatePartition(igraph_vector_int_t const *membership);

#endif

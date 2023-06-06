#ifndef MEX_IGRAPH_H
#define MEX_IGRAPH_H

#include "mex.h"
#include <igraph_vector.h>
#include <igraph_datatype.h>

igraph_integer_t mxIgraphVectorLength(const mxArray *p);

int mxIgraphGetVectorInt(igraph_vector_int_t *vec, const mxArray *p);
mxArray *mxIgraphCreateFullAdj(igraph_t const *graph,
                               igraph_vector_t const *weights);
mxArray *mxIgraphCreateSparseAdj(igraph_t const *graph,
                                 igraph_vector_t const *weights);

#endif

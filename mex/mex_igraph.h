#ifndef MEX_IGRAPH_H
#define MEX_IGRAPH_H

#include "mex.h"
#include <igraph_vector.h>
#include <igraph_datatype.h>

size_t vec_length(const mxArray *);
int mxGetVector(igraph_vector_t *, const mxArray *);
int mxGetIntVector(igraph_vector_int_t *, const mxArray *);
int mxGetBoolVector(igraph_vector_bool_t *, const mxArray *);
int mxGetMatrix(igraph_t *, const mxArray *);

#endif

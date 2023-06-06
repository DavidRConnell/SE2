#include "mex_igraph.h"

#include <igraph_interface.h>

static igraph_bool_t mxIgraphIsVector(const mxArray *p)
{
  mwSize n = mxGetN(p);
  mwSize m = mxGetM(p);

  if ((n != 1) && (m != 1)) {
    return false;
  }

  if ((n == 1) && (m == 1)) {
    return false;
  }

  return true;
}

igraph_integer_t mxIgraphVectorLength(const mxArray *p)
{
  mwSize n = mxGetN(p);
  mwSize m = mxGetM(p);

  if ((n != 1) && (m != 1)) {
    mexErrMsgIdAndTxt("Igraph:NotVector",
                      "Inputs should be a vector not a matrix");
  }

  if ((n == 1) && (m == 1)) {
    mexErrMsgIdAndTxt("Igraph:NotVector",
                      "Inputs should be a vector not a scaler");
  }

  return n > m ? n : m;
}

int mxIgraphGetVectorInt(igraph_vector_int_t *vec, const mxArray *p)
{
  mxDouble *x_mat = mxGetDoubles(p);
  igraph_integer_t n = mxIgraphVectorLength(p);

  igraph_vector_int_init(vec, n);
  for (igraph_integer_t i = 0; i < n; i++) {
    VECTOR(*vec)[i] = (igraph_integer_t)x_mat[i];
  }

  return EXIT_SUCCESS;
}

{

  }

  return EXIT_SUCCESS;
}

mxArray *mxIgraphCreateFullAdj(igraph_t const *graph,
                               igraph_vector_t const *weights)
{
  igraph_integer_t n_nodes = igraph_vcount(graph);
  mxArray *p = mxCreateDoubleMatrix(n_nodes, n_nodes, mxREAL);
  mxDouble *adj = mxGetDoubles(p);
  igraph_eit_t eit;
  igraph_integer_t eid;
  mwIndex idx;

  igraph_eit_create(graph, igraph_ess_all(IGRAPH_EDGEORDER_ID), &eit);

  while (!IGRAPH_EIT_END(eit)) {
    eid = IGRAPH_EIT_GET(eit);
    idx = IGRAPH_FROM(graph, eid) + (n_nodes * IGRAPH_TO(graph, eid));
    adj[idx] = (mxDouble)(weights ? VECTOR(*weights)[eid] : 1);

    IGRAPH_EIT_NEXT(eit);
  }

  igraph_eit_destroy(&eit);

  return p;
}

mxArray *mxIgraphCreateSparseAdj(igraph_t const *graph,
                                 igraph_vector_t const *weights)
{
  igraph_integer_t n_edges = igraph_ecount(graph);
  igraph_integer_t n_nodes = igraph_vcount(graph);
  mxArray *p = mxCreateSparse(n_nodes, n_nodes, n_edges, mxREAL);
  mxDouble *adj = mxGetDoubles(p);
  mwIndex *jc = mxGetJc(p);
  mwIndex *ir = mxGetIr(p);
  igraph_eit_t eit;
  igraph_integer_t eid;

  igraph_eit_create(graph, igraph_ess_all(IGRAPH_EDGEORDER_TO), &eit);

  mwIndex count = 0;
  mwIndex column, prev_column = 0;
  jc[prev_column] = count;
  while (!IGRAPH_EIT_END(eit)) {
    eid = IGRAPH_EIT_GET(eit);
    column = IGRAPH_TO(graph, eid);
    if (prev_column != column) {
      for (igraph_integer_t i = prev_column; i < column; i++) {
        jc[i + 1] = count;
      }
      prev_column = column;
    }

    ir[count] = IGRAPH_FROM(graph, eid);
    adj[count] = (mxDouble)(weights ? VECTOR(*weights)[eid] : 1);

    IGRAPH_EIT_NEXT(eit);
    count++;
  }
  jc[n_nodes] = count;

  igraph_eit_destroy(&eit);

  return p;
}

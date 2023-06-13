#include "mex_igraph.h"

#include <igraph_community.h>
#include <igraph_interface.h>

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  igraph_t graph;
  igraph_bool_t directed;
  igraph_vector_t weights;
  igraph_vector_t *weight_ptr = NULL;
  igraph_vector_int_t membership;
  igraph_vector_int_t *membership_ptr = NULL;
  igraph_real_t modularity;

  if (nlhs > 2) {
    mexErrMsgIdAndTxt("Igraph:OptimalModularity:TooManyOutputs",
                      "OptimalModularity returns 1 to 2 outputs.");
  }

  if (nrhs != 1) {
    mexErrMsgIdAndTxt("Igraph:OptimalModularity:WrongNumberOfInputs",
                      "OptimalModularity requires only 1 inputs,"
                      "the graph adjacency matrix.");
  }

  directed = !mxIgraphIsSymmetric(prhs[0]);
  mxIgraphArrayToGraph(&graph, prhs[0], directed);
  if (mxIgraphIsWeighted(prhs[0])) {
    mxIgraphArrayToWeights(&weights, prhs[0], directed);
    weight_ptr = &weights;
  }

  if (nlhs == 2) {
    igraph_vector_int_init(&membership, 0);
    membership_ptr = &membership;
  }

  igraph_community_optimal_modularity(&graph, &modularity, membership_ptr,
                                      weight_ptr);

  if (weight_ptr) {
    igraph_vector_destroy(&weights);
  }
  igraph_destroy(&graph);

  plhs[0] = mxCreateDoubleScalar((double)modularity);

  if (nlhs == 2) {
    plhs[1] = mxIgraphCreatePartition(&membership);
    igraph_vector_int_destroy(&membership);
  }
}

#include "mex_igraph.h"

#include <igraph_community.h>
#include <igraph_interface.h>

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  igraph_t graph;
  igraph_bool_t directed;
  igraph_vector_t *weights;
  igraph_vector_int_t membership;
  igraph_real_t modularity;

  if (nlhs > 1) {
    mexErrMsgIdAndTxt("Igraph:OptimalModularity:TooManyOutputs",
                      "OptimalModularity returns 1 output.");
  }

  if (nrhs != 2) {
    mexErrMsgIdAndTxt("Igraph:OptimalModularity:WrongNumberOfInputs",
                      "OptimalModularity requires 2 inputs, the graph adjacency"
                      "matrix and a membership partition.");
  }

  directed = mxIgraphIsDirected(prhs[0]);
  mxIgraphArrayToGraph(&graph, prhs[0], directed);
  if (mxIgraphIsWeighted(prhs[0])) {
    mxIgraphArrayToWeights(weights, prhs[0], directed);
  } else {
    weights = NULL;
  }

  mxIgraphArrayToPartition(&membership, prhs[1]);

  igraph_reindex_membership(&membership, NULL, NULL);

  igraph_modularity(&graph, &membership, weights,
                    1, directed, &modularity);

  if (weights) {
    igraph_vector_destroy(weights);
  }
  igraph_destroy(&graph);

  plhs[0] = mxCreateDoubleScalar((double)modularity);
}

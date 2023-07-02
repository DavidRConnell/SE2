#include "mex_igraph.h"

#include <igraph_interface.h>
#include <igraph_community.h>

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  mxIgraphSetErrorHandler();

  igraph_t graph;
  igraph_bool_t directed;
  igraph_vector_t weights;
  igraph_vector_t *weights_ptr = NULL;
  igraph_vector_int_t membership;

  igraph_real_t resolution = 0.7;
  igraph_real_t beta = 0.01;
  igraph_integer_t n_iterations = -1;

  if (nrhs != 1) {
    mexErrMsgIdAndTxt("Igraph:Leiden:WrongNumberOfInputs",
                      "%s requires one input argument, an adjacency matrix",
                      mexFunctionName());
  }

  if (nlhs > 1) {
    mexErrMsgIdAndTxt("Igraph:Leiden:WrongNumberOfOutputs",
                      "%s returns only one value.",
                      mexFunctionName());
  }

  directed = mxIgraphIsDirected(prhs[0]);
  if (directed) {
    // TODO: Eventually should be covered by setting the igraph error handler.
    mexErrMsgIdAndTxt("Igraph:Leiden:Undirected",
                      "Leiden algorithm is only implemented for undirected graphs.");
  }

  mxIgraphArrayToGraph(&graph, prhs[0], directed);
  if (mxIgraphIsWeighted(prhs[0])) {
    mxIgraphArrayToWeights(&weights, prhs[0], directed);
    weights_ptr = &weights;
  }

  igraph_vector_int_init(&membership, 0);
  igraph_community_leiden(&graph, weights_ptr, NULL, resolution, beta, false,
                          n_iterations, &membership, NULL, NULL);

  if (weights_ptr) {
    igraph_vector_destroy(&weights);
  }
  igraph_destroy(&graph);

  plhs[0] = mxIgraphCreatePartition(&membership);
  igraph_vector_int_destroy(&membership);
}

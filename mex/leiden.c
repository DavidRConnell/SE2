#include "mex_igraph.h"

#include <string.h>
#include <igraph_interface.h>
#include <igraph_community.h>
#include <igraph_structural.h>

typedef enum {
  LEIDEN_MODULARITY = 0,
  LEIDEN_CPM
} leiden_optimizer;

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  mxIgraphSetErrorHandler();

  igraph_t graph;
  igraph_bool_t directed;
  igraph_vector_t weights;
  igraph_vector_t *weights_ptr = NULL;
  igraph_vector_t k;
  igraph_vector_int_t membership;

  leiden_optimizer optimizer = LEIDEN_MODULARITY;
  igraph_real_t resolution = 1;
  igraph_real_t edge_sum;
  igraph_real_t beta = 0.01;
  igraph_integer_t n_iterations = -1;

  if (!((nrhs == 1) || (nrhs == 3))) {
    mexErrMsgIdAndTxt("Igraph:Leiden:WrongNumberOfInputs",
                      "%s requires one or three input argument, an adjacency matrix.",
                      mexFunctionName());
  }

  if (nrhs == 3) {
    if (strcmp(mxArrayToString(prhs[1]), "method") != 0) {
      mexErrMsgIdAndTxt("Igraph:Leiden:UnknownArgument",
                        "%s is not a known argument name.",
                        mxArrayToString(prhs[1]));
    }

    char *optimizer_name = mxArrayToString(prhs[2]);
    if (strcmp(optimizer_name, "modularity") == 0) {
      optimizer = LEIDEN_MODULARITY;
    } else if (strcmp(optimizer_name, "cpm") == 0) {
      optimizer = LEIDEN_CPM;
    } else {
      mexErrMsgIdAndTxt("Igraph:Leiden:UnknownOptimizer",
                        "%s is not a known metric to optimize.",
                        optimizer_name);
    }
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

  if (optimizer == LEIDEN_MODULARITY) {
    edge_sum = weights_ptr ? igraph_vector_sum(&weights) :
               igraph_ecount(&graph);

    igraph_vector_init(&k, igraph_vcount(&graph));
    igraph_strength(&graph, &k, igraph_vss_all(), IGRAPH_ALL, 1,
                    weights_ptr);
    igraph_community_leiden(&graph, weights_ptr, &k,
                            resolution / (2 * edge_sum), beta, false,
                            n_iterations, &membership, NULL, NULL);
    igraph_vector_destroy(&k);
  } else {
    // Resolution of 1 has a tendency to cause leiden to get stuck.
    resolution = 0.7;
    igraph_community_leiden(&graph, weights_ptr, NULL,
                            resolution, beta, false,
                            n_iterations, &membership, NULL, NULL);
  }

  if (weights_ptr) {
    igraph_vector_destroy(&weights);
  }
  igraph_destroy(&graph);

  plhs[0] = mxIgraphCreatePartition(&membership);
  igraph_vector_int_destroy(&membership);
}

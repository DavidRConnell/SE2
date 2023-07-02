#include "mex_igraph.h"

#include <igraph_community.h>

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  mxIgraphSetErrorHandler();

  if (nrhs != 2) {
    mexErrMsgIdAndTxt("Igraph:nmi:WrongNumberOfInputs",
                      "%s requires two inputs, the partitions to be compared.",
                      mexFunctionName());
  }

  if (nlhs > 1) {
    mexErrMsgIdAndTxt("Igraph:nmi:WrongNumberOfOutputs",
                      "%s only returns one value.", mexFunctionName());
  }

  if (!(mxIgraphVectorLength(prhs[0]) == mxIgraphVectorLength(prhs[1]))) {
    mexErrMsgIdAndTxt("Igraph:nmi:SizeMismatch",
                      "Inputs must have the same length");
  }

  igraph_vector_int_t x;
  igraph_vector_int_t y;
  igraph_real_t res;

  mxIgraphArrayToPartition(&x, prhs[0]);
  mxIgraphArrayToPartition(&y, prhs[1]);

  igraph_reindex_membership(&x, NULL, NULL);
  igraph_reindex_membership(&y, NULL, NULL);

  igraph_compare_communities(&x, &y, &res, IGRAPH_COMMCMP_NMI);

  plhs[0] = mxCreateDoubleScalar((double)res);
}

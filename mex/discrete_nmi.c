#include "mex_igraph.h"
#include <igraph_community.h>

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  if (!(vec_length(prhs[0]) == vec_length(prhs[1]))) {
    mexErrMsgIdAndTxt("IGRAPH:nmi:sizeMismatch",
                      "Inputs must have the same length");
  }

  igraph_vector_int_t x;
  igraph_vector_int_t y;
  igraph_real_t res;

  mxGetIntVector(&x, prhs[0]);
  mxGetIntVector(&y, prhs[1]);

  igraph_compare_communities(&x, &y, &res, IGRAPH_COMMCMP_NMI);

  plhs[0] = mxCreateDoubleScalar((double)res);

  igraph_vector_int_destroy(&x);
  igraph_vector_int_destroy(&y);
}

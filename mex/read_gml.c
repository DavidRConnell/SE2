#include "mex_igraph.h"

#include <igraph_foreign.h>
#include <igraph_interface.h>

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  mxIgraphSetErrorHandler();

  if (nrhs != 1) {
    mexErrMsgIdAndTxt("Igraph:WrongNumberOfInputs", "%s requires a file name.",
                      mexFunctionName());
  }

  if (!(mxIsChar(prhs[0]))) {
    mexErrMsgIdAndTxt("Igraph:WrongInputType", "%s requires a file name.",
                      mexFunctionName());
  }

  FILE *fptr;
  igraph_t graph;

  if (!(fptr = fopen(mxArrayToString(prhs[0]), "r"))) {
    mexErrMsgIdAndTxt("Igraph:NotAFile", "Could not read file %s.",
                      mxArrayToString(prhs[0]));
  }

  igraph_read_graph_gml(&graph, fptr);

  fclose(fptr);

  plhs[0] = mxIgraphCreateSparseAdj(&graph, NULL);
  igraph_destroy(&graph);
}

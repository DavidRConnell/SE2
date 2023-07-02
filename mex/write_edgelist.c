#include "mex_igraph.h"

#include <igraph_foreign.h>
#include <igraph_interface.h>

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  mxIgraphSetErrorHandler();

  if (nrhs != 2) {
    mexErrMsgIdAndTxt("Igraph:WrongNumberOfInputs", "%s requires a file name.",
                      mexFunctionName());
  }

  if (!(mxIsLogical(prhs[0]))) {
    mexErrMsgIdAndTxt("Igraph:WrongInputType", "%s requires a adjacency matrix.",
                      mexFunctionName());
  }

  if (!(mxIsChar(prhs[1]))) {
    mexErrMsgIdAndTxt("Igraph:WrongInputType", "%s requires a file name.",
                      mexFunctionName());
  }

  FILE *fptr = fopen(mxArrayToString(prhs[1]), "w");
  igraph_t graph;
  igraph_bool_t is_directed = mxIgraphIsDirected(prhs[0]);

  mxIgraphArrayToGraph(&graph, prhs[0], is_directed);

  fprintf(fptr, "from to\n");
  igraph_write_graph_edgelist(&graph, fptr);
  fclose(fptr);

  igraph_destroy(&graph);
}

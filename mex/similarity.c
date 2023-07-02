#include "mex_igraph.h"

#include <string.h>
#include <igraph_community.h>

#ifdef _WINDOWS
#define strcasecmp stricmp
#endif

#define STREQ(a, b) strcasecmp((a), (b)) == 0

void mexFunction(int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
  mxIgraphSetErrorHandler();

  mwSize n_required_inputs = 2;
  mwSize n_optional_inputs = 1;
  igraph_vector_int_t x;
  igraph_vector_int_t y;
  igraph_real_t res;
  igraph_community_comparison_t method = IGRAPH_COMMCMP_NMI;

  if (nrhs < n_required_inputs) {
    mexErrMsgIdAndTxt("Igraph:similarity:IncorrectArguments",
                      "%s requires at least two arguments, the partitions to compare.",
                      mexFunctionName());
  }

  if (nrhs > (n_required_inputs + (n_optional_inputs * 2))) {
    mexErrMsgIdAndTxt("Igraph:similarity:IncorrectArguments",
                      "%s accepts at most %zu arguments, the two partitions "
                      "to compare and a method name value pair.",
                      mexFunctionName(),
                      (size_t)(n_required_inputs + (n_optional_inputs * 2)));
  }

  if (nlhs > 1) {
    mexErrMsgIdAndTxt("Igraph:similarity:WrongNumberOfOutputs",
                      "%s only returns one value.", mexFunctionName());
  }

  if (!(mxIgraphVectorLength(prhs[0]) == mxIgraphVectorLength(prhs[1]))) {
    mexErrMsgIdAndTxt("Igraph:similarity:SizeMismatch",
                      "Inputs must have the same length");
  }

  if (nrhs > n_required_inputs) {
    const mxArray **optionals = prhs + n_required_inputs;
    char *name;
    char *method_name;
    char *methods[] = {
      [IGRAPH_COMMCMP_VI] = "vi",
      [IGRAPH_COMMCMP_NMI] = "nmi",
      [IGRAPH_COMMCMP_RAND] = "rand",
      [IGRAPH_COMMCMP_SPLIT_JOIN] = "split_join",
      [IGRAPH_COMMCMP_ADJUSTED_RAND] = "adjusted_rand"
    };
    igraph_integer_t n_methods = sizeof(methods) / sizeof(*methods);

    if (!mxIsChar(optionals[0])) {
      mexErrMsgIdAndTxt("Igraph:similarity:IncorrectArguments",
                        "%zu input must be a string.",
                        (size_t)(n_required_inputs + 1));
    }

    name = mxArrayToString(optionals[0]);
    if (!(STREQ(name, "method"))) {
      mexErrMsgIdAndTxt("Igraph:similarity:UnknownOption",
                        "Did not recognize \"%s\". "
                        "\"method\" is the only optional name value pair.",
                        name);
    }

    if (!mxIsChar(optionals[1])) {
      mexErrMsgIdAndTxt("Igraph:similarity:IncorrectArguments",
                        "Method type must be a string.");
    }

    method_name = mxArrayToString(optionals[1]);
    for (igraph_integer_t i = 0; i <= n_methods; i++) {
      if (i == n_methods) {
        mexErrMsgIdAndTxt("Igraph:similarity:UnknownMethod",
                          "%s is not a known method.", method_name);
      }
      if (STREQ(method_name, methods[i])) {
        method = i;
        break;
      }
    }
  }

  mxIgraphArrayToPartition(&x, prhs[0]);
  mxIgraphArrayToPartition(&y, prhs[1]);

  igraph_reindex_membership(&x, NULL, NULL);
  igraph_reindex_membership(&y, NULL, NULL);

  igraph_compare_communities(&x, &y, &res, method);

  plhs[0] = mxCreateDoubleScalar((double)res);
}

#include "mex_igraph.h"

size_t vec_length(const mxArray *p)
{
  double n = mxGetN(p);
  double m = mxGetM(p);

  if ((n != 1) && (m != 1)) {
    mexErrMsgIdAndTxt("IGRAPH:nmi:notVector",
                      "Inputs should be a vector not a matrix");
  }

  return n > m ? n : m;
}

int mxGetVector(igraph_vector_t *vec, const mxArray *p)
{
  double *x_mat = mxGetPr(p);
  size_t n = vec_length(p);

  igraph_vector_init(vec, n);
  for (int i = 0; i < n; i++) {
    VECTOR(*vec)[i] = (igraph_real_t)x_mat[i];
  }

  return EXIT_SUCCESS;
}

int mxGetIntVector(igraph_vector_int_t *vec, const mxArray *p)
{
  double *x_mat = mxGetPr(p);
  int n = vec_length(p);

  igraph_vector_int_init(vec, n);
  for (int i = 0; i < n; i++) {
    VECTOR(*vec)[i] = (igraph_integer_t)x_mat[i];
  }

  return EXIT_SUCCESS;
}

int mxGetBoolVector(igraph_vector_bool_t *vec, const mxArray *p)
{
  double *x_mat = mxGetPr(p);
  int n = vec_length(p);

  igraph_vector_bool_init(vec, n);
  for (int i = 0; i < n; i++) {
    VECTOR(*vec)[i] = (igraph_bool_t)x_mat[i];
  }

  return EXIT_SUCCESS;
}

int mxGetMatrix(igraph_t *mat, const mxArray *p);

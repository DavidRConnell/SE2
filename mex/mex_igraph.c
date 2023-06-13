#include "mex_igraph.h"

#include <igraph_interface.h>

static bool isSquare(const mxArray *p)
{
  mwIndex m = mxGetM(p);
  mwIndex n = mxGetN(p);

  return m == n;
}

static igraph_bool_t mxIgraphIsWeightedSparse(const mxArray *p)
{
  mxDouble *adj = mxGetDoubles(p);
  mwIndex *ir = mxGetIr(p);
  mwIndex *jc = mxGetJc(p);
  mwIndex n = mxGetN(p);
  mwIndex numel = jc[n];

  if (!isSquare(p)) {
    mexErrMsgIdAndTxt("Igraph:NotSquare", "Adjacency matrix must be square.");
  }

  for (mwIndex i = 0; i < numel; i++) {
    if ((adj[i] != 0) && (adj[i] != 1)) {
      return true;
    }
  }

  return false;
}

static igraph_bool_t mxIgraphIsWeightedFull(const mxArray *p)
{
  mxDouble *adj = mxGetDoubles(p);
  mwSize m = mxGetM(p);

  if (!isSquare(p)) {
    mexErrMsgIdAndTxt("Igraph:NotSquare", "Adjacency matrix must be square.");
  }

  mxDouble el;
  for (mwIndex i = 0; i < m; i++) {
    for (mwIndex j = 0; j < m; j++) {
      el = adj[i + (j * m)];
      if ((el != 0) && (el != 1)) {
        return true;
      }
    }
  }

  return false;
}


/* Test if adjacency matrix p points to has values other than 0 or 1. */
igraph_bool_t mxIgraphIsWeighted(const mxArray *p)
{
  if (mxIsSparse(p)) {
    return mxIgraphIsWeightedSparse(p);
  }

  return mxIgraphIsWeightedFull(p);
}

static mxDouble sparse_index(const mxDouble *column, const mwIndex i,
                             const mwIndex *row_indices, const mwIndex len)
{
  if (len == 0) {
    return 0;
  }

  mwIndex rng[] = { 0, len - 1 };
  mwIndex idx = (len - 1) / 2;
  while ((rng[1] >= rng[0]) && (rng[1] < len)) { // Protect against underflow
    if (row_indices[idx] == i) {
      return column[idx];
    }

    if (row_indices[idx] < i) {
      rng[0] = idx + 1;
    } else {
      rng[1] = idx - 1;
    }
    idx = (rng[0] + rng[1]) / 2;
  }

  return 0;
}

static igraph_bool_t mxIgraphIsSymmetricSparse(const mxArray *p)
{
  mxDouble *adj = mxGetDoubles(p);
  mwIndex *ir = mxGetIr(p);
  mwIndex *jc = mxGetJc(p);
  mwIndex n_nodes = mxGetM(p);

  mxDouble reflection;
  mwIndex row_i;
  for (mwIndex j = 0; j < n_nodes; j++) {
    for (mwIndex i = jc[j]; i < jc[j + 1]; i++) {
      row_i = ir[i];
      reflection = sparse_index(adj + jc[row_i], j,
                                ir + jc[row_i],
                                jc[row_i + 1] - jc[row_i]);
      if (reflection != adj[i]) {
        return false;
      }
    }
  }

  return true;
}

static igraph_bool_t mxIgraphIsSymmetricFull(const mxArray *p)
{
  mxDouble *adj = mxGetDoubles(p);
  mwIndex n_nodes = mxGetM(p);

  for (mwIndex i = 0; i < n_nodes; i++) {
    for (mwIndex j = (i + 1); j < n_nodes; j++) {
      if (adj[i + (j * n_nodes)] != adj[j + (i * n_nodes)]) {
        return false;
      }
    }
  }

  return true;
}

/* Test if the adjacency matrix pointed to by p is symmetric. */
igraph_bool_t mxIgraphIsSymmetric(const mxArray *p)
{
  if (!(isSquare(p))) {
    return false;
  }

  if (mxIsSparse(p)) {
    return mxIgraphIsSymmetricSparse(p);
  }

  return mxIgraphIsSymmetricFull(p);
}

/* Return the number of nodes in the adjacency matrix pointed to by p. */
igraph_integer_t mxIgraphVCount(const mxArray *p)
{
  return (igraph_integer_t)mxGetM(p);
}

/* Returns the number of edges in the matrix pointed to by p.

   NOTE: This does not take into account whether the matrix is a directed or
   undirected adjacency matrix. In the undirected condition, this will double
   count off-diagonal edges. */
igraph_integer_t mxIgraphECount(const mxArray *p)
{
  mwIndex n_nodes = mxGetM(p);

  if (mxIsSparse(p)) {
    mwIndex *jc = mxGetJc(p);
    return (igraph_integer_t)jc[n_nodes];
  }

  mxDouble *adj = mxGetDoubles(p);
  igraph_integer_t n_edges = 0;
  for (mwIndex i = 0; i < n_nodes; i++) {
    for (mwIndex j = 0; j < n_nodes; j++) {
      if (adj[i + (j * n_nodes)]) {
        n_edges++;
      }
    }
  }

  return n_edges;
}

/* Check if the matlab array pointed to by p is either a row or column
vector. */
static igraph_bool_t mxIgraphIsVector(const mxArray *p)
{
  mwSize n = mxGetN(p);
  mwSize m = mxGetM(p);

  if ((n != 1) && (m != 1)) {
    return false;
  }

  if ((n == 1) && (m == 1)) {
    return false;
  }

  return true;
}

/* Return the length of the vector pointed to by p. */
igraph_integer_t mxIgraphVectorLength(const mxArray *p)
{
  mwSize n = mxGetN(p);
  mwSize m = mxGetM(p);

  if ((n != 1) && (m != 1)) {
    mexErrMsgIdAndTxt("Igraph:NotVector",
                      "Inputs should be a vector not a matrix");
  }

  if ((n == 1) && (m == 1)) {
    mexErrMsgIdAndTxt("Igraph:NotVector",
                      "Inputs should be a vector not a scalar");
  }

  return n > m ? n : m;
}

/* Copy a matlab vector to an igraph vector.

 The igraph vector should be uninitialized, but it's the callers responsibility
 to destroy it when done. */
int mxIgraphGetVectorInt(igraph_vector_int_t *vec, const mxArray *p)
{
  mxDouble *x_mat = mxGetDoubles(p);
  igraph_integer_t n = mxIgraphVectorLength(p);

  igraph_vector_int_init(vec, n);
  for (igraph_integer_t i = 0; i < n; i++) {
    VECTOR(*vec)[i] = (igraph_integer_t)x_mat[i];
  }

  return EXIT_SUCCESS;
}

static int mxIgraphGetPartitionFromCell(igraph_vector_int_t *membership,
                                        const mxArray *p)
{
  mexErrMsgIdAndTxt("Igraph:NotImplemented",
                    "Getting a partition from a cell format has not been implemented.");

  return EXIT_FAILURE;
}

static int mxIgraphGetPartitionFromDouble(igraph_vector_int_t *membership,
    const mxArray *p)
{
  if (!(mxIgraphIsVector(p))) {
    mexErrMsgIdAndTxt("Igraph:NotVector", "Partition must be a vector.");
  }

  mxIgraphGetVectorInt(membership, p);
  return EXIT_SUCCESS;
}

/* Copy a matlab partition to an igraph vector.

   If matlab partition starts with node id 1 instead of node id 0, all values
   will be reduced such that the smallest node id is 0.

   Membership should be uninitialized, but it's the callers responsibility to
   destroy it when done.*/
int mxIgraphArrayToPartition(igraph_vector_int_t *membership,
                             const mxArray *p)
{
  int rc;

  if (mxIsCell(p)) {
    rc = mxIgraphGetPartitionFromCell(membership, p);
  } else if (mxIsDouble(p)) {
    rc = mxIgraphGetPartitionFromDouble(membership, p);
  }

  igraph_integer_t min_id = igraph_vector_int_min(membership);
  if (min_id != 0) {
    for (igraph_integer_t i = 0; i < igraph_vector_int_size(membership); i++) {
      VECTOR(*membership)[i] -= min_id;
    }
  }

  if (rc) {
    mexErrMsgIdAndTxt("Igraph:NotPartition",
                      "Value not a valid partition format.");
  }

  return rc;
}

static int mxIgraphGraphFromSparseAdj(igraph_t *graph, const mxArray *p,
                                      const igraph_bool_t directed)
{
  mxDouble *adj = mxGetDoubles(p);
  mwIndex *ir = mxGetIr(p);
  mwIndex *jc = mxGetJc(p);
  igraph_integer_t n_nodes = mxIgraphVCount(p);
  igraph_integer_t n_edges = mxIgraphECount(p);
  igraph_vector_int_t edges;

  igraph_vector_int_init(&edges, n_edges * 2);

  igraph_integer_t edge_i = 0;
  for (igraph_integer_t j = 0; j < n_nodes; j++) {
    for (igraph_integer_t i = jc[j];
         (directed ? true : ir[i] <= j) && (i < jc[j + 1]); i++) {
      VECTOR(edges)[edge_i] = ir[i];
      VECTOR(edges)[edge_i + 1] = j;
      edge_i += 2;
    }
  }

  if (edge_i < n_edges) {
    igraph_vector_int_resize(&edges, edge_i);
  }

  igraph_add_edges(graph, &edges, NULL);
  igraph_vector_int_destroy(&edges);

  return EXIT_SUCCESS;
}

static int mxIgraphGraphFromFullAdj(igraph_t *graph, const mxArray *p,
                                    const igraph_bool_t directed)
{
  mxDouble *adj = mxGetDoubles(p);
  igraph_integer_t n_nodes = mxIgraphVCount(p);
  igraph_integer_t n_edges = mxIgraphECount(p);
  igraph_vector_int_t edges;

  igraph_vector_int_init(&edges, n_edges * 2);

  igraph_integer_t edge_i = 0;
  for (igraph_integer_t i = 0; i < n_nodes; i++) {
    for (igraph_integer_t j = (directed ? 0 : i); j < n_nodes; j++) {
      if (adj[i + (j * n_nodes)]) {
        VECTOR(edges)[edge_i] = i;
        VECTOR(edges)[edge_i + 1] = j;
        edge_i += 2;
      }
    }
  }

  if (edge_i < n_edges) {
    igraph_vector_int_resize(&edges, edge_i);
  }

  igraph_add_edges(graph, &edges, NULL);
  igraph_vector_int_destroy(&edges);

  return EXIT_SUCCESS;
}

/* Copy a matlab adjacency matrix (sparse or full) to an igraph graph type.

   Graph should be uninitialized but it's the callers responsibility to destroy
   it when done.

   See `mxIgraphArrayToWeights` for collecting the weights if the matrix is
   weighted. */
int mxIgraphArrayToGraph(igraph_t *graph, const mxArray *p,
                         const igraph_bool_t directed)
{
  if (!isSquare(p)) {
    mexErrMsgIdAndTxt("Igraph:notSquare", "Adjacency matrix must be square.");
  }

  igraph_integer_t n_nodes = mxIgraphVCount(p);

  igraph_empty(graph, n_nodes, directed);

  int rc;
  if (mxIsSparse(p)) {
    rc = mxIgraphGraphFromSparseAdj(graph, p, directed);
  } else {
    rc = mxIgraphGraphFromFullAdj(graph, p, directed);
  }

  return rc;
}

static int mxIgraphWeightsFromSparseAdj(igraph_vector_t *weights,
                                        const mxArray *p,
                                        const igraph_bool_t directed)
{
  mxDouble *adj = mxGetDoubles(p);
  mwIndex *ir = mxGetIr(p);
  mwIndex *jc = mxGetJc(p);
  igraph_integer_t n_nodes = mxIgraphVCount(p);
  igraph_integer_t n_edges = mxIgraphECount(p);

  igraph_vector_init(weights, n_edges * 2);

  igraph_integer_t edge_i = 0;
  for (igraph_integer_t j = 0; j < n_nodes; j++) {
    for (igraph_integer_t i = jc[j];
         (directed ? true : ir[i] < j) && (i < jc[j + 1]); i++) {
      VECTOR(*weights)[edge_i] = ir[i];
      VECTOR(*weights)[edge_i + 1] = j;
      edge_i += 2;
    }
  }

  if (edge_i < n_edges) {
    igraph_vector_resize(weights, edge_i);
  }

  return EXIT_SUCCESS;
}

static int mxIgraphWeightsFromFullAdj(igraph_vector_t *weights,
                                      const mxArray *p,
                                      const igraph_bool_t directed)
{
  mxDouble *adj = mxGetDoubles(p);
  igraph_integer_t n_nodes = mxIgraphVCount(p);
  igraph_integer_t n_edges = mxIgraphECount(p);

  igraph_vector_init(weights, n_edges * 2);

  igraph_integer_t edge_i = 0;
  for (igraph_integer_t i = 0; i < n_nodes; i++) {
    for (igraph_integer_t j = (directed ? 0 : i); j < n_nodes; j++) {
      if (adj[i + (j * n_nodes)]) {
        VECTOR(*weights)[edge_i] = i;
        VECTOR(*weights)[edge_i + 1] = j;
        edge_i += 2;
      }
    }
  }

  if (edge_i < n_edges) {
    igraph_vector_resize(weights, edge_i);
  }

  return EXIT_SUCCESS;
}

/* Copy the weights from a weighted matlab adjacency matrix (sparse or full) to
   an igraph vector.

   Weights should be uninitialized, but it's the callers responsibility to
   destroy it when done.

   See `mxIgraphArrayToGraph` for collecting the adjacency matrices edges. */
int mxIgraphArrayToWeights(igraph_vector_t *weights, const mxArray *p,
                           const igraph_bool_t directed)
{
  if (!isSquare(p)) {
    mexErrMsgIdAndTxt("Igraph:notSquare", "Adjacency matrix must be square.");
  }

  int rc;
  if (mxIsSparse(p)) {
    rc = mxIgraphWeightsFromSparseAdj(weights, p, directed);
  } else {
    rc = mxIgraphWeightsFromFullAdj(weights, p, directed);
  }

  return rc;
}

/* Create a matlab adjacency matrix using an igraph graph and weight vector.

   If the weights vector is `NULL` the resulting adjacency matrix will be use
   `1` for the weight of all edges.

   See `mxIgraphCreateSparseAdj` for the sparse equivalent. */
mxArray *mxIgraphCreateFullAdj(igraph_t const *graph,
                               igraph_vector_t const *weights)
{
  mwSize n_nodes = (mwSize)igraph_vcount(graph);
  mxArray *p = mxCreateDoubleMatrix(n_nodes, n_nodes, mxREAL);
  double *adj = mxGetDoubles(p);
  igraph_eit_t eit;
  igraph_integer_t eid;
  mwIndex idx;

  igraph_eit_create(graph, igraph_ess_all(IGRAPH_EDGEORDER_ID), &eit);

  while (!IGRAPH_EIT_END(eit)) {
    eid = IGRAPH_EIT_GET(eit);
    idx = (mwSize)IGRAPH_FROM(graph, eid) +
          (n_nodes * (mwSize)IGRAPH_TO(graph, eid));
    adj[idx] = (double)(weights ? VECTOR(*weights)[eid] : 1);

    IGRAPH_EIT_NEXT(eit);
  }

  igraph_eit_destroy(&eit);

  return p;
}

/* Create a matlab adjacency matrix using an igraph graph and weight vector.

   If the weights vector is `NULL` the resulting adjacency matrix will be use
   `1` for the weight of all edges.

   See `mxIgraphCreateFullAdj` for the full equivalent. */
mxArray *mxIgraphCreateSparseAdj(igraph_t const *graph,
                                 igraph_vector_t const *weights)
{
  mwSize n_edges = (mwSize)igraph_ecount(graph);
  mwSize n_nodes = (mwSize)igraph_vcount(graph);
  mxArray *p = mxCreateSparse(n_nodes, n_nodes, n_edges, mxREAL);
  double *adj = mxGetDoubles(p);
  mwIndex *jc = mxGetJc(p);
  mwIndex *ir = mxGetIr(p);
  igraph_eit_t eit;
  igraph_integer_t eid;

  igraph_eit_create(graph, igraph_ess_all(IGRAPH_EDGEORDER_TO), &eit);

  mwIndex count = 0;
  mwIndex column, prev_column = 0;
  jc[prev_column] = count;
  while (!IGRAPH_EIT_END(eit)) {
    eid = IGRAPH_EIT_GET(eit);
    column = IGRAPH_TO(graph, eid);
    if (prev_column != column) {
      for (igraph_integer_t i = prev_column; i < column; i++) {
        jc[i + 1] = count;
      }
      prev_column = column;
    }

    ir[count] = (mwIndex)IGRAPH_FROM(graph, eid);
    adj[count] = (double)(weights ? VECTOR(*weights)[eid] : 1);

    IGRAPH_EIT_NEXT(eit);
    count++;
  }

  while (column < n_nodes) {
    jc[column + 1] = count;
    column++;
  }

  igraph_eit_destroy(&eit);

  return p;
}

/* Create a matlab vector for storing the labels of each node from an igraph
   membership partition.

   Increments igraph partition so that the smallest node id is 1 instead of 0.*/
mxArray *mxIgraphCreatePartition(igraph_vector_int_t const *membership)
{
  igraph_integer_t n_nodes = igraph_vector_int_size(membership);
  mxArray *p = mxCreateDoubleMatrix(n_nodes, 1, mxREAL);
  double *mxMembership = mxGetDoubles(p);

  for (igraph_integer_t i = 0; i < n_nodes; i++) {
    mxMembership[(mwIndex)i] = (double)VECTOR(*membership)[i] + 1;
  }

  return p;
}

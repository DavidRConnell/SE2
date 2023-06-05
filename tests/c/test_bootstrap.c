#include <igraph.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "speak_easy_2.h"

int main()
{
  char *graph_name = "karate.gml";
  char graph_path[1000] = "../examples/";
  igraph_t graph;
  options opts = {
    .random_seed = 2,
    .independent_runs = 10,
  };
  igraph_vector_int_t res;
  igraph_vector_int_init(&res, 1);

  strncat(graph_path, graph_name, 999);
  FILE *fptr = fopen(graph_path, "r");

  igraph_read_graph_gml(&graph, fptr);

  speak_easy_2(&graph, NULL, &opts, &res);

  igraph_vector_int_destroy(&res);
  igraph_destroy(&graph);

  return EXIT_SUCCESS;
}

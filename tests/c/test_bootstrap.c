#include <igraph.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "speak_easy_2.h"


int main()
{
  char *graph_name = "lesmis.gml";
  char graph_path[1000] = "../examples/";
  igraph_t graph;
  options opts = {
    .random_seed = 1,
    .independent_runs = 1, // TODO: Change later.
  };
  outputs res;

  strncat(graph_path, graph_name, 999);
  FILE *fptr = fopen(graph_path, "r");

  igraph_read_graph_gml(&graph, fptr);

  speak_easy_2(&graph, NULL, &opts, &res);

  igraph_destroy(&graph);

  return EXIT_SUCCESS;
}

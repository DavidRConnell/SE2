#include <stdio.h>
#include <stdlib.h>
#include <igraph.h>

#include "speak_easy_2.h"

int main()
{
  igraph_t graph;
  options opts = {
    .random_seed = 1,
    .independent_runs = 10,
  };
  outputs res;
  FILE *fptr = fopen("../examples/karate.gml", "r");

  igraph_read_graph_gml(&graph, fptr);

  speak_easy_2(&graph, NULL, &opts, &res);

  igraph_destroy(&graph);

  return EXIT_SUCCESS;
}

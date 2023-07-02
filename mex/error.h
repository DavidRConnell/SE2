#ifndef ERROR_H
#define ERROR_H

#include <igraph_error.h>

igraph_error_handler_t mxIgraphErrorHandlerMex;

/* void mxIgraphErrorHandlerMex(const char *reason, const char *file, */
/*                              int line, igraph_error_t igraph_errno); */
void mxIgraphSetErrorHandler();

#endif

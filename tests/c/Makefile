CC = gcc
CFLAGS = -Wall -Wextra

TOP_DIR = ../..
INCLUDE_DIR = $(TOP_DIR)/include
LIB_DIR = $(TOP_DIR)/lib
SRC_DIR = $(TOP_DIR)/src

memcheck: CFLAGS += -g -O0
debug: CFLAGS += -g3

.PHONY: check clean debug memcheck

test_bootstrap.o: test_bootstrap.c $(INCLUDE_DIR)/speak_easy_2.h
	$(CC) $(CFLAGS) -fopenmp -c -I$(INCLUDE_DIR) $*.c

test_bootstrap: test_bootstrap.o $(LIB_DIR)/libse2.a
	$(CC) $(CFLAGS) -fopenmp -o $@ $^ -L$(LIB_DIR) -lse2 -ligraph -lm

check: test_bootstrap
	./test_bootstrap

memcheck: test_bootstrap
	valgrind --leak-check=yes --track-origins=yes ./test_bootstrap

# Debug dependencies will change as entry points are added or removed.
debug: test_bootstrap

clean:
	-find -type f -not -name '*.c' -not -name 'Makefile' -exec rm {} \;

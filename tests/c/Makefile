CC = gcc
CFLAGS = -Wall -Wextra

TOP_DIR = ../..
INCLUDE_DIR = $(TOP_DIR)/include
SRC_DIR = $(TOP_DIR)/src

memcheck: CFLAGS += -g -O0
debug: CFLAGS += -g3

.PHONY: check clean debug memcheck

test_bootstrap.o: test_bootstrap.c $(INCLUDE_DIR)/speak_easy_2.h
	$(CC) -c $*.c -I$(INCLUDE_DIR) -fopenmp

test_bootstrap: test_bootstrap.o
	$(CC) $(CFLAGS) test_bootstrap.o $(SRC_DIR)/se2_core.o -o $@ -fopenmp -ligraph -lm

check: test_bootstrap
	./test_bootstrap

memcheck: test_bootstrap
	valgrind --leak-check=yes --track-origins=yes ./test_bootstrap

# Debug dependencies will change as entry points are added or removed.
debug: test_bootstrap

clean:
	-find -type f -not -name '*.c' -not -name 'Makefile' -exec rm {} \;

CC = gcc
CFLAGS = -Wall -Wextra

TOP_DIR = ../..
INCLUDE_DIR = $(TOP_DIR)/include
SRC_DIR = $(TOP_DIR)/src

memcheck: CFLAGS = -g -O0

.PHONY: check clean memcheck

test_random.o: test_random.c $(SRC_DIR)/se2_random.h
	$(CC) -c $*.c -fopenmp

test_bootstrap.o: test_bootstrap.c $(INCLUDE)/speak_easy_2.h $(SRC_DIR)/se2_random.h
	$(CC) -c $*.c -fopenmp -ligraph

test_random: test_random.o
	@cd $(SRC_DIR); $(MAKE) se2_random.o
	$(CC) -fopenmp $(CFLAGS) $^ -o $@ -ligraph

test_bootstrap: test_bootstrap.o
	@cd $(SRC_DIR); $(MAKE) speak_easy_2.o se2_random.o
	$(CC) $(CFLAGS) $^ -o $@ -fopenmp -ligraph

check: test_random test_bootstrap
	./test_random
	./test_bootstrap

memcheck: test_bootstrap
	valgrind --leak-check=yes --track-origins=yes ./test_bootstrap

clean:
	-find -type f -not -name '*.c' -not -name 'Makefile' -exec rm {} \;

CC = gcc
CFLAGS = -Wall -Wextra

TEST_DIR = ./tests

.PHONY: all clean test
se_random.o: se_random.c se_random.h
	$(CC) -c $*.c -ligraph

$(TEST_DIR)/test_random: test_random.o se_random.o
	$(CC) -fopenmp $(CFLAGS) $^ -o $@ -ligraph

all: $(TEST_DIR)/test_random

test: $(TEST_DIR)/test_random
	$(TEST_DIR)/test_random

clean:
	-rm *.o
	-rm -rf $(TEST_DIR)
	-mkdir $(TEST_DIR)

CC = gcc
CFLAGS = -Wall -Wextra

INCLUDE_DIR = ../include
LIB_DIR = ../lib
BIN_DIR = ../bin
MEX_DIR = ../mex
MAT_DIR = ../private

# TODO: Change to if statements to detect OS
LIB_EXT = so

debug: CFLAGS += -g3

.PHONY: all libse2 clean

all: $(BIN_DIR)/speak_easy_2
libse2: $(LIB_DIR)/libse2.$(LIB_EXT)

$(BIN_DIR)/speak_easy_2: se2_core.o
	@[ -d $(BIN_DIR) ] || mkdir $(BIN_DIR)
	$(CC) -o $@ -c $^ -fopenmp -ligraph

$(LIB_DIR)/libse2.$(LIB_EXT): se2_core.o
	@[ -d $(LIB_DIR) ] || mkdir $(LIB_DIR)
	$(CC) -shared -o $@ $^ -ligraph

se2_core.o: se2_core.c $(INCLUDE_DIR)/speak_easy_2.h
	$(CC) -c $*.c -I$(INCLUDE_DIR) -fopenmp -ligraph

clean:
	-rm *.o
	-rm -r $(BIN_DIR)

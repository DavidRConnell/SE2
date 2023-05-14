CC = gcc
CFLAGS = -Wall -Wextra

INCLUDE_DIR = ../include
LIB_DIR = ../lib

# TODO: Change to if statements to detect OS
LIB_EXT = so

debug: CFLAGS += -g3

.PHONY: all objs libse2 clean

all: libse2
libse2: $(LIB_DIR)/libse2.$(LIB_EXT)
objs: se2_core.o

$(LIB_DIR)/libse2.$(LIB_EXT): se2_core.o
	@[ -d $(LIB_DIR) ] || mkdir $(LIB_DIR)
	$(CC) -shared -o $@ $^ -fopenmp

se2_core.o: se2_core.c $(INCLUDE_DIR)/speak_easy_2.h
	$(CC) -c $*.c -I$(INCLUDE_DIR) -fopenmp

debug: objs libse2

clean:
	-rm *.o

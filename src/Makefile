CC = gcc
CFLAGS = -Wall -Wextra

INCLUDE_DIR = ../include
LIB_DIR = ../lib

# TODO: Change to if statements to detect OS
LIB_EXT = a

debug: CFLAGS += -g3

ifdef SE2_PRINT_PATH
CFLAGS += -DSE2_PRINT_PATH=$(SE2_PRINT_PATH)
endif

.PHONY: all libse2 clean

all: libse2
libse2: $(LIB_DIR)/libse2.$(LIB_EXT)

$(LIB_DIR)/libse2.$(LIB_EXT): se2_core.o se2_random.o se2_seeding.o se2_modes.o \
			      se2_partitions.o se2_label.o se2_reweight_graph.o
	@[ -d $(LIB_DIR) ] || mkdir $(LIB_DIR)
	ar rcs $@ $^

se2_core.o: se2_core.c se2_random.h se2_seeding.h se2_modes.h \
	    se2_reweight_graph.h $(INCLUDE_DIR)/speak_easy_2.h
	$(CC) $(CFLAGS) -c -fopenmp -I$(INCLUDE_DIR) $*.c

se2_random.o: se2_random.c se2_random.h
	$(CC) $(CFLAGS) -c $*.c

se2_seeding.o: se2_seeding.c se2_seeding.h se2_random.h $(INCLUDE_DIR)/speak_easy_2.h
	$(CC) $(CFLAGS) -c -I$(INCLUDE_DIR) $*.c

se2_reweight_graph.o: se2_reweight_graph.c se2_reweight_graph.h
	$(CC) $(CFLAGS) -c $*.c

se2_modes.o: se2_modes.c se2_modes.h se2_label.h se2_partitions.h \
	     $(INCLUDE_DIR)/speak_easy_2.h
	$(CC) $(CFLAGS) -c -I$(INCLUDE_DIR) $*.c

se2_label.o: se2_label.c se2_label.h se2_partitions.h
	$(CC) $(CFLAGS) -c $*.c

se2_partitions.o: se2_partitions.c se2_partitions.h se2_random.h
	$(CC) $(CFLAGS) -c $*.c

debug: libse2

clean:
	-rm *.o
	-rm -r $(LIB_DIR)

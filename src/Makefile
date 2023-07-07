CC = gcc

INCLUDE_DIR = ../include
LIB_DIR = ../lib
INSTALL_DIR = $(HOME)/.local/share

CFLAGS = -Wall -Wextra -fPIC
CPPFLAGS = -I$(INCLUDE_DIR) -I$(INSTALL_DIR)/igraph/include
LDFLAGS = -L$(INSTALL_DIR)/igraph/lib

vpath %.h $(INCLUDE_DIR)

# TODO: Change to if statements to detect OS
LIB_EXT = so

debug: CFLAGS += -g3 -O0

ifdef SE2_PRINT_PATH
CFLAGS += -DSE2_PRINT_PATH=$(SE2_PRINT_PATH)
endif

.PHONY: all
all: libse2

.PHONY: libse2
libse2: lib
libse2: $(LIB_DIR)/libse2.$(LIB_EXT)

$(LIB_DIR)/libse2.$(LIB_EXT): se2_core.o se2_random.o se2_seeding.o se2_modes.o
$(LIB_DIR)/libse2.$(LIB_EXT): se2_partitions.o se2_label.o se2_reweight_graph.o
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -shared -fopenmp \
		$^ -o $@ -ligraph -lm

.PHONY: lib
lib:
	@[ -d $(LIB_DIR) ] || mkdir $(LIB_DIR)

se2_core.o: se2_random.h se2_seeding.h se2_modes.h se2_reweight_graph.h \
	speak_easy_2.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -fopenmp $*.c

se2_random.o: se2_random.h
se2_seeding.o: se2_seeding.h se2_random.h speak_easy_2.h
se2_reweight_graph.o: se2_reweight_graph.h
se2_modes.o: se2_modes.h se2_label.h se2_partitions.h speak_easy_2.h
se2_label.o: se2_label.h se2_partitions.h
se2_partitions.o: se2_partitions.h se2_random.h
%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $*.c

.PHONY: debug
debug: libse2

.PHONY: install
install: libse2
	@[ -d $(INSTALL_DIR)/SE2 ] || mkdir -p $(INSTALL_DIR)/SE2/lib && \
		mkdir -p $(INCLUDE_DIR)/SE2/include
	-mv $^ $(INSTALL_DIR)/SE2/lib
	-cp $(INCLUDE_DIR)/* $(INSTALL_DIR)/SE2/lib

.PHONY: clean
clean:
	-rm *.o
	-rm -r $(LIB_DIR)
	-rm -r $(INSTALL_DIR)/SE2

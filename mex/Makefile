CC = gcc
MEXFLAGS = -outdir $(MAT_DIR) -I$(INCLUDE_DIR) -L$(LIB_DIR) -R2018a

INCLUDE_DIR = ../include
LIB_DIR = ../lib
SRC_DIR = ../src
MAT_DIR = ../private

# TODO: Change to if statements to detect OS
MEX_EXT = mexa64

.PHONY: all clean

all: $(MAT_DIR)/speak_easy_2_mex.$(MEX_EXT) $(MAT_DIR)/discreate_nmi.$(MEX_EXT)

$(MAT_DIR)/speak_easy_2_mex.$(MEX_EXT): speak_easy_2.c \
					$(INCLUDE_DIR)/speak_easy_2.h
	@cd $(SRC_DIR); $(MAKE) libse2
	mex $(MEXFLAGS) -output speak_easy_2_mex COPTIMFLAGS='-fopenmp' -ligraph -lse2 \
	  speak_easy_2.c mex_igraph.c

$(MAT_DIR)/discreate_nmi.$(MEX_EXT): discrete_nmi.c mex_igraph.c mex_igraph.h
	mex $(MEXFLAGS) -ligraph discrete_nmi.c mex_igraph.c

clean:
	-rm $(MAT_DIR)/*.$(MEX_EXT)

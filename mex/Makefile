INCLUDE_DIR = ../include
LIB_DIR = ../lib
MAT_DIR = ../private

COPTIMFLAGS = "COPTIMFLAGS=-O3 -fwrapv -fopenmp -DNDEBUG"
MEXFLAGS = -outdir $(MAT_DIR) -I$(INCLUDE_DIR) -L$(LIB_DIR) -R2018a

# TODO: Change to if statements to detect OS
MEX_EXT = mexa64

.PHONY: all clean

all: $(MAT_DIR)/SpeakEasy2_mex.$(MEX_EXT) \
	$(MAT_DIR)/leiden.$(MEX_EXT) \
	$(MAT_DIR)/modularity.$(MEX_EXT) \
	$(MAT_DIR)/optimal_modularity.$(MEX_EXT) \
	$(MAT_DIR)/discrete_nmi.$(MEX_EXT) \
	$(MAT_DIR)/read_gml.$(MEX_EXT)

$(MAT_DIR)/SpeakEasy2_mex.$(MEX_EXT): SpeakEasy2_mex.c mex_igraph.c \
	mex_igraph.h $(INCLUDE_DIR)/speak_easy_2.h

	mex $(MEXFLAGS) $(COPTIMFLAGS) LDFLAGS="-fopenmp" \
	-output SpeakEasy2_mex -ligraph -lse2 SpeakEasy2_mex.c mex_igraph.c

$(MAT_DIR)/%.$(MEX_EXT): %.c mex_igraph.c mex_igraph.h
	mex $(MEXFLAGS) $*.c mex_igraph.c -ligraph

clean:
	-rm $(MAT_DIR)/*.$(MEX_EXT)

INCLUDE_DIR = ../include
LIB_DIR = ../lib
MAT_DIR = ../private
IGRAPH_DIR = ../+igraph

COPTIMFLAGS = "COPTIMFLAGS=-O3 -fwrapv -fopenmp -DNDEBUG"
MEXFLAGS = -I$(INCLUDE_DIR) -L$(LIB_DIR) -L$(LD_LIBRARY_PATH) -R2018a

# TODO: Change to if statements to detect OS
MEX_EXT = mexa64

.PHONY: all clean

all: $(IGRAPH_DIR)/SpeakEasy2.$(MEX_EXT) \
	$(IGRAPH_DIR)/leiden.$(MEX_EXT) \
	$(IGRAPH_DIR)/modularity.$(MEX_EXT) \
	$(IGRAPH_DIR)/optimal_modularity.$(MEX_EXT) \
	$(MAT_DIR)/discrete_nmi.$(MEX_EXT) \
	$(IGRAPH_DIR)/read_gml.$(MEX_EXT)

$(IGRAPH_DIR)/SpeakEasy2.$(MEX_EXT): SpeakEasy2.c mex_igraph.c \
	mex_igraph.h $(INCLUDE_DIR)/speak_easy_2.h

	mex -outdir $(IGRAPH_DIR) $(MEXFLAGS) $(COPTIMFLAGS) \
		LDFLAGS="-fopenmp" SpeakEasy2.c mex_igraph.c -ligraph -lse2

$(MAT_DIR)/%.$(MEX_EXT): %.c mex_igraph.c mex_igraph.h
	mex -outdir $(MAT_DIR) $(MEXFLAGS) $*.c mex_igraph.c -ligraph

$(IGRAPH_DIR)/%.$(MEX_EXT): %.c mex_igraph.c mex_igraph.h
	mex -outdir $(IGRAPH_DIR) $(MEXFLAGS) $*.c mex_igraph.c -ligraph

clean:
	-rm $(MAT_DIR)/*.$(MEX_EXT)

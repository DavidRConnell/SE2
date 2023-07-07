INCLUDE_DIR = ../include
MAT_DIR = ../private
INSTALL_DIR = $(HOME)/.local/share
IGRAPH_DIR = ../+igraph

COPTIMFLAGS = "COPTIMFLAGS=-O3 -fwrapv -fopenmp -DNDEBUG"
CFLAGS = "CFLAGS=-Wno-format-security -Wno-format"
LDFLAGS = "LDFLAGS=-Wl,-rpath=$(INSTALL_DIR)/igraph/lib,-rpath=$(INSTALL_DIR)/SE2/lib,-z=noexecstack -fopenmp"
MEXFLAGS = -I$(INCLUDE_DIR) -I$(INSTALL_DIR)/igraph/include -L$(INSTALL_DIR)/SE2/lib -L$(INSTALL_DIR)/igraph/lib -R2018a $(CFLAGS) $(LDFLAGS)

# TODO: Change to if statements to detect OS
MEX_EXT = mexa64

.PHONY: all clean

all: $(IGRAPH_DIR)/SpeakEasy2.$(MEX_EXT) \
	$(IGRAPH_DIR)/leiden.$(MEX_EXT) \
	$(IGRAPH_DIR)/modularity.$(MEX_EXT) \
	$(IGRAPH_DIR)/optimal_modularity.$(MEX_EXT) \
	$(MAT_DIR)/discrete_nmi.$(MEX_EXT) \
	$(IGRAPH_DIR)/read_gml.$(MEX_EXT) \
	$(IGRAPH_DIR)/write_edgelist.$(MEX_EXT) \
	$(IGRAPH_DIR)/similarity.$(MEX_EXT)

$(IGRAPH_DIR)/SpeakEasy2.$(MEX_EXT): SpeakEasy2.c mex_igraph.c error.c error.h \
	mex_igraph.h $(INCLUDE_DIR)/speak_easy_2.h $(INSTALL_DIR)/SE2/lib/libse2.so

	mex -outdir $(IGRAPH_DIR) $(MEXFLAGS) $(COPTIMFLAGS) \
		SpeakEasy2.c mex_igraph.c error.c -ligraph -lse2

$(MAT_DIR)/%.$(MEX_EXT): %.c mex_igraph.c error.c mex_igraph.h error.h
	mex -outdir $(MAT_DIR) $(MEXFLAGS) $*.c mex_igraph.c error.c -ligraph

$(IGRAPH_DIR)/%.$(MEX_EXT): %.c mex_igraph.c error.c mex_igraph.h error.h
	mex -outdir $(IGRAPH_DIR) $(MEXFLAGS) $*.c mex_igraph.c error.c -ligraph

clean:
	-rm $(MAT_DIR)/*.$(MEX_EXT)
	-rm $(IGRAPH_DIR)/*.$(MEX_EXT)

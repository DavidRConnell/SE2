SRC_DIR = src
MEX_DIR = mex
INCLUDE_DIR = include
TEST_DIR = tests/c

.PHONY: all cli check memcheck clean

all:
	@cd $(MEX_DIR); $(MAKE) all

cli:
	@cd $(SRC_DIR); $(MAKE) all

check:
	@cd $(TEST_DIR); $(MAKE) check

memcheck:
	@cd $(TEST_DIR); $(MAKE) memcheck

clean:
	@cd $(SRC_DIR); $(MAKE) clean
	@cd $(MEX_DIR); $(MAKE) clean
	@cd $(TEST_DIR); $(MAKE) clean

SRC_DIR = src
MEX_DIR = mex
INCLUDE_DIR = include
TEST_DIR = tests/c

.PHONY: all mex check lib install debug memcheck clean

all: mex
	@cd $(SRC_DIR); $(MAKE) all

lib:
	@cd $(SRC_DIR); $(MAKE) libse2

install:
	@cd $(SRC_DIR); $(MAKE) install

mex: install
	@cd $(MEX_DIR); $(MAKE) all

check: lib
	@cd $(TEST_DIR); $(MAKE) check

debug:
	@cd $(SRC_DIR); $(MAKE) debug
	@cd $(TEST_DIR); $(MAKE) debug

memcheck:
	@cd $(TEST_DIR); $(MAKE) memcheck

clean:
	@cd $(SRC_DIR); $(MAKE) clean
	@cd $(MEX_DIR); $(MAKE) clean
	@cd $(TEST_DIR); $(MAKE) clean

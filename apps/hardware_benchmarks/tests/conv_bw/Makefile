#### Halide flags
HALIDE_BIN_PATH := ../../../..
HALIDE_SRC_PATH := ../../../..
HALIDE_DISTRIB_PATH := ../../../../distrib

include ../../hw_support/Makefile.inc

#### HLS flags
#include ../hls_support/Makefile.inc

#### Common CoreIR targets
#### try make {design_top.json, out.png, graph.png, test, update_golden}
#include ../hls_support/coreir.mk

# Use halide compiler to create design.
pipeline: pipeline.cpp
	$(CXX) $(CXXFLAGS) -Wall -g $^ $(LIB_HALIDE) -o $@ $(LDFLAGS) -ltinfo

pipeline_hls.cpp hls_target.cpp pipeline_native.o design_top.json design_top.txt: pipeline
	HL_DEBUG_CODEGEN=0 ./pipeline

# Test if app works, using cached result if json design matches golden.
test:
	@-$(MAKE) -s pipeline $(OUTPUT_REDIRECTION)
	@-$(MAKE) -s design_top.json $(OUTPUT_REDIRECTION)
	@-$(MAKE) -s run $(OUTPUT_REDIRECTION)
	@if [ -f "passed.md5" ]; then \
		md5sum -c --status passed.md5; \
		EXIT_CODE=$$?; \
		if [[ $$EXIT_CODE = "0" ]]; then \
			printf "%-15s \033[0;32m%s\033[0m\n" $(APPNAME) "PASSED"; \
		else \
			rm -f passed.md5; \
			$(MAKE) -s testrun; \
		fi \
	elif [ -f "failed.md5" ]; then \
		md5sum -c --status failed.md5; \
		EXIT_CODE=$$?; \
		if [[ $$EXIT_CODE = "0" ]]; then \
			printf "%-15s \033[0;31m%s\033[0m\n" $(APPNAME) "FAILED" && exit 1; \
		else \
			rm -f failed.md5; \
			$(MAKE) -s testrun; \
		fi \
	else \
		$(MAKE) -s testrun; \
	fi

run: run.cpp pipeline_hls.cpp pipeline_native.o hls_target.cpp
	$(CXX) $(CXXFLAGS) -O1 -DNDEBUG $(HLS_CXXFLAGS) -g -Wall -Werror $^ -lpthread -ldl $(LIB_HALIDE) -o $@ $(PNGFLAGS) $(LDFLAGS)

out.png: run input.png design_top.json
	./run input.png quick
out_coreir.png: run input.png design_top.json
	./run input.png all

graph.png: design_top.txt
	dot -Tpng design_top.txt > graph.png

halide compiler:
	$(MAKE) -C ../../../../

clean:
	rm -f pipeline run out.png
	rm -f pipeline_native.* pipeline_coreir.*
	rm -f coreir_target.*
	rm -f design_top.* design_prepass.json design_golden.json design_flattened.json
	rm -f graph.png


.PHONY: all
all: out.png

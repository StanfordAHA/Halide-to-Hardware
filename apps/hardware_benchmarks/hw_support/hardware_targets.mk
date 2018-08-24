# Usage:
#  make all:       compiles all code
#       generator: create Halide generator
#       design:    create cpu design
#       run:       run cpu design with image
#       compare:   compare two output images
#       eval:      evaluate runtime
#       clean:     remove bin directory


# define defulats to environment variables
BIN ?= bin
HWSUPPORT ?= ../../hw_support

# set default to TESTNAME which forces failure
TESTNAME ?= undefined_testname

default: all
all: $(BIN)/process

$(HWSUPPORT)/$(BIN)/hardware_process_helper.o: $(HWSUPPORT)/hardware_process_helper.cpp
	@-mkdir -p $(HWSUPPORT)/$(BIN)
	$(CXX) $(CXXFLAGS) -c $< -o $@


generator $(BIN)/$(TESTNAME).generator: $(TESTNAME)_generator.cpp $(GENERATOR_DEPS)
	@-mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) -g -fno-rtti $(filter-out %.h,$^) -o $@ $(LDFLAGS)

design design-cpu $(BIN)/$(TESTNAME).a: $(BIN)/$(TESTNAME).generator
	@-mkdir -p $(BIN)
	$^ -g $(TESTNAME) -o $(BIN) -f $(TESTNAME) target=$(HL_TARGET)

$(BIN)/process: process.cpp $(BIN)/$(TESTNAME).a $(HWSUPPORT)/$(BIN)/hardware_process_helper.o
	@-mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) -I$(BIN) -I$(HWSUPPORT) -Wall -O3 $^ -o $@ $(LDFLAGS) $(IMAGE_IO_FLAGS)

image image-cpu: $(BIN)/process
	@-mkdir -p $(BIN)
	$(BIN)/process image

run run-cpu: $(BIN)/process
	@-mkdir -p $(BIN)
	$(BIN)/process run input.png

eval eval-cpu: $(BIN)/process
	@-mkdir -p $(BIN)
	$(BIN)/process eval input.png


clean:
	rm -rf $(BIN)

test: run

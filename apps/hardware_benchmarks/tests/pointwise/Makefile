include ../../hw_support/Makefile.inc

BIN ?= bin

all: $(BIN)/process

$(BIN)/pointwise.generator: pointwise_generator.cpp $(GENERATOR_DEPS)
	@-mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) -g -fno-rtti $(filter-out %.h,$^) -o $@ $(LDFLAGS)

$(BIN)/pointwise.a: $(BIN)/pointwise.generator
	@-mkdir -p $(BIN)
	$^ -g pointwise -o $(BIN) -f pointwise target=$(HL_TARGET)

$(BIN)/process: process.cpp $(BIN)/pointwise.a 
	@-mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) -I$(BIN) -Wall -O3 $^ -o $@ $(LDFLAGS)

run: $(BIN)/process
	@-mkdir -p $(BIN)
	$(BIN)/process

clean:
	rm -rf $(BIN)

test: run

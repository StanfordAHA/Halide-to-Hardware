include ../../hw_support/Makefile.inc

BIN ?= bin

all: $(BIN)/process

$(BIN)/harris.generator: harris_generator.cpp $(GENERATOR_DEPS)
	@-mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) -g -fno-rtti $(filter-out %.h,$^) -o $@ $(LDFLAGS)

$(BIN)/harris.a: $(BIN)/harris.generator
	@-mkdir -p $(BIN)
	$^ -g harris -o $(BIN) -f harris target=$(HL_TARGET)

$(BIN)/process: process.cpp $(BIN)/harris.a 
	@-mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) -I$(BIN) -Wall -O3 $^ -o $@ $(LDFLAGS)

run: $(BIN)/process
	@-mkdir -p $(BIN)
	$(BIN)/process

clean:
	rm -rf $(BIN)

test: run

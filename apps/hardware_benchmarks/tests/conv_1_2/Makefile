include ../../hw_support/Makefile.inc

BIN ?= bin

all: $(BIN)/process

$(BIN)/conv_1_2.generator: conv_1_2_generator.cpp $(GENERATOR_DEPS)
	@-mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) -g -fno-rtti $(filter-out %.h,$^) -o $@ $(LDFLAGS)

$(BIN)/conv_1_2.a: $(BIN)/conv_1_2.generator
	@-mkdir -p $(BIN)
	$^ -g conv_1_2 -o $(BIN) -f conv_1_2 target=$(HL_TARGET)

$(BIN)/process: process.cpp $(BIN)/conv_1_2.a 
	@-mkdir -p $(BIN)
	$(CXX) $(CXXFLAGS) -I$(BIN) -Wall -O3 $^ -o $@ $(LDFLAGS)

run: $(BIN)/process
	@-mkdir -p $(BIN)
	$(BIN)/process

clean:
	rm -rf $(BIN)

test: run

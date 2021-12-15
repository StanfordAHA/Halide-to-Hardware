SHELL = /bin/bash
HALIDE_DEBUG_REDIRECT ?= &>/dev/null

default: all

# fp_arith should be included eventually
TRAVIS_TESTS := arith sshift conv_4_4 \
								inout_cgraconfig up_sample rom
travis := $(TRAVIS_TESTS)
travis: $(TRAVIS_TESTS)

# all directories (just text) excluding a particular list
ALL_APPS := $(subst /,, $(dir $(wildcard */.)))
ALL_APPS := $(filter-out hls_support merged_unit_tests tools, $(ALL_APPS))

### app suites include: ###
APP_SUITES := ops fpops conv fpconv inout mem bc matrix travis

### operator tests ###
OP_TESTS := absolute arith bitwise boolean_ops divide multiply equal ternary \
            scomp sminmax sshift ucomp uminmax ushift \
            counter
ops := $(OP_TESTS)
op_tests op-tests ops: $(OP_TESTS)

### floating point operator tests ###
FP_OP_TESTS := fp_arith fp_comp fp_round fp_rectifier \
               fp_exp fp_invtrig fp_trig
fpops := $(FP_OP_TESTS)
fp_op_tests fp-op-tests fpops: $(FP_OP_TESTS)

### convolutions ###
CONV_TESTS := conv_1_1 conv_1_2 conv_2_1  conv_2_2 conv_3_1 \
              conv_3_3 conv_4_4 conv_5_5 conv_6_6 conv_7_7  \
              conv_chain conv_multi
conv := $(CONV_TESTS)
conv_tests conv-tests convs conv: $(CONV_TESTS)

### floating point convolutions ###
FP_CONV_TESTS := fp_pointwise fp_conv_1_1 fp_conv_2_2 fp_conv_3_3  \
                 fp_conv_4_4 fp_conv_5_5 fp_conv_6_6 fp_conv_7_7
fpconv := $(FP_CONV_TESTS)
fp_conv_tests fp-conv-tests fpconvs: $(FP_CONV_TESTS)

### accelerator interface tests ###
INOUT_TESTS := inout_16bit inout_onebit inout_fp inout_cgraconfig \
							 conv_2input tap_param casts
inout := $(INOUT_TESTS)
inout_tests inout-tests inouts inout: $(INOUT_TESTS)

### memory tile tests ###
MEM_TESTS := rom fifo_test \
             conv_rolled conv_2x \
						 mem_unbuffered \
             strided_conv \
             down_sample up_sample \
             db_3_3 \
             accumulation histogram multi_channel_conv \
             two_level_memory three_level_memory \
             pond three_level_pond pond_benchmark complex_mem_pond \
						 glb_channel_reduction

mem := $(MEM_TESTS)
mem_tests mem-tests mems mem: $(MEM_TESTS)

### boundary condition tests ###
BC_TESTS := bc_const bc_repeat bc_mirror
bc := $(BC_TESTS)
bc boundary: $(BC_TESTS)

### matrix multiplication tests ###
MATRIX_TESTS := matmul rnn_layer lstm_layer attention
matrix := $(MATRIX_TESTS)
matrix mat: $(MATRIX_TESTS)

### generator tests ###
GEN_TESTS := conv_gen random_pipeline


ALL_APPS_SORTED := $(OP_TESTS) $(FP_OP_TESTS) $(CONV_TESTS) \
                   $(FP_CONV_TESTS) $(INOUT_TESTS) $(MEM_TESTS) \
                   $(BC_TESTS) $(MATMUL_TESTS) $(GEN_TESTS) $(MATRIX_TESTS)

NOT_CLASSIFIED_APPS := $(filter-out $(ALL_TESTS_SORTED), $(ALL_TESTS))


# Define targets for running each app and suite including:
#  $(APP)-cpu:               create cpu design files without running
#  $(APP)-clockwork:         create clockwork files without running
#  $(APP)-compare-clockwork: run cpu and clockwork and compare output images
#  $(APP)-check:             see what files were successfully generated
#  $(APP)-golden:            copy generated files to golden directory
#  $(APP)-clean:             remove all generated files from the app
include ../include.mk

SHELL = /bin/bash
HALIDE_DEBUG_REDIRECT ?= &>/dev/null

default: all
travis testtravis: arith conv_3_3 

# all directories (just text) excluding a particular list
ALL_APPS := $(subst /,, $(dir $(wildcard */.)))
ALL_APPS := $(filter-out hls_support merged_unit_tests tools, $(ALL_APPS))

### app suites include: ###
# ops fpops convs fpconvs inouts mems
APP_SUITES := ops fpops conv fpconv inout mem

### operator tests ###
# OP_TESTS := absolute arith bitwise boolean_ops equal ternary \
#             scomp sminmax sshift ucomp uminmax ushift \
#             counter
OP_TESTS := absolute arith bitwise boolean_ops \
            counter equal 
ops := $(OP_TESTS)
op_tests op-tests ops: $(OP_TESTS)

### floating point operator tests ###
# FP_OP_TESTS := fp_arith fp_comp fp_round fp_rectifier \
#                fp_exp fp_invtrig fp_trig
FP_OP_TESTS := fp_arith
fpops := $(FP_OP_TESTS)
fp_op_tests fp-op-tests fpops: $(FP_OP_TESTS)

### convolutions ###
CONV_TESTS := conv_1_1 conv_1_2 conv_2_1  conv_2_2 conv_3_1 \
              conv_3_3 conv_4_4 conv_5_5 conv_6_6 conv_7_7  \
              conv_multi strided_conv conv_rolled
conv := $(CONV_TESTS)
conv_tests conv-tests convs conv: $(CONV_TESTS)

### floating point convolutions ###
# FP_CONV_TESTS := fp_pointwise fp_conv_1_1 fp_conv_2_2 fp_conv_3_3  \
#                  fp_conv_4_4 fp_conv_5_5 fp_conv_6_6 fp_conv_7_7
# fpconv := $(FP_CONV_TESTS)
fp_conv_tests fp-conv-tests fpconvs: $(FP_CONV_TESTS)
FP_CONV_TESTS := fp_conv_3_3

### accelerator interface tests ###
# INOUT_TESTS := conv_2input inout_fp inout_onebit tap_param
INOUT_TESTS := casts conv_2input inout_fp
inout := $(INOUT_TESTS)
inout_tests inout-tests inouts inout: $(INOUT_TESTS)

### memory tile tests ###
# MEM_TESTS := rom fifo_test \
#              conv_rolled conv_2x \
#              down_sample up_sample strided_conv \
#              conv_3_3 db_3_3 \
#              accumulation histogram multi_channel_conv \
#              two_level_memory three_level_memory pond
MEM_TESTS := fifo_test \
             conv_2x \
             strided_conv \
             down_sample \
             db_3_3 \
             accumulation histogram multi_channel_conv \
             two_level_memory three_level_memory pond

mem := $(MEM_TESTS)
mem_tests mem-tests mems mem: $(MEM_TESTS)

### generator tests ###
# random_pipeline
GEN_TESTS := conv_gen 


ALL_APPS_SORTED := $(OP_TESTS) $(FP_OP_TESTS) $(CONV_TESTS) \
                   $(FP_CONV_TESTS) $(INOUT_TESTS) $(MEM_TESTS) \
                   $(GEN_TESTS)

NOT_CLASSIFIED_APPS := $(filter-out $(ALL_TESTS_SORTED), $(ALL_TESTS))


# Define targets for running each app and suite including:
#  $(APP)-cpu:               create cpu design files without running
#  $(APP)-clockwork:         create clockwork files without running
#  $(APP)-compare-clockwork: run cpu and clockwork and compare output images
#  $(APP)-check:             see what files were successfully generated
#  $(APP)-golden:            copy generated files to golden directory
#  $(APP)-clean:             remove all generated files from the app
include ../include.mk

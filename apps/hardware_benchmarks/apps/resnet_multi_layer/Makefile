# Usage:
#  make all:                    compiles all code without running
#       generator:              create Halide generator
#       design:                 create cpu design
#       design-clockwork:       create clockwork design
#       image:                  create an image with random data
#       run:                    run cpu design with image
#       run-clockwork:          run clockwork design with image
#       compare:                compare two output images
#       test:                   run and compare to cpu output
#       eval:                   evaluate runtime
#       clean:                  remove bin directory

###############################################################################
# MODIFICATION: NONE
#------------------------------------------------------------------------------
# Makefile parameter/variable declarations
#
###############################################################################

include ../../hw_support/Makefile.inc



###############################################################################
# MODIFICATION: OPTIONAL
#------------------------------------------------------------------------------
# App-specific info
#
# TESTNAME                      : name of the app
# USE_CORE_IR_VALID             : whether to generate valid signal for coreir codegen
# HL_TARGET                     : Halide target
# RDAI_PLATFORM_RUNTIME         : RDAI platform runtime to use 
###############################################################################

TESTNAME                        = resnet_multi_layer
USE_COREIR_VALID                = 1
HL_TARGET                       = host-x86-64
RDAI_PLATFORM_RUNTIME           = clockwork_sim
EXT                             = mat
#HALIDE_GEN_ARGS                 := kernel.size=1 $(HALIDE_GEN_ARGS)
NLAYERS                         ?= 2
NLAYERS_PLUS_ONE                = $(shell expr $(NLAYERS) + 1)
HALIDE_GEN_ARGS                 = n_layers=$(NLAYERS)
HALIDE_GEN_SIZE_ARGS            = kernel.size=$(NLAYERS)
#HALIDE_GEN_ARGS                 = kernel.size=$(NLAYERS) pad.size=$(NLAYERS) ksize.size=$(NLAYERS) stride.size=$(NLAYERS)
#HALIDE_GEN_ARGS                 += n_c.size=$(NLAYERS_PLUS_ONE) n_layers=$(NLAYERS)

# Set enviroment variable to set these:
#  HALIDE_GEN_ARGS="ksize=3 stride=2 k_ic=10 k_oc=3 in_img=64"

# resnet layers include:
#  conv1:   HALIDE_GEN_ARGS="in_img=224 pad=3 ksize=7 stride=2 k_ic=3   k_oc=64"
#  conv2_x: HALIDE_GEN_ARGS="in_img=56  pad=1 ksize=3 stride=1 k_ic=64  k_oc=64"
#  conv3_1: HALIDE_GEN_ARGS="in_img=56  pad=1 ksize=3 stride=2 k_ic=64  k_oc=128"
#  conv3_x: HALIDE_GEN_ARGS="in_img=28  pad=1 ksize=3 stride=1 k_ic=128 k_oc=128"
#  conv4_1: HALIDE_GEN_ARGS="in_img=28  pad=1 ksize=3 stride=2 k_ic=128 k_oc=256"
#  conv4_x: HALIDE_GEN_ARGS="in_img=14  pad=1 ksize=3 stride=1 k_ic=256 k_oc=256"
#  conv5_1: HALIDE_GEN_ARGS="in_img=14  pad=1 ksize=3 stride=2 k_ic=256 k_oc=512"
#  conv5_x: HALIDE_GEN_ARGS="in_img=7   pad=1 ksize=3 stride=1 k_ic=512 k_oc=512"


#  conv1:   HALIDE_GEN_ARGS="in_img=224 pad=3 ksize=7 stride=2 k_ic=3 k_oc=8"
#  conv2_x: HALIDE_GEN_ARGS="in_img=56  pad=1 ksize=3 stride=1 k_ic=8 k_oc=8"
#  conv3_1: HALIDE_GEN_ARGS="in_img=56  pad=1 ksize=3 stride=2 k_ic=8 k_oc=8"
#  conv3_x: HALIDE_GEN_ARGS="in_img=28  pad=1 ksize=3 stride=1 k_ic=8 k_oc=8"
#  conv4_1: HALIDE_GEN_ARGS="in_img=28  pad=1 ksize=3 stride=2 k_ic=8 k_oc=8"
#  conv4_x: HALIDE_GEN_ARGS="in_img=14  pad=1 ksize=3 stride=1 k_ic=8 k_oc=8"
#  conv5_1: HALIDE_GEN_ARGS="in_img=14  pad=1 ksize=3 stride=2 k_ic=8 k_oc=8"
#  conv5_x: HALIDE_GEN_ARGS="in_img=7   pad=1 ksize=3 stride=1 k_ic=8 k_oc=8"



###############################################################################
# MODIFICATION : NONE
#------------------------------------------------------------------------------
# Include hardwrae build targets
#
###############################################################################

include ../../hw_support/hardware_targets.mk

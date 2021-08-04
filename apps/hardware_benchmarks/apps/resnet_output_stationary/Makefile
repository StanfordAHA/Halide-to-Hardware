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

TESTNAME                        = resnet_output_stationary
USE_COREIR_VALID                = 1
HL_TARGET                       = host-x86-64
RDAI_PLATFORM_RUNTIME           = clockwork_sim
EXT                             = mat

# Set enviroment variable to set these:
# HALIDE_GEN_ARGS ?= ksize=3 stride=2 k_ic=10 k_oc=3 in_img=64

# resnet layers include:
#  conv1:   HALIDE_GEN_ARGS="in_img=224 pad=3 ksize=7 stride=2 n_ic=3   n_oc=64  k_ic=3"
#  conv2_x: HALIDE_GEN_ARGS="in_img=56  pad=1 ksize=3 stride=1 n_ic=64  n_oc=64"
#  conv3_1: HALIDE_GEN_ARGS="in_img=56  pad=1 ksize=3 stride=2 n_ic=64  n_oc=128"
#  conv3_x: HALIDE_GEN_ARGS="in_img=28  pad=1 ksize=3 stride=1 n_ic=128 n_oc=128"
#  conv4_1: HALIDE_GEN_ARGS="in_img=28  pad=1 ksize=3 stride=2 n_ic=128 n_oc=256"
#  conv4_x: HALIDE_GEN_ARGS="in_img=14  pad=1 ksize=3 stride=1 n_ic=256 n_oc=256"
#  conv5_1: HALIDE_GEN_ARGS="in_img=14  pad=1 ksize=3 stride=2 n_ic=256 n_oc=512"
#  conv5_x: HALIDE_GEN_ARGS="in_img=7   pad=1 ksize=3 stride=1 n_ic=512 n_oc=512"


#  conv1:   HALIDE_GEN_ARGS="in_img=224 pad=3 ksize=7 stride=2 n_ic=3 n_oc=8"
#  conv2_x: HALIDE_GEN_ARGS="in_img=56  pad=1 ksize=3 stride=1 n_ic=8 n_oc=8"
#  conv3_1: HALIDE_GEN_ARGS="in_img=56  pad=1 ksize=3 stride=2 n_ic=8 n_oc=8"
#  conv3_x: HALIDE_GEN_ARGS="in_img=28  pad=1 ksize=3 stride=1 n_ic=8 n_oc=8"
#  conv4_1: HALIDE_GEN_ARGS="in_img=28  pad=1 ksize=3 stride=2 n_ic=8 n_oc=8"
#  conv4_x: HALIDE_GEN_ARGS="in_img=14  pad=1 ksize=3 stride=1 n_ic=8 n_oc=8"
#  conv5_1: HALIDE_GEN_ARGS="in_img=14  pad=1 ksize=3 stride=2 n_ic=8 n_oc=8"
#  conv5_x: HALIDE_GEN_ARGS="in_img=7   pad=1 ksize=3 stride=1 n_ic=8 n_oc=8"



###############################################################################
# MODIFICATION : NONE
#------------------------------------------------------------------------------
# Include hardwrae build targets
#
###############################################################################

include ../../hw_support/hardware_targets.mk

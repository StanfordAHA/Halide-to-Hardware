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

TESTNAME                        = maxpooling_dense_rv_fp
USE_COREIR_VALID                = 1
HL_TARGET                       = host-x86-64-bfloat_hardware
# HL_TARGET                       = host-x86-64
RDAI_PLATFORM_RUNTIME           = clockwork_sim
EXT                             = mat

# Set enviroment variable to set these:
#  HALIDE_GEN_ARGS="ksize=3 stride=2 k_ic=10 k_oc=3 in_img=64"

# mobilenet layers include:
#  conv1:   HALIDE_GEN_ARGS="in_img=112 pad=1 ksize=3 stride=1 k_ic=32   k_oc=64"
#  conv2:   HALIDE_GEN_ARGS="in_img=112 pad=1 ksize=3 stride=2 k_ic=64   k_oc=128"
#  conv3:   HALIDE_GEN_ARGS="in_img=56  pad=1 ksize=3 stride=1 k_ic=128  k_oc=128"
#  conv4:   HALIDE_GEN_ARGS="in_img=56  pad=1 ksize=3 stride=2 k_ic=128  k_oc=256"
#  conv5:   HALIDE_GEN_ARGS="in_img=28  pad=1 ksize=3 stride=1 k_ic=256  k_oc=256"
#  conv6:   HALIDE_GEN_ARGS="in_img=28  pad=1 ksize=3 stride=2 k_ic=256  k_oc=512"
#  conv7:   HALIDE_GEN_ARGS="in_img=14  pad=1 ksize=3 stride=1 k_ic=512  k_oc=512"
#  conv8:   HALIDE_GEN_ARGS="in_img=14  pad=1 ksize=3 stride=2 k_ic=512  k_oc=1024"
#  conv9:   HALIDE_GEN_ARGS="in_img=7   pad=4 ksize=3 stride=2 k_ic=1024 k_oc=1024"


#  conv1:   HALIDE_GEN_ARGS="in_img=112 pad=1 ksize=3 stride=1 k_ic=4 k_oc=8"
#  conv2:   HALIDE_GEN_ARGS="in_img=112 pad=1 ksize=3 stride=2 k_ic=4 k_oc=8"
#  conv3:   HALIDE_GEN_ARGS="in_img=56  pad=1 ksize=3 stride=1 k_ic=4 k_oc=16"
#  conv4:   HALIDE_GEN_ARGS="in_img=56  pad=1 ksize=3 stride=2 k_ic=4 k_oc=8"
#  conv5:   HALIDE_GEN_ARGS="in_img=28  pad=1 ksize=3 stride=1 k_ic=4 k_oc=8"
#  conv6:   HALIDE_GEN_ARGS="in_img=28  pad=1 ksize=3 stride=2 k_ic=4 k_oc=8"
#  conv7:   HALIDE_GEN_ARGS="in_img=14  pad=1 ksize=3 stride=1 k_ic=4 k_oc=8"
#  conv8:   HALIDE_GEN_ARGS="in_img=14  pad=1 ksize=3 stride=2 k_ic=4 k_oc=8"
#  conv9:   HALIDE_GEN_ARGS="in_img=7   pad=4 ksize=3 stride=2 k_ic=4 k_oc=8"


###############################################################################
# MODIFICATION : NONE
#------------------------------------------------------------------------------
# Include hardwrae build targets
#
###############################################################################

include ../../hw_support/hardware_targets.mk

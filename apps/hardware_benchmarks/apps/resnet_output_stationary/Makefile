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
#HL_TARGET                       = host-x86-64
HL_TARGET                       = host-x86-64-enable_ponds
RDAI_PLATFORM_RUNTIME           = clockwork_sim
EXT                             = mat

# Set enviroment variable to set these:
# HALIDE_GEN_ARGS ?= ksize=3 stride=2 k_ic=10 k_oc=3 in_img=64

# resnet layers include:
#  conv1:   HALIDE_GEN_ARGS="in_img=224 pad=3 ksize=7 stride=2 n_ic=3   n_oc=64  k_ic=3 k_oc=16 glb_o=4"
#  conv2_x: HALIDE_GEN_ARGS="in_img=56  pad=1 ksize=3 stride=1 n_ic=64  n_oc=64"
#  conv3_1: HALIDE_GEN_ARGS="in_img=56  pad=1 ksize=3 stride=2 n_ic=64  n_oc=128 glb_i=4"
#  conv3_x: HALIDE_GEN_ARGS="in_img=28  pad=1 ksize=3 stride=1 n_ic=128 n_oc=128"
#  conv4_1: HALIDE_GEN_ARGS="in_img=28  pad=1 ksize=3 stride=2 n_ic=128 n_oc=256 glb_i=4"
#  conv4_x: HALIDE_GEN_ARGS="in_img=14  pad=1 ksize=3 stride=1 n_ic=256 n_oc=256"
#  conv5_1: HALIDE_GEN_ARGS="in_img=14  pad=1 ksize=3 stride=2 n_ic=256 n_oc=512 glb_i=4"
#  conv5_x: HALIDE_GEN_ARGS="in_img=7   pad=1 ksize=3 stride=1 n_ic=512 n_oc=512 k_ic=8 k_oc=8 m_oc=4"


#  conv1:   HALIDE_GEN_ARGS="in_img=224 pad=3 ksize=7 stride=2 n_ic=3 n_oc=8"
#  conv2_x: HALIDE_GEN_ARGS="in_img=56  pad=1 ksize=3 stride=1 n_ic=8 n_oc=8"
#  conv3_1: HALIDE_GEN_ARGS="in_img=56  pad=1 ksize=3 stride=2 n_ic=8 n_oc=8"
#  conv3_x: HALIDE_GEN_ARGS="in_img=28  pad=1 ksize=3 stride=1 n_ic=8 n_oc=8"
#  conv4_1: HALIDE_GEN_ARGS="in_img=28  pad=1 ksize=3 stride=2 n_ic=8 n_oc=8"
#  conv4_x: HALIDE_GEN_ARGS="in_img=14  pad=1 ksize=3 stride=1 n_ic=8 n_oc=8"
#  conv5_1: HALIDE_GEN_ARGS="in_img=14  pad=1 ksize=3 stride=2 n_ic=8 n_oc=8"
#  conv5_x: HALIDE_GEN_ARGS="in_img=7   pad=1 ksize=3 stride=1 n_ic=8 n_oc=8"

# 1x1 convolution:
# HALIDE_GEN_ARGS="in_img=14 pad=0 ksize=1 stride=1 n_ic=16 n_oc=128 k_ic=8 k_oc=8 schedule=1"
# HALIDE_GEN_ARGS="in_img=14 pad=0 ksize=1 stride=1 n_ic=16 n_oc=128 k_ic=8 k_oc=8 m_ic=2 m_oc=8"

# unet layers include:
# conv1_i0: HALIDE_GEN_ARGS="in_img=572 pad=0 ksize=3 stride=1 n_ic=1   n_oc=64  k_ic=1 k_oc=64"
# conv2_i0: HALIDE_GEN_ARGS="in_img=284 pad=0 ksize=3 stride=1 n_ic=64  n_oc=128 k_ic=8 k_oc=8 tw=32 th=32"
# conv3_i0: HALIDE_GEN_ARGS="in_img=140 pad=0 ksize=3 stride=1 n_ic=128 n_oc=256 k_ic=8 k_oc=8 tw=28 th=28"
# conv4_i0: HALIDE_GEN_ARGS="in_img=68  pad=0 ksize=3 stride=1 n_ic=256 n_oc=512 k_ic=8 k_oc=8 tw=24 th=24"
# conv5_i0: HALIDE_GEN_ARGS="in_img=32  pad=0 ksize=3 stride=1 n_ic=512 n_oc=1024 k_ic=8 k_oc=8 tw=32 th=32"
# conv6_i0: HALIDE_GEN_ARGS="in_img=56  pad=0 ksize=3 stride=1 n_ic=1024 n_oc=512 k_ic=8 k_oc=8 tw=28 th=28"
# conv6_i1: HALIDE_GEN_ARGS="in_img=54  pad=0 ksize=3 stride=1 n_ic=512 n_oc=512 k_ic=8 k_oc=8 tw=28 th=28"
# conv7_i0: HALIDE_GEN_ARGS="in_img=104 pad=0 ksize=3 stride=1 n_ic=512 n_oc=256 k_ic=8 k_oc=8 tw=28 th=28"
# conv7_i1: HALIDE_GEN_ARGS="in_img=102 pad=0 ksize=3 stride=1 n_ic=256 n_oc=256 k_ic=8 k_oc=8 tw=28 th=28"
# conv8_i0: HALIDE_GEN_ARGS="in_img=200 pad=0 ksize=3 stride=1 n_ic=256 n_oc=128 k_ic=8 k_oc=8 tw=20 th=20"
# conv8_i1: HALIDE_GEN_ARGS="in_img=198 pad=0 ksize=3 stride=1 n_ic=128 n_oc=128 k_ic=8 k_oc=8 tw=20 th=20"
# conv9_i0: HALIDE_GEN_ARGS="in_img=392 pad=0 ksize=3 stride=1 n_ic=128 n_oc=64  k_ic=8 k_oc=8 tw=28 th=28"
# conv9_i1: HALIDE_GEN_ARGS="in_img=390 pad=0 ksize=3 stride=1 n_ic=64  n_oc=64  k_ic=8 k_oc=8 tw=28 th=28"
# conv9_i2: HALIDE_GEN_ARGS="in_img=388 pad=0 ksize=3 stride=1 n_ic=64  n_oc=2  k_ic=32 k_oc=2 tw=28 th=28 glb_i=4"


# jitnet layers include:
# conv1:  HALIDE_GEN_ARGS="in_w=1280 in_h=720 tw=24 th=24 pad=1 ksize=3  stride=2 n_ic=3   n_oc=8   k_ic=3 k_oc=4  glb_i=3"
# conv2:  HALIDE_GEN_ARGS="in_w=720 in_h=360 tw=24 th=24 pad=1 ksize=3   stride=2 n_ic=8   n_oc=8   k_ic=8 k_oc=4  glb_i=4 glb_k=2 glb_o=2"

# enc1_c: HALIDE_GEN_ARGS="in_w=320 in_h=180 tw=32 th=20 pad=1 ksize=3   stride=2 n_ic=8   n_oc=64  k_ic=8 k_oc=16 glb_i=8 glb_k=4 glb_o=8"
# enc1_h: HALIDE_GEN_ARGS="in_w=160 in_h=90  tw=32 th=24 pad=1 kw=1 kh=3 stride=1 n_ic=64  n_oc=64  k_ic=8 k_oc=16 glb_i=4 glb_k=4 glb_o=8"
# enc1_w: HALIDE_GEN_ARGS="in_w=160 in_h=90  tw=32 th=24 pad=1 kw=3 kh=1 stride=1 n_ic=64  n_oc=64  k_ic=8 k_oc=16 glb_i=4 glb_k=4 glb_o=8"
# enc1_i: HALIDE_GEN_ARGS="in_w=320 in_h=180 tw=32 th=20 pad=0 ksize=1   stride=2 n_ic=8   n_oc=64  k_ic=8 k_oc=16 glb_i=4 glb_k=4 glb_o=8 schedule=1"

# enc2_c: HALIDE_GEN_ARGS="in_w=160 in_h=90  tw=20 th=32 pad=1 ksize=3   stride=2 n_ic=64  n_oc=64  k_ic=8 k_oc=16 glb_i=8 glb_k=4 glb_o=8"
# enc2_h: HALIDE_GEN_ARGS="in_w=80  in_h=45  tw=20 th=24 pad=1 kw=1 kh=3 stride=1 n_ic=64  n_oc=64  k_ic=8 k_oc=16 glb_i=4 glb_k=4 glb_o=8"
# enc2_w: HALIDE_GEN_ARGS="in_w=80  in_h=45  tw=20 th=24 pad=1 kw=3 kh=1 stride=1 n_ic=64  n_oc=64  k_ic=8 k_oc=16 glb_i=4 glb_k=4 glb_o=8"
# enc2_i: HALIDE_GEN_ARGS="in_w=160 in_h=90  tw=20 th=32 pad=0 ksize=1   stride=2 n_ic=64  n_oc=64  k_ic=8 k_oc=16 glb_i=4 glb_k=4 glb_o=8 schedule=1"

# enc3_c: HALIDE_GEN_ARGS="in_w=80  in_h=45  tw=20 th=24 pad=1 ksize=3   stride=2 n_ic=64  n_oc=128 k_ic=8 k_oc=16 glb_i=8 glb_k=4 glb_o=8"
# enc3_h: HALIDE_GEN_ARGS="in_w=40  in_h=23  tw=20 th=24 pad=1 kw=1 kh=3 stride=1 n_ic=128 n_oc=128 k_ic=8 k_oc=16 glb_i=4 glb_k=4 glb_o=8"
# enc3_w: HALIDE_GEN_ARGS="in_w=40  in_h=23  tw=20 th=24 pad=1 kw=3 kh=1 stride=1 n_ic=128 n_oc=128 k_ic=8 k_oc=16 glb_i=4 glb_k=4 glb_o=8"
# enc3_i: HALIDE_GEN_ARGS="in_w=80  in_h=45  tw=20 th=24 pad=0 ksize=1   stride=2 n_ic=64  n_oc=128 k_ic=8 k_oc=16 glb_i=4 glb_k=4 glb_o=8 schedule=1"

# dec1_c: HALIDE_GEN_ARGS="in_w=40  in_h=23  tw=20 th=24 pad=1 ksize=3   stride=1 n_ic=128 n_oc=64  k_ic=8 k_oc=16 glb_o=2"
# dec1_h: HALIDE_GEN_ARGS="in_w=40  in_h=23  tw=20 th=24 pad=1 kw=1 kh=3 stride=1 n_ic=64  n_oc=64  k_ic=8 k_oc=16 glb_i=4 glb_k=4 glb_o=8"
# dec1_w: HALIDE_GEN_ARGS="in_w=40  in_h=23  tw=20 th=24 pad=1 kw=3 kh=1 stride=1 n_ic=64  n_oc=64  k_ic=8 k_oc=16 glb_i=4 glb_k=4 glb_o=8"
# dec1_i: HALIDE_GEN_ARGS="in_w=40  in_h=23  tw=20 th=24 pad=0 ksize=1   stride=1 n_ic=128 n_oc=64  k_ic=8 k_oc=16 glb_i=8 glb_k=8 glb_o=16"

# dec2_c: HALIDE_GEN_ARGS="in_w=80  in_h=45  tw=20 th=24 pad=1 ksize=3   stride=1 n_ic=64  n_oc=32  k_ic=8 k_oc=16 glb_o=2"
# dec2_h: HALIDE_GEN_ARGS="in_w=80  in_h=45  tw=20 th=24 pad=1 kw=1 kh=3 stride=1 n_ic=32  n_oc=32  k_ic=8 k_oc=16 glb_i=4 glb_k=4 glb_o=8"
# dec2_w: HALIDE_GEN_ARGS="in_w=80  in_h=45  tw=20 th=24 pad=1 kw=3 kh=1 stride=1 n_ic=32  n_oc=32  k_ic=8 k_oc=16 glb_i=4 glb_k=4 glb_o=8"
# dec2_i: HALIDE_GEN_ARGS="in_w=80  in_h=45  tw=20 th=24 pad=0 ksize=1   stride=1 n_ic=64  n_oc=32  k_ic=8 k_oc=16 glb_i=8 glb_k=8 glb_o=16"

# dec3_c: HALIDE_GEN_ARGS="in_w=160 in_h=90  tw=20 th=32 pad=1 ksize=3   stride=1 n_ic=32  n_oc=32  k_ic=8 k_oc=16 glb_o=2"
# dec3_h: HALIDE_GEN_ARGS="in_w=160 in_h=90  tw=20 th=32 pad=1 kw=1 kh=3 stride=1 n_ic=32  n_oc=32  k_ic=8 k_oc=16 glb_i=4 glb_k=4 glb_o=8"
# dec3_w: HALIDE_GEN_ARGS="in_w=160 in_h=90  tw=20 th=32 pad=1 kw=3 kh=1 stride=1 n_ic=32  n_oc=32  k_ic=8 k_oc=16 glb_i=4 glb_k=4 glb_o=8"
# dec3_i: HALIDE_GEN_ARGS="in_w=160 in_h=90  tw=20 th=32 pad=0 ksize=1   stride=1 n_ic=32  n_oc=32  k_ic=8 k_oc=16 glb_i=8 glb_k=8 glb_o=16"

# conv3,4:HALIDE_GEN_ARGS="in_w=640 in_h=360 tw=32 th=24 pad=1 ksize=3   stride=1 n_ic=32  n_oc=32  k_ic=8 k_oc=16 glb_o=2"
# conv5:  HALIDE_GEN_ARGS="in_w=1280 in_h=720 tw=32 th=24 pad=0 ksize=1  stride=1 n_ic=32  n_oc=32  k_ic=8 k_oc=16 glb_i=8 glb_k=8 glb_o=16"


# initial mobilenet layer
# conv: HALIDE_GEN_ARGS="in_img=224 pad=1 ksize=3 stride=2 n_ic=3 n_oc=32 k_ic=3 k_oc=16 glb_i=3 glb_o=8"

###############################################################################
# MODIFICATION : NONE
#------------------------------------------------------------------------------
# Include hardware build targets
#
###############################################################################

include ../../hw_support/hardware_targets.mk

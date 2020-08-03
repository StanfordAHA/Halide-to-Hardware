SHELL = /bin/bash
HALIDE_DEBUG_REDIRECT ?= &>/dev/null

default: all
travis testtravis: pointwise harris

# all directories (just text) excluding a particular list
ALL_APPS := $(subst /,, $(dir $(wildcard */.)))
ALL_APPS := $(filter-out hls_support tools unet, $(ALL_APPS))

### app suites include: ###
APP_SUITES := simple img dnn other

### simple applications ###
SIMPLE_APPS := pointwise gaussian cascade harris
simple := $(SIMPLE_APPS)

### imaging applications ###
# IMG_APPS := pointwise gaussian cascade harris \
#             example fast_corner unsharp \
#             demosaic demosaic_complex camera_pipeline \
#             exposure_fusion local_laplacian stereo hdr_plus
IMG_APPS := pointwise gaussian cascade harris \
						unsharp \
						camera_pipeline
img := $(IMG_APPS)

### DNN applications ###
# DNN_APPS := gemm unet \
#             conv_layer_2D conv_layer_3D conv_layer_mobile
DNN_APPS := unet/conv_3_3 unet/down_sample unet/up_sample \
						resnet resnet_stride resnet_block \
            conv_layer_mobile
dnn := $(DNN_APPS)

### other applications ###
# OTHER_APPS := smith_waterman
# other := $(OTHER_APPS)

ALL_APPS_SORTED := $(IMG_APPS) $(DNN_APPS) $(OTHER_APPS)

# Define targets for running each app and suite including:
#  $(APP)-cpu:               create cpu design files without running
#  $(APP)-clockwork:         create clockwork files without running
#  $(APP)-compare-clockwork: run cpu and clockwork and compare output images
#  $(APP)-check:             see what files were successfully generated
#  $(APP)-golden:            copy generated files to golden directory
#  $(APP)-clean:             remove all generated files from the app
include ../include.mk

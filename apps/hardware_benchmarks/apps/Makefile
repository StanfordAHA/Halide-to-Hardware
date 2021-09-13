SHELL = /bin/bash
HALIDE_DEBUG_REDIRECT ?= &>/dev/null

default: all

TRAVIS_APPS := pointwise gaussian cascade harris \
								unsharp camera_pipeline resnet_layer_gen
travis := $(TRAVIS_APPS)
travis: $(TRAVIS_APPS)

# all directories (just text) excluding a particular list
ALL_APPS := $(subst /,, $(dir $(wildcard */.)))
ALL_APPS += $(subst / , , $(dir $(wildcard unet/*/.)))
IGNORED_DIRS := hls_support tools unet \
								unet/conv_3_3/ example gemm conv_layer_2D
ALL_APPS := $(filter-out $(IGNORED_DIRS), $(ALL_APPS))
#ALL_APPS += $(dir $(wildcard unet/*/.))

### app suites include: ###
APP_SUITES := simple img video dnn soc other travis paper

### simple applications ###
SIMPLE_APPS := pointwise brighten_and_blur cascade \
							 demosaic demosaic_complex demosaic_2x2
simple := $(SIMPLE_APPS)

### imaging applications ###
# IMG_APPS := pointwise gaussian cascade harris \
#             example fast_corner unsharp \
#             demosaic demosaic_complex camera_pipeline \
#             exposure_fusion local_laplacian stereo hdr_plus
IMG_APPS := blur gaussian harris harris_color \
						unsharp camera_pipeline fast_corner lens_distortion \
						pyramid_blur local_laplacian laplacian_pyramid bilateral_grid \
						max_filter nlmeans \
            lucas_kanade stereo 
img := $(IMG_APPS)

### VIDEO APPS ###
VIDEO_APPS := exposure_fusion \
							hdr_plus \
							hologram_wgs

### DNN applications ###
# DNN_APPS := gemm \
#             conv_layer_2D conv_layer_3D conv_layer_mobile
DNN_APPS := unet/conv_1_1 unet/conv_2_2 unet/conv_3_3 unet/down_sample unet/up_sample \
            conv_layer_3D \
						resnet resnet_stride resnet_layer_gen resnet_block  resnet_output_stationary \
						resnet_pond_layer resnet_pond_stationary \
            conv_layer_mobile mobilenet_layer_gen \
						jitnet resnet50 mobilenetv2
dnn := $(DNN_APPS)

PAPER_APPS := gaussian harris \
							unet/up_sample \
							unsharp camera_pipeline \
							resnet_layer_gen conv_layer_mobile
paper := $(PAPER_APPS)

### SOC applications ###
SOC_APPS := resnet_full_layer resnet_full_pond_layer resnet_multi_layer
soc := $(SOC_APPS)

### other applications ###
OTHER_APPS := audio_pipeline fft smith_waterman
other := $(OTHER_APPS)

ALL_APPS_SORTED := $(SIMPLE_APPS) $(IMG_APPS) $(VIDEO_APPS) $(DNN_APPS) $(SOC_APPS) $(OTHER_APPS)

# Define targets for running each app and suite including:
#  $(APP)-cpu:               create cpu design files without running
#  $(APP)-clockwork:         create clockwork files without running
#  $(APP)-compare-clockwork: run cpu and clockwork and compare output images
#  $(APP)-check:             see what files were successfully generated
#  $(APP)-golden:            copy generated files to golden directory
#  $(APP)-clean:             remove all generated files from the app
include ../include.mk

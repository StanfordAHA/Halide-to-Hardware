SHELL = /bin/bash
HALIDE_DEBUG_REDIRECT ?= &>/dev/null

default: all

TRAVIS_APPS := pointwise gaussian cascade harris \
								unsharp camera_pipeline resnet_layer_gen
travis := $(TRAVIS_APPS)
travis: $(TRAVIS_APPS)


# application suite:
# - stencils: gaussian, cascade, harris, unsharp, camera, nlmeans
# - data-dependent: histogram, bilateral
# - hierarchical: gaussian, laplacaian, exposure_fusion, optical_flow, image_blend
# - DNNs: resnet, mobilenet, jitnet, unet, gemm
THESIS_STENCIL_APPS := gaussian cascade harris unsharp camera_pipeline_2x2 nlmeans \
												lucas_kanade histogram bilateral_grid
THESIS_HIERARCHICAL_APPS := pyramid_blur laplacian_pyramid exposure_fusion image_blend lucas_kanade_hierarchical
THESIS_DNN_APPS := resnet_output_stationary mobilenet_layer_gen gemm jitnet unet

THESIS_APPS := $(THESIS_STENCIL_APPS) \
								$(THESIS_HIERARCHICAL_APPS) \
								$(THESIS_DNN_APPS)

thesis_stencil thesis-stencil: $(THESIS_STENCIL_APPS)
thesis_pyramid thesis-pyramid thesis_hier thesis-hier: $(THESIS_HIERARCHICAL_APPS)
thesis_dnn thesis-dnn: $(THESIS_DNN_APPS)
thesis := $(THESIS_APPS)
thesis: $(THESIS_APPS)

# all directories (just text) excluding a particular list
ALL_APPS := $(subst /,, $(dir $(wildcard */.)))
ALL_APPS += $(subst / , , $(dir $(wildcard unet/*/.)))
IGNORED_DIRS := hls_support tools unet \
								unet/conv_3_3/ example gemm conv_layer_2D
ALL_APPS := $(filter-out $(IGNORED_DIRS), $(ALL_APPS))
#ALL_APPS += $(dir $(wildcard unet/*/.))

### app suites include: ###
APP_SUITES := simple img video dnn soc other travis paper thesis

### simple applications ###
SIMPLE_APPS := pointwise brighten_and_blur cascade \
							 demosaic demosaic_complex demosaic_2x2
simple := $(SIMPLE_APPS)

### imaging applications ###
IMG_APPS := blur gaussian harris harris_color \
						unsharp camera_pipeline camera_pipeline_2x2 \
						fast_corner lens_distortion \
						pyramid_blur local_laplacian laplacian_pyramid interpolate \
						bilateral_grid \
						max_filter nlmeans \
            lucas_kanade stereo lens_blur
img := $(IMG_APPS)

### VIDEO applications ###
VIDEO_APPS := exposure_fusion \
							hdr_plus \
							hologram_wgs

### DNN applications ###
DNN_APPS := unet/conv_1_1 unet/conv_2_2 unet/conv_3_3 unet/down_sample unet/up_sample \
            conv_layer_3D \
						resnet resnet_stride resnet_layer_gen resnet_block  resnet_output_stationary \
						resnet_pond_layer resnet_pond_stationary \
            conv_layer_mobile mobilenet_layer_gen
dnn := $(DNN_APPS)

### FULL DNN applications ###
FULL_DNN_APPS := jitnet resnet50 mobilenetv2 \
                 bert
full-dnn := $(DNN_APPS) $(FULL_DNN_APPS)

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

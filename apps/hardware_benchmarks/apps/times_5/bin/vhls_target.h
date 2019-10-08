#ifndef HALIDE_CODEGEN_VHLS_TARGET_VHLS_TARGET_H
#define HALIDE_CODEGEN_VHLS_TARGET_VHLS_TARGET_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <hls_stream.h>
#include "Stencil.h"

void vhls_target(
hls::stream<AxiPackedStencil<uint16_t, 1, 1> > &arg_0,
hls::stream<AxiPackedStencil<uint16_t, 1, 1> > &arg_1);

#endif


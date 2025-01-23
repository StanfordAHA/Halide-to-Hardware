hcompute_hw_input_b_global_wrapper_stencil() {
// _hw_input_b_stencil_1 added as input _hw_input_b_stencil_1
// connected _hw_input_b_stencil_1 to the output port
}

hcompute_hw_input_global_wrapper_stencil() {
// _hw_input_stencil_1 added as input _hw_input_stencil_1
// connected _hw_input_stencil_1 to the output port
}

hcompute_add_stencil() {
uint16_t _393 = _hw_input_global_wrapper_stencil_1 + _hw_input_b_global_wrapper_stencil_1;
// _hw_input_global_wrapper_stencil_1 added as input _hw_input_global_wrapper_stencil_1
// _hw_input_b_global_wrapper_stencil_1 added as input _hw_input_b_global_wrapper_stencil_1
// adda: self.in // addb: self.in o: _393 with obitwidth:16
// connected _393 to the output port
}

hcompute_hw_output_stencil() {
// _add_stencil_1 added as input _add_stencil_1
// connected _add_stencil_1 to the output port
}


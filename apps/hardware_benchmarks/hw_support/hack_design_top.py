#!/usr/bin/env python3

import os, re, argparse, json, copy
from collections import OrderedDict, defaultdict, Counter
from pretty_format_json import pretty_format_json

APPS_NEEDING_HACKS = [
    "gelu_pass1_mu_input_fp",
    "gelu_pass2_fp",
    "add_gelu_pass1_mu_input_mem_buf_fp",
    "add_gelu_pass2_fp",
    "maxpooling_dense_rv_fp",
    "maxpooling_dense_rv_mem_buf_fp",
    "tanh_fp",
]


class SelectedDesignHacker:
    """
    A class to handle design JSON modifications (aka 'hacks') for specific apps.
    """

    def __init__(self, hack_apps):
        """
        :param hack_apps: A list of test/app names that require hacking
        """
        self.hack_apps = hack_apps

        # Extract halide_gen_args dict for config
        HALIDE_GEN_ARGS = os.environ.get("HALIDE_GEN_ARGS", None)
        self.halide_gen_args_dict = dict(
            item.split("=") for item in (HALIDE_GEN_ARGS or "").strip().split()
        )

        # Instance templates
        self.APPLY_SCALE_INSTR = "84'h0220001000550015300a9"
        self.DATA_PACKING_INSTR = "84'h0200201104128c0d3001d"
        self.FP_MUL_INSTR = "84'h00000420009004040000e"
        self.FP_ADD_INSTR = "84'h000008000410002480082"
        self.ABS_MAX_INSTR = "84'h0008003fff94400440016"
        self.GET_SHARED_EXP_INSTR = "84'h0200040dc420041530025"
        self.DUMMY_MAX_NOP_INSTR = "84'h0010005fefe0800400092"

        self.pond_tpl = {
            "genref": "cgralib.Pond",
            "genargs": {
                "ID": ["String", ""],
                "has_stencil_valid": ["Bool", True],
                "num_inputs": ["Int", 2],
                "num_outputs": ["Int", 2],
                "width": ["Int", 16],
            },
            "modargs": {"config": ["Json", {}], "mode": ["String", "pond"]},
            "metadata": {"config": {}, "mode": "pond"},
        }
        self.mem_tpl = {
            "genref": "cgralib.Mem",
            "genargs": {
                "ID": ["String", ""],
                "ctrl_width": ["Int", 16],
                "has_chain_en": ["Bool", False],
                "has_external_addrgen": ["Bool", False],
                "has_flush": ["Bool", True],
                "has_read_valid": ["Bool", False],
                "has_reset": ["Bool", False],
                "has_stencil_valid": ["Bool", True],
                "has_valid": ["Bool", False],
                "is_rom": ["Bool", True],
                "num_inputs": ["Int", 2],
                "num_outputs": ["Int", 2],
                "use_prebuilt_mem": ["Bool", True],
                "width": ["Int", 16]
            },
            "modargs": {"config": ["Json", {}], "init": ["Json", None], "mode": ["String", "lake"]},
            "metadata": {"config": {}, "init": None, "mode": "lake"},
        }
        self.shift_fifo_tpl = {
            "genref": "coreir.reg",
            "genargs": {"width": ["Int", 16]},
            "modargs": {"clk_posedge": ["Bool", True], "init": [["BitVector", 16], "16'h0000"]},
            "metadata": {"extra_data": 1},
        }
        self.pipeline_fifo_tpl = {
            "genref": "coreir.reg",
            "genargs": {"width": ["Int", 16]},
            "modargs": {"clk_posedge": ["Bool", True], "init": [["BitVector", 16], "16'h0000"]},
            "metadata": {"extra_data": 0},
        }
        self.pe_tpl = {
            "modref": "global.PE"
        }
        self.apply_scale_const_tpl = {
            "genref": "coreir.const",
            "genargs": {"width": ["Int", 84]},
            "modargs": {"value": [["BitVector", 84], self.APPLY_SCALE_INSTR]},
        }
        self.data_packing_const_tpl = {
            "genref": "coreir.const",
            "genargs": {"width": ["Int", 84]},
            "modargs": {"value": [["BitVector", 84], self.DATA_PACKING_INSTR]},
        }
        self.fp_add_const_tpl = {
            "genref": "coreir.const",
            "genargs": {"width": ["Int", 84]},
            "modargs": {"value": [["BitVector", 84], self.FP_ADD_INSTR]},
        }
        self.fp_mul_const_tpl = {
            "genref": "coreir.const",
            "genargs": {"width": ["Int", 84]},
            "modargs": {"value": [["BitVector", 84], self.FP_MUL_INSTR]},
        }
        self.abs_max_const_tpl = {
            "genref": "coreir.const",
            "genargs": {"width": ["Int", 84]},
            "modargs": {"value": [["BitVector", 84], self.ABS_MAX_INSTR]},
        }
        self.get_shared_exp_const_tpl = {
            "genref": "coreir.const",
            "genargs": {"width": ["Int", 84]},
            "modargs": {"value": [["BitVector", 84], self.GET_SHARED_EXP_INSTR]},
        }
        self.const_clk_tpl = {
            "modref": "corebit.const",
            "modargs": {"value": ["Bool", True]},
        }
        self.input_io_tpl = {
            "modref": "global.IO",
            "modargs": {"mode": ["String", "in"]},
            "metadata": {
                "in2glb_0": {
                    "cycle_starting_addr": [0],
                    "cycle_stride": [1],
                    "dimensionality": 1,
                    "extent": [128],
                    "read_data_starting_addr": [0],
                    "read_data_stride": [1],
                }
            }
        }
        self.output_io_tpl = {
            "modref": "global.IO",
            "modargs": {"mode": ["String", "out"]},
            "metadata": {
                "in2glb_0": {
                    "cycle_starting_addr": [0],
                    "cycle_stride": [1],
                    "dimensionality": 1,
                    "extent": [128],
                    "write_data_starting_addr": [0],
                    "write_data_stride": [1],
                }
            }
        }

    def hack_design_if_needed(self, testname, json_path, bin_path):
        """
        Only apply hacks if `testname` is in self.hack_apps.
        Otherwise, do nothing.

        :param testname: Name of the test
        :param json_path: Path to the JSON file (input & output in-place)
        """
        use_strait_coreir = os.getenv("USE_STRAIT_COREIR", "0") == "1"
        if testname not in self.hack_apps and not use_strait_coreir:
            print(
                f"\033[92m[INFO] Skipping selected hack for '{testname}', not in hack list: {self.hack_apps}\033[0m"
            )
            return

        print(f"\033[94m[INFO] Applying hack for '{testname}'...\033[0m")

        # Apply different hacks based on whether using rv or static
        use_rv = os.getenv("DENSE_READY_VALID", "0") != "0"
        if use_rv:
            hack_method_name = f"hack_for_{testname}_rv"
        else:
            hack_method_name = f"hack_for_{testname}_static"

        hack_method = getattr(self, hack_method_name, None)
        if hack_method is None:
            raise AttributeError(
                f"\033[91mError: Method '{hack_method_name}' does not exist for test '{testname}'.\033[0m"
            )
        hack_method(json_path, bin_path)

    def hack_for_vector_reduction_fp_rv(self, json_path, bin_path, top_module="vector_reduction_fp"):
        """
        Strait-generated tree sum-reduction graph (one scalar per row).
        """
        from strait.coreir_backend.templates.reduction_sum_bf16 import (
            emit_reduction_sum_bf16_design,
        )
        unroll = int(self.halide_gen_args_dict["glb_i"])
        vec_length = int(self.halide_gen_args_dict["vec_width"])
        num_vecs = int(self.halide_gen_args_dict["vec_height"])
        emit_reduction_sum_bf16_design(
            unroll, vec_length, num_vecs, bin_path,
            top_module=top_module,
        )
        design_meta_path = os.path.join(bin_path, "design_meta_halide.json")
        with open(design_meta_path, "r") as f:
            design_meta = json.load(f)
        design_meta["IOs"]["inputs"][0]["shape"] = [vec_length, num_vecs]
        design_meta["IOs"]["outputs"][0]["shape"] = [num_vecs]
        with open(design_meta_path, "w") as f:
            json.dump(design_meta, f, indent=2)
        self._assert_strait_names_match_halide_meta(bin_path)

    def hack_for_mat_vec_mul_fp_rv(self, json_path, bin_path, top_module="mat_vec_mul_fp"):
        """
        Replace the Halide-generated design_top.json with a strait-generated
        bf16 GEMV graph (no bias).

        unroll = glb_i (tree width / parallel matrix+vector lanes).
        vec_length = matrix_width (reduction dim per row).
        num_vecs = matrix_height (number of output rows).
        """
        from strait.coreir_backend.templates.reduction_gemv_bf16 import (
            emit_reduction_gemv_bf16_design,
        )

        unroll = int(self.halide_gen_args_dict["glb_i"])
        vec_length = int(self.halide_gen_args_dict["matrix_width"])
        num_vecs = int(self.halide_gen_args_dict["matrix_height"])

        print(f"\033[94m[INFO] Generating strait {top_module} (GEMV) design: "
              f"unroll={unroll}, vec_length={vec_length}, num_vecs={num_vecs}\033[0m")
        emit_reduction_gemv_bf16_design(
            unroll, vec_length, num_vecs, bin_path,
            has_bias=False, top_module=top_module,
        )
        print(f"\033[92m[INFO] Replaced design_top.json at {json_path}\033[0m")

        # Update Halide meta: add vector input, set matrix/output shapes.
        design_meta_path = os.path.join(bin_path, "design_meta_halide.json")
        with open(design_meta_path, "r") as f:
            design_meta = json.load(f)
        inputs = design_meta["IOs"]["inputs"]
        for inp in inputs:
            if inp.get("name") == "matrix_host_stencil":
                inp["shape"] = [vec_length, num_vecs]
        if not any(x.get("name") == "vector_host_stencil" for x in inputs):
            inputs.append({
                "bitwidth": 16,
                "datafile": "vector_host_stencil.raw",
                "name": "vector_host_stencil",
                "shape": [vec_length],
            })
        assert len(design_meta["IOs"]["outputs"]) == 1, "Expected only one output"
        design_meta["IOs"]["outputs"][0]["shape"] = [num_vecs]
        with open(design_meta_path, "w") as f:
            json.dump(design_meta, f, indent=2)

        self._assert_strait_names_match_halide_meta(bin_path)

    def hack_for_fully_connected_layer_fp_rv(self, json_path, bin_path):
        """
        Replace the Halide-generated design_top.json with a strait-generated
        bf16 GEMV + bias-add graph.

        unroll = glb_i (tree width / parallel matrix+vector lanes).
        vec_length = matrix_width (reduction dim per row).
        num_vecs = matrix_height (number of output rows).
        """
        from strait.coreir_backend.templates.reduction_gemv_bf16 import (
            emit_reduction_gemv_bf16_design,
        )

        top_module = "fully_connected_layer_fp"
        unroll = int(self.halide_gen_args_dict["glb_i"])
        vec_length = int(self.halide_gen_args_dict["matrix_width"])
        num_vecs = int(self.halide_gen_args_dict["matrix_height"])

        print(f"\033[94m[INFO] Generating strait {top_module} (GEMV + bias) design: "
              f"unroll={unroll}, vec_length={vec_length}, num_vecs={num_vecs}\033[0m")
        emit_reduction_gemv_bf16_design(
            unroll, vec_length, num_vecs, bin_path,
            has_bias=True, top_module=top_module,
        )
        print(f"\033[92m[INFO] Replaced design_top.json at {json_path}\033[0m")

        # Update Halide meta to match the matrix+vector+bias in / 1D out shapes.
        design_meta_path = os.path.join(bin_path, "design_meta_halide.json")
        with open(design_meta_path, "r") as f:
            design_meta = json.load(f)
        inputs = design_meta["IOs"]["inputs"]
        for inp in inputs:
            if inp.get("name") == "matrix_host_stencil":
                inp["shape"] = [vec_length, num_vecs]
        if not any(x.get("name") == "vector_host_stencil" for x in inputs):
            inputs.append({
                "bitwidth": 16,
                "datafile": "vector_host_stencil.raw",
                "name": "vector_host_stencil",
                "shape": [vec_length],
            })
        if not any(x.get("name") == "bias_host_stencil" for x in inputs):
            inputs.append({
                "bitwidth": 16,
                "datafile": "bias_host_stencil.raw",
                "name": "bias_host_stencil",
                "shape": [num_vecs],
            })
        assert len(design_meta["IOs"]["outputs"]) == 1, "Expected only one output"
        design_meta["IOs"]["outputs"][0]["shape"] = [num_vecs]
        with open(design_meta_path, "w") as f:
            json.dump(design_meta, f, indent=2)

        self._assert_strait_names_match_halide_meta(bin_path)

    def hack_for_stable_softmax_pass1_fp_rv(self, json_path, bin_path):
        """
        Replace the Halide-generated design_top.json with a strait-generated
        bf16 max-reduction tree + dual-accumulator graph.

        unroll = glb_i (tree width / parallel input lanes).
        vec_length = vec_width, num_vecs = vec_height (from halide_gen_args).
        """
        from strait.coreir_backend.templates.reduction_max_bf16 import (
            emit_reduction_max_bf16_design,
        )

        unroll = int(self.halide_gen_args_dict["glb_i"])
        vec_length = int(self.halide_gen_args_dict["vec_width"])
        num_vecs = int(self.halide_gen_args_dict["vec_height"])

        print(f"\033[94m[INFO] Generating strait stable_softmax_pass1 (max-reduction) design: "
              f"unroll={unroll}, vec_length={vec_length}, num_vecs={num_vecs}\033[0m")
        emit_reduction_max_bf16_design(
            unroll, vec_length, num_vecs, bin_path,
        )
        print(f"\033[92m[INFO] Replaced design_top.json at {json_path}\033[0m")

        # Update Halide meta to match the 1D-in, 1D-out shape the strait design produces.
        design_meta_path = os.path.join(bin_path, "design_meta_halide.json")
        with open(design_meta_path, "r") as f:
            design_meta = json.load(f)
        assert len(design_meta["IOs"]["inputs"]) == 1, "Expected only one input"
        assert len(design_meta["IOs"]["outputs"]) == 1, "Expected only one output"
        design_meta["IOs"]["inputs"][0]["shape"] = [vec_length, num_vecs]
        design_meta["IOs"]["outputs"][0]["shape"] = [num_vecs]
        with open(design_meta_path, "w") as f:
            json.dump(design_meta, f, indent=2)

        self._assert_strait_names_match_halide_meta(bin_path)

    def hack_for_stable_softmax_pass2_fp_rv(self, json_path, bin_path):
        """
        Replace the Halide-generated design_top.json with a strait-generated
        elementwise exp(input - vec_max) graph. One shared vec_max IO replaces
        the glb_i-way unrolled copies Halide emits; shrink vec_max_host_stencil
        shape in design_meta_halide.json to match the broadcast.
        """
        from strait.coreir_backend.templates.elementwise_sub_exp_bf16 import (
            emit_elementwise_sub_exp_bf16_design,
        )

        unroll = int(self.halide_gen_args_dict["glb_i"])
        vec_length = int(self.halide_gen_args_dict["vec_width"])
        num_vecs = int(self.halide_gen_args_dict["vec_height"])

        print(f"\033[94m[INFO] Generating strait stable_softmax_pass2 (exp(input-vec_max)) design: "
              f"unroll={unroll}, vec_length={vec_length}, num_vecs={num_vecs}\033[0m")
        emit_elementwise_sub_exp_bf16_design(unroll, vec_length, num_vecs, bin_path)
        print(f"\033[92m[INFO] Replaced design_top.json at {json_path}\033[0m")

        design_meta_path = os.path.join(bin_path, "design_meta_halide.json")
        with open(design_meta_path, "r") as f:
            design_meta = json.load(f)
        for inp in design_meta.get("IOs", {}).get("inputs", []):
            if inp.get("name") == "vec_max_host_stencil":
                inp["shape"] = [num_vecs]
                break
        with open(design_meta_path, "w") as f:
            json.dump(design_meta, f, indent=2)

        self._assert_strait_names_match_halide_meta(bin_path)

    def hack_for_stable_softmax_pass3_fp_rv(self, json_path, bin_path):
        """
        Replace the Halide-generated design_top.json with a strait-generated
        sum-reduction + reciprocal + broadcast-multiply graph.

        unroll = glb_i (tree width / parallel lanes).
        vec_length = vec_width, num_vecs = vec_height (from halide_gen_args).
        """
        from strait.coreir_backend.templates.reduction_sum_recip_elementwise_mul_bf16 import (
            emit_reduction_sum_recip_elementwise_mul_design,
        )

        unroll = int(self.halide_gen_args_dict["glb_i"])
        vec_length = int(self.halide_gen_args_dict["vec_width"])
        num_vecs = int(self.halide_gen_args_dict["vec_height"])

        print(f"\033[94m[INFO] Generating strait stable_softmax_pass3 (sum + 1/sum + broadcast-mul) design: "
              f"unroll={unroll}, vec_length={vec_length}, num_vecs={num_vecs}\033[0m")
        emit_reduction_sum_recip_elementwise_mul_design(unroll, vec_length, num_vecs, bin_path)
        print(f"\033[92m[INFO] Replaced design_top.json at {json_path}\033[0m")

        design_meta_path = os.path.join(bin_path, "design_meta_halide.json")
        with open(design_meta_path, "r") as f:
            design_meta = json.load(f)
        assert len(design_meta["IOs"]["inputs"]) == 1, "Expected only one input"
        assert len(design_meta["IOs"]["outputs"]) == 1, "Expected only one output"
        design_meta["IOs"]["inputs"][0]["shape"] = [vec_length, num_vecs]
        design_meta["IOs"]["outputs"][0]["shape"] = [vec_length, num_vecs]
        with open(design_meta_path, "w") as f:
            json.dump(design_meta, f, indent=2)

        self._assert_strait_names_match_halide_meta(bin_path)


    def hack_for_layer_norm_pass1_fp_rv(self, json_path, bin_path):
        """
        Replace the Halide-generated design_top.json with a strait-generated
        sum-reduction + scalar-multiply-by-(-1/vec_width) + broadcast-add graph.

        unroll = glb_i (tree width / parallel lanes).
        vec_length = vec_width, num_vecs = vec_height (from halide_gen_args).
        """
        from strait.coreir_backend.templates.reduction_sum_mul_elementwise_sub_bf16 import (
            emit_reduction_sum_mul_elementwise_sub_bf16_design,
        )

        unroll = int(self.halide_gen_args_dict["glb_i"])
        vec_length = int(self.halide_gen_args_dict["vec_width"])
        num_vecs = int(self.halide_gen_args_dict["vec_height"])

        print(f"\033[94m[INFO] Generating strait layer_norm_pass1 (sum + scalar_mul + broadcast-add) design: "
              f"unroll={unroll}, vec_length={vec_length}, num_vecs={num_vecs}\033[0m")
        emit_reduction_sum_mul_elementwise_sub_bf16_design(unroll, vec_length, num_vecs, bin_path)
        print(f"\033[92m[INFO] Replaced design_top.json at {json_path}\033[0m")

        design_meta_path = os.path.join(bin_path, "design_meta_halide.json")
        with open(design_meta_path, "r") as f:
            design_meta = json.load(f)
        assert len(design_meta["IOs"]["inputs"]) == 1, "Expected only one input"
        assert len(design_meta["IOs"]["outputs"]) == 1, "Expected only one output"
        design_meta["IOs"]["inputs"][0]["shape"] = [vec_length, num_vecs]
        design_meta["IOs"]["outputs"][0]["shape"] = [vec_length, num_vecs]
        with open(design_meta_path, "w") as f:
            json.dump(design_meta, f, indent=2)

        self._assert_strait_names_match_halide_meta(bin_path)

    def hack_for_layer_norm_pass2_fp_rv(self, json_path, bin_path):
        """
        Replace the Halide-generated design_top.json with a strait-generated
        sum-of-squares + rsqrt-and-scale + broadcast mul-add graph for
        layer_norm_pass2_fp.

        unroll = glb_i (tree width / parallel lanes).
        vec_length = vec_width, num_vecs = vec_height (from halide_gen_args).
        gamma/beta default to the Halide-hardcoded 1.2 / -0.35; override via env
        if future app variants need different affine params.
        """
        from strait.coreir_backend.templates.reduction_sum_of_sqr_sqrt_recip_mul_elementwise_mul_add_bf16 import (
            emit_reduction_sum_of_sqr_sqrt_recip_mul_elementwise_mul_add_bf16_design,
        )

        unroll = int(self.halide_gen_args_dict["glb_i"])
        vec_length = int(self.halide_gen_args_dict["vec_width"])
        num_vecs = int(self.halide_gen_args_dict["vec_height"])
        gamma = float(os.environ.get("LAYER_NORM_GAMMA", 1.2))
        beta = float(os.environ.get("LAYER_NORM_BETA", -0.35))

        print(f"\033[94m[INFO] Generating strait layer_norm_pass2 (sum-of-squares + rsqrt + broadcast mul-add) design: "
              f"unroll={unroll}, vec_length={vec_length}, num_vecs={num_vecs}, gamma={gamma}, beta={beta}\033[0m")
        emit_reduction_sum_of_sqr_sqrt_recip_mul_elementwise_mul_add_bf16_design(
            unroll, vec_length, num_vecs, bin_path, gamma=gamma, beta=beta
        )
        print(f"\033[92m[INFO] Replaced design_top.json at {json_path}\033[0m")

        design_meta_path = os.path.join(bin_path, "design_meta_halide.json")
        with open(design_meta_path, "r") as f:
            design_meta = json.load(f)
        assert len(design_meta["IOs"]["inputs"]) == 1, "Expected only one input"
        assert len(design_meta["IOs"]["outputs"]) == 1, "Expected only one output"
        design_meta["IOs"]["inputs"][0]["shape"] = [vec_length, num_vecs]
        design_meta["IOs"]["outputs"][0]["shape"] = [vec_length, num_vecs]
        with open(design_meta_path, "w") as f:
            json.dump(design_meta, f, indent=2)

        self._assert_strait_names_match_halide_meta(bin_path)

    def hack_for_rms_norm_pass1_fp_rv(self, json_path, bin_path):
        """
        Replace the Halide-generated design_top.json with a strait-generated
        sum-of-squares + rsqrt + broadcast elementwise-mul graph (RMS norm).

        Reuses the layer_norm_pass2 template with:
          - top_module="rms_norm_pass1_fp"
          - gamma=1.0 (numerator = sqrt(N) instead of sqrt(N)*gamma)
          - has_beta=False (no trailing beta add after elementwise fp_mul)
          - lane_to_stencil: mirror gold's Halide-scheduler stencil permutation
            on BOTH input and output sides, so the new design's wiring matches
            gold's exactly and gold's path_balancing values apply to the same
            physical wires they balanced in gold.
        """
        from strait.coreir_backend.templates.reduction_sum_of_sqr_sqrt_recip_mul_elementwise_mul_add_bf16 import (
            emit_reduction_sum_of_sqr_sqrt_recip_mul_elementwise_mul_add_bf16_design,
        )

        unroll = int(self.halide_gen_args_dict["glb_i"])
        vec_length = int(self.halide_gen_args_dict["vec_width"])
        num_vecs = int(self.halide_gen_args_dict["vec_height"])

        print(f"\033[94m[INFO] Generating strait rms_norm_pass1 (sum-of-squares + rsqrt + broadcast mul) design: "
              f"unroll={unroll}, vec_length={vec_length}, num_vecs={num_vecs}\033[0m")
        emit_reduction_sum_of_sqr_sqrt_recip_mul_elementwise_mul_add_bf16_design(
            unroll, vec_length, num_vecs, bin_path,
            gamma=1.0, beta=0.0,
            top_module="rms_norm_pass1_fp", has_beta=False,
        )
        print(f"\033[92m[INFO] Replaced design_top.json at {json_path}\033[0m")

        design_meta_path = os.path.join(bin_path, "design_meta_halide.json")
        with open(design_meta_path, "r") as f:
            design_meta = json.load(f)
        assert len(design_meta["IOs"]["inputs"]) == 1, "Expected only one input"
        assert len(design_meta["IOs"]["outputs"]) == 1, "Expected only one output"
        design_meta["IOs"]["inputs"][0]["shape"] = [vec_length, num_vecs]
        design_meta["IOs"]["outputs"][0]["shape"] = [vec_length, num_vecs]
        with open(design_meta_path, "w") as f:
            json.dump(design_meta, f, indent=2)

        self._assert_strait_names_match_halide_meta(bin_path)

    def hack_for_layer_norm_pass3_fp_rv(self, json_path, bin_path):
        """
        Replace the Halide-generated design_top.json with a strait-generated
        affine rescale graph: ((input * (1/gamma)) + (-beta/gamma)) * weight + bias.

        unroll = glb_i (parallel lanes).
        vec_length = vec_width, num_vecs = vec_height (from halide_gen_args).
        gamma/beta default to the Halide-hardcoded 1.2 / -0.35; override via env
        if future app variants need different affine params.
        """
        from strait.coreir_backend.templates.elementwise_mul_add_mul_add_bf16 import (
            emit_elementwise_mul_add_mul_add_bf16_design,
        )

        unroll = int(self.halide_gen_args_dict["glb_i"])
        vec_length = int(self.halide_gen_args_dict["vec_width"])
        num_vecs = int(self.halide_gen_args_dict["vec_height"])
        gamma = float(os.environ.get("LAYER_NORM_PASS3_GAMMA", 1.2))
        beta = float(os.environ.get("LAYER_NORM_PASS3_BETA", -0.35))

        print(f"\033[94m[INFO] Generating strait layer_norm_pass3 (affine rescale) design: "
              f"unroll={unroll}, vec_length={vec_length}, num_vecs={num_vecs}, gamma={gamma}, beta={beta}\033[0m")
        emit_elementwise_mul_add_mul_add_bf16_design(
            unroll, vec_length, num_vecs, bin_path, gamma=gamma, beta=beta
        )
        print(f"\033[92m[INFO] Replaced design_top.json at {json_path}\033[0m")

        design_meta_path = os.path.join(bin_path, "design_meta_halide.json")
        with open(design_meta_path, "r") as f:
            design_meta = json.load(f)
        for inp in design_meta.get("IOs", {}).get("inputs", []):
            inp["shape"] = [vec_length, num_vecs]
        for out in design_meta.get("IOs", {}).get("outputs", []):
            out["shape"] = [vec_length, num_vecs]
        with open(design_meta_path, "w") as f:
            json.dump(design_meta, f, indent=2)

        self._assert_strait_names_match_halide_meta(bin_path)

    def hack_for_rms_norm_pass2_fp_rv(self, json_path, bin_path):
        """
        Replace the Halide-generated design_top.json with a strait-generated
        elementwise input * weight graph (input_x_weight_broadcast mode of the
        shared elementwise_mul_bf16 template). Weight is broadcast across rows.
        """
        from strait.coreir_backend.templates.elementwise_mul_bf16 import (
            emit_elementwise_mul_bf16_design,
        )

        unroll = int(self.halide_gen_args_dict["glb_i"])
        vec_width = int(self.halide_gen_args_dict["vec_width"])
        vec_height = int(self.halide_gen_args_dict["vec_height"])

        print(f"\033[94m[INFO] Generating strait rms_norm_pass2 (input * broadcast_weight) design: "
              f"unroll={unroll}, vec_width={vec_width}, vec_height={vec_height}\033[0m")
        emit_elementwise_mul_bf16_design(
            unroll, vec_width * vec_height, bin_path,
            mode="input_x_weight_broadcast", num_vecs=vec_height,
            top_module="rms_norm_pass2_fp",
        )
        print(f"\033[92m[INFO] Replaced design_top.json at {json_path}\033[0m")

        design_meta_path = os.path.join(bin_path, "design_meta_halide.json")
        with open(design_meta_path, "r") as f:
            design_meta = json.load(f)
        for inp in design_meta.get("IOs", {}).get("inputs", []):
            inp["shape"] = [vec_width, vec_height]
        for out in design_meta.get("IOs", {}).get("outputs", []):
            out["shape"] = [vec_width, vec_height]
        with open(design_meta_path, "w") as f:
            json.dump(design_meta, f, indent=2)

        self._assert_strait_names_match_halide_meta(bin_path)

    def hack_for_gelu_pass1_mu_input_fp_rv(self, json_path, bin_path):

        with open(json_path, "r") as f:
            design = json.load(f)

        top_module = "gelu_pass1_mu_input_fp"
        global_modules = design["namespaces"]["global"]["modules"]
        if top_module not in global_modules:
            print(f"WARNING: Module '{top_module}' not found in design. No hack applied.")
            return
        gelu_pass1_mu_input_fp = global_modules[top_module]

        instances = gelu_pass1_mu_input_fp["instances"]
        connections = gelu_pass1_mu_input_fp["connections"]
        type_fields = gelu_pass1_mu_input_fp["type"][1]

        # Get halide gen args
        vec_len = int(self.halide_gen_args_dict["vec_width"])
        num_vecs = int(self.halide_gen_args_dict["vec_height"])
        mu_i = int(self.halide_gen_args_dict["mu_i"])
        dummy_max_nop = int(self.halide_gen_args_dict.get("dummy_max_nop", "0"))
        extent = vec_len * num_vecs // mu_i

        # Find all MU input IO instances
        input_io_instances = {}
        for inst_name, inst_config in instances.items():
            if (inst_name.startswith("io16in_mu_input_host_stencil") and
                inst_config.get("modref") == "global.IO" and
                "_clkwrk_" in inst_name and
                "_op_hcompute_mu_input_glb_stencil" in inst_name):
                # Extract clkwrk index
                match = re.search(r"_clkwrk_(\d+)_op_hcompute_mu_input_glb_stencil", inst_name)
                if match:
                    clkwrk_idx = int(match.group(1))
                    # Extract stencil index from the read operation
                    # Pattern can be either "stencil_read" (index 0) or "stencil_<idx>_read"
                    # Look for pattern like "glb_stencil_10_read" or "glb_stencil_read"
                    stencil_match = re.search(r"glb_stencil_(\d+)_read", inst_name)
                    if stencil_match:
                        stencil_idx = int(stencil_match.group(1))
                    else:
                        # If no number after "glb_stencil", it's stencil_0 (implicit)
                        stencil_idx = 0
                    input_io_instances[clkwrk_idx] = {
                        "name": inst_name,
                        "stencil_idx": stencil_idx
                    }

        # Sort by clkwrk index
        sorted_indices = sorted(input_io_instances.keys())

        # Divide lanes into two halves
        first_half_indices = [idx for idx in sorted_indices if idx < mu_i // 2]
        second_half_indices = [idx for idx in sorted_indices if idx >= mu_i // 2]

        # Create output IO template
        output_io_tpl = {
            "modref": "global.IO",
            "modargs": {"mode": ["String", "out"]},
            "metadata": {
                "in2glb_0": {
                    "cycle_starting_addr": [0],
                    "cycle_stride": [1],
                    "dimensionality": 1,
                    "extent": [extent],
                    "write_data_starting_addr": [0],
                    "write_data_stride": [1]
                }
            }
        }

        # Helper function to add connection only if it doesn't exist
        def add_conn_once(src, dst):
            pair = [src, dst]
            if pair not in connections:
                connections.append(pair)

        # Helper function to remove connection
        # Connections can be in [src, dst] or [dst, src] format
        def remove_conn(src, dst):
            pair1 = [src, dst]
            pair2 = [dst, src]
            if pair1 in connections:
                connections.remove(pair1)
            elif pair2 in connections:
                connections.remove(pair2)

        # For second half: remove compute pipelines and create pass-through paths
        for clkwrk_idx in second_half_indices:
            input_io_info = input_io_instances[clkwrk_idx]
            stencil_idx = input_io_info["stencil_idx"]
            input_io_name = input_io_info["name"]

            # Find output IO instance for hw_output_stencil (offset by mu_i)
            output_clkwrk_idx = clkwrk_idx + mu_i
            output_io_name = f"io16_hw_output_stencil_clkwrk_{output_clkwrk_idx}_op_hcompute_hw_output_stencil"
            if stencil_idx > 0:
                output_io_name += f"_{stencil_idx}"
            output_io_name += "_write_0"

            # Pattern to match compute pipeline instances for this clkwrk_idx
            compute_pattern = f"op_hcompute_output_cgra_stencil_{clkwrk_idx}$"

            # Remove ALL connections involving the compute pipeline
            connections_to_remove = []
            for conn in connections:
                src, dst = conn[0], conn[1]
                # Check if either source or destination is part of the compute pipeline
                src_is_compute = src.startswith(compute_pattern)
                dst_is_compute = dst.startswith(compute_pattern)

                if src_is_compute or dst_is_compute:
                    connections_to_remove.append(conn)

            for conn in connections_to_remove:
                remove_conn(conn[0], conn[1])

            # Collect all compute pipeline instances to remove (MEMs, PEs, constants, etc.)
            compute_instances_to_remove = []
            for inst_name in instances.keys():
                if inst_name.startswith(compute_pattern):
                    compute_instances_to_remove.append(inst_name)

            # Remove compute pipeline instances that are no longer connected
            for inst_name in compute_instances_to_remove:
                # Check if this instance is still referenced in any connection
                still_referenced = False
                for conn in connections:
                    src, dst = conn[0], conn[1]
                    # Extract instance name from connection endpoints
                    src_inst = src.split(".")[0] if "." in src else src
                    dst_inst = dst.split(".")[0] if "." in dst else dst
                    if inst_name == src_inst or inst_name == dst_inst:
                        still_referenced = True
                        break

                # Only remove if not referenced in any connection
                if not still_referenced:
                    del instances[inst_name]

            # Create dummy PEs chain
            dummy_pe_names = []
            dummy_const_names = []
            for i in range(dummy_max_nop):
                dummy_pe_name = f"dummy_max_nop_gelu_pass1_mu_input_fp_clkwrk_{clkwrk_idx}_pe{i}"
                dummy_const_name = f"dummy_max_nop_gelu_pass1_mu_input_fp_clkwrk_{clkwrk_idx}_const{i}"

                # Create const instruction instance
                if dummy_const_name not in instances:
                    instances[dummy_const_name] = {
                        "genref": "coreir.const",
                        "genargs": {"width": ["Int", 84]},
                        "modargs": {"value": [["BitVector", 84], self.DUMMY_MAX_NOP_INSTR]},
                    }

                # Create PE instance
                if dummy_pe_name not in instances:
                    instances[dummy_pe_name] = {"modref": "global.PE"}

                dummy_pe_names.append(dummy_pe_name)
                dummy_const_names.append(dummy_const_name)

            # Verify input IO instance exists
            if input_io_name not in instances:
                raise ValueError(f"[ERROR]: {input_io_name} not found in instances.")

            # Remove any existing connections from input IO to io16_hw_output_stencil (we don't want this for second half)
            connections_to_remove = []
            for conn in connections:
                src, dst = conn[0], conn[1]
                if (src == f"{input_io_name}.out" and dst == f"{output_io_name}.in"):
                    connections_to_remove.append(conn)
            for conn in connections_to_remove:
                remove_conn(conn[0], conn[1])

            # Remove self connections for unused io16_hw_output_stencil output IOs (second half)
            port_name = output_io_name.replace("io16_", "")
            connections_to_remove = []
            for conn in connections:
                src, dst = conn[0], conn[1]
                # Remove self connections: self.port_name -> output_io.out
                if (src == f"self.{port_name}" and dst == f"{output_io_name}.out"):
                    connections_to_remove.append(conn)
            for conn in connections_to_remove:
                remove_conn(conn[0], conn[1])

            # Remove the unused io16_hw_output_stencil output IO instance (second half)
            if output_io_name in instances:
                del instances[output_io_name]

            # Wire up the chain: input_io.out -> first_dummy.data0 (only for hw_activation_output_stencil)
            if dummy_pe_names:
                add_conn_once(f"{dummy_pe_names[0]}.data0", f"{input_io_name}.out")
                add_conn_once(f"{dummy_pe_names[0]}.inst", f"{dummy_const_names[0]}.out")

                # Wire up dummy PEs in chain: dummy[i].O0 -> dummy[i+1].data0
                for i in range(len(dummy_pe_names) - 1):
                    add_conn_once(f"{dummy_pe_names[i+1]}.data0", f"{dummy_pe_names[i]}.O0")
                    add_conn_once(f"{dummy_pe_names[i+1]}.inst", f"{dummy_const_names[i+1]}.out")
            else:
                # If dummy_max_nop is 0, we'll connect directly to activation output IO below
                pass

            # Create output IO instance and connection for hw_activation_output_stencil (second half only)
            activation_output_io_name = f"io16_hw_activation_output_stencil_clkwrk_{clkwrk_idx}_op_hcompute_hw_activation_output_stencil_{stencil_idx}_write_0"

            # Create output IO instance if it doesn't exist
            if activation_output_io_name not in instances:
                instances[activation_output_io_name] = copy.deepcopy(output_io_tpl)
            else:
                # Update extent if it already exists
                instances[activation_output_io_name]["metadata"]["in2glb_0"]["extent"] = [extent]

            # Connect from the output of the pass-through path (either dummy PE chain or direct)
            if dummy_pe_names:
                # Connect from last dummy PE to activation output IO
                add_conn_once(f"{activation_output_io_name}.in", f"{dummy_pe_names[-1]}.O0")
            else:
                # Direct connection from input IO to activation output IO
                add_conn_once(f"{activation_output_io_name}.in", f"{input_io_name}.out")

            # Add self connection and type field
            port_name = activation_output_io_name.replace("io16_", "")
            if all(field[0] != port_name for field in type_fields):
                type_fields.append([port_name, ["Array", 16, "Bit"]])
            add_conn_once(f"self.{port_name}", f"{activation_output_io_name}.out")

        # For first half: insert input buffer MEM between input IO and final fp mul PE
        # First, find or create shared clock constant
        shared_clk_const_name = f"{top_module}_clk_en_const"
        if shared_clk_const_name not in instances:
            instances[shared_clk_const_name] = copy.deepcopy(self.const_clk_tpl)

        # Update extents for first half output IOs (io16_hw_output_stencil) and insert MEM buffers
        for clkwrk_idx in first_half_indices:
            input_io_info = input_io_instances[clkwrk_idx]
            stencil_idx = input_io_info["stencil_idx"]
            input_io_name = input_io_info["name"]
            output_clkwrk_idx = clkwrk_idx + mu_i
            output_io_name = f"io16_hw_output_stencil_clkwrk_{output_clkwrk_idx}_op_hcompute_hw_output_stencil"
            if stencil_idx > 0:
                output_io_name += f"_{stencil_idx}"
            output_io_name += "_write_0"

            if output_io_name in instances:
                instances[output_io_name]["metadata"]["in2glb_0"]["extent"] = [extent]

            # Find both fp mul PEs connected to this input IO
            # Pattern: op_hcompute_output_cgra_stencil $inner_compute$float_DW_fp_mul
            # First fp mul PE: has .data0 connected to input IO
            # Final fp mul PE: has .data1 connected to input IO and .O0 connected to output IO
            first_fp_mul_pe_name = None
            final_fp_mul_pe_name = None

            # Check both with and without "MU_" prefix for input IO
            input_io_sources = [f"{input_io_name}.out", f"MU_{input_io_name}.out"]

            # Find first fp mul PE connected to input IO via .data0
            for conn in connections:
                a, b = conn[0], conn[1]
                if ((a.endswith(".data0") and b in input_io_sources) or
                    (b.endswith(".data0") and a in input_io_sources)):
                    pe_port = a if a.endswith(".data0") else b
                    pe_inst = pe_port.split(".")[0]
                    if ("op_hcompute_output_cgra_stencil" in pe_inst and
                        "float_DW_fp_mul" in pe_inst):
                        first_fp_mul_pe_name = pe_inst
                        break

            # Find final fp mul PE connected to output IO via .O0
            for conn in connections:
                a, b = conn[0], conn[1]
                if ((a.endswith(".O0") and b == f"{output_io_name}.in") or
                    (b.endswith(".O0") and a == f"{output_io_name}.in")):
                    pe_port = a if a.endswith(".O0") else b
                    pe_inst = pe_port.split(".")[0]
                    if ("op_hcompute_output_cgra_stencil" in pe_inst and
                        "float_DW_fp_mul" in pe_inst):
                        final_fp_mul_pe_name = pe_inst
                        break

            if first_fp_mul_pe_name is None:
                raise ValueError(f"[ERROR]: Could not find first fp mul PE (data0) for clkwrk_idx {clkwrk_idx}.")
            if final_fp_mul_pe_name is None:
                raise ValueError(f"[ERROR]: Could not find final fp mul PE (O0->output) for clkwrk_idx {clkwrk_idx}.")

            # Verify final fp mul PE's .data1 is connected to input IO
            final_fp_mul_data1_conn = None
            for conn in connections:
                a, b = conn[0], conn[1]
                if ((a == f"{final_fp_mul_pe_name}.data1" and b in input_io_sources) or
                    (b == f"{final_fp_mul_pe_name}.data1" and a in input_io_sources)):
                    final_fp_mul_data1_conn = conn
                    break

            if final_fp_mul_data1_conn is None:
                raise ValueError(f"[ERROR]: Final fp mul PE {final_fp_mul_pe_name} for clkwrk_idx {clkwrk_idx} does not have .data1 connected to input IO.")

            # Find connection from input IO to first fp mul PE's data0
            first_fp_mul_data0_conn = None
            for conn in connections:
                a, b = conn[0], conn[1]
                if ((a == f"{first_fp_mul_pe_name}.data0" and b in input_io_sources) or
                    (b == f"{first_fp_mul_pe_name}.data0" and a in input_io_sources)):
                    first_fp_mul_data0_conn = conn
                    break

            if first_fp_mul_data0_conn is None:
                raise ValueError(f"[ERROR]: First fp mul PE {first_fp_mul_pe_name} for clkwrk_idx {clkwrk_idx} does not have .data0 connected to input IO.")

            # Create input buffer MEM instance
            input_buffer_mem_name = f"{top_module}_input_buffer_mem_clkwrk_{clkwrk_idx}"
            if input_buffer_mem_name not in instances:
                instances[input_buffer_mem_name] = copy.deepcopy(self.mem_tpl)
                instances[input_buffer_mem_name]["genargs"]["ID"][1] = input_buffer_mem_name

            # Break existing connections from input IO to both fp mul PEs
            remove_conn(first_fp_mul_data0_conn[0], first_fp_mul_data0_conn[1])
            remove_conn(final_fp_mul_data1_conn[0], final_fp_mul_data1_conn[1])

            # Get input IO port from connection
            input_io_port = None
            for port in first_fp_mul_data0_conn:
                if port in input_io_sources:
                    input_io_port = port
                    break

            if input_io_port is None:
                raise ValueError(f"[ERROR]: Could not find input IO port in connection for clkwrk_idx {clkwrk_idx}.")

            # Wire up MEM: input IO -> MEM -> both fp mul PEs
            add_conn_once(input_io_port, f"{input_buffer_mem_name}.data_in_0")
            add_conn_once(f"{input_buffer_mem_name}.data_out_0", f"{first_fp_mul_pe_name}.data0")
            add_conn_once(f"{input_buffer_mem_name}.data_out_1", f"{final_fp_mul_pe_name}.data1")

            # Connect clock enable
            add_conn_once(f"{input_buffer_mem_name}.clk_en", f"{shared_clk_const_name}.out")

        # Overwrite the JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

        # Update design_meta_halide.json
        design_meta_path = os.path.join(bin_path, "design_meta_halide.json")
        with open(design_meta_path, "r") as f:
            design_meta = json.load(f)

        # Add new output to design_meta_halide.json
        design_meta["IOs"]["outputs"].append({
            "bitwidth": 16,
            "datafile": "hw_activation_output.raw",
            "name": "hw_activation_output_stencil",
            "shape": [vec_len // 2, num_vecs]
        })

        # Modify existing output shapes
        for output in design_meta["IOs"]["outputs"]:
            output["shape"] = [vec_len // 2, num_vecs]

        with open(design_meta_path, "w") as f:
            json.dump(design_meta, f, indent=2)
    

    def hack_for_add_gelu_pass1_mu_input_fp_mem_buf_rv(self, json_path, bin_path):
        with open(json_path, "r") as f:
            design = json.load(f)

        top_module = "add_gelu_pass1_mu_input_fp"
        global_modules = design["namespaces"]["global"]["modules"]
        if top_module not in global_modules:
            print(f"WARNING: Module '{top_module}' not found in design. No hack applied.")
            return
        add_gelu_pass1_mu_input_fp = global_modules[top_module]

        instances = add_gelu_pass1_mu_input_fp["instances"]
        connections = add_gelu_pass1_mu_input_fp["connections"]
        type_fields = add_gelu_pass1_mu_input_fp["type"][1]

        # Get halide gen args
        vec_len = int(self.halide_gen_args_dict["vec_width"])
        num_vecs = int(self.halide_gen_args_dict["vec_height"])
        mu_i = int(self.halide_gen_args_dict["mu_i"])
        dummy_max_nop = int(self.halide_gen_args_dict.get("dummy_max_nop", "0"))
        extent = vec_len * num_vecs // mu_i

        # Find all MU input IO instances
        input_io_instances = {}
        for inst_name, inst_config in instances.items():
            if (inst_name.startswith("io16in_mu_input_host_stencil") and
                inst_config.get("modref") == "global.IO" and
                "_clkwrk_" in inst_name and
                "_op_hcompute_mu_input_glb_stencil" in inst_name):
                # Extract clkwrk index
                match = re.search(r"_clkwrk_(\d+)_op_hcompute_mu_input_glb_stencil", inst_name)
                if match:
                    clkwrk_idx = int(match.group(1))
                    # Extract stencil index from the read operation
                    # Pattern can be either "stencil_read" (index 0) or "stencil_<idx>_read"
                    # Look for pattern like "glb_stencil_10_read" or "glb_stencil_read"
                    stencil_match = re.search(r"glb_stencil_(\d+)_read", inst_name)
                    if stencil_match:
                        stencil_idx = int(stencil_match.group(1))
                    else:
                        # If no number after "glb_stencil", it's stencil_0 (implicit)
                        stencil_idx = 0
                    input_io_instances[clkwrk_idx] = {
                        "name": inst_name,
                        "stencil_idx": stencil_idx
                    }

        # Find all output IO instances and extract stencil indices
        output_io_info = {}  # Maps stencil_idx to (clkwrk_idx, output_io_name, input_io_info)

        for inst_name, inst_config in instances.items():
            if (inst_name.startswith("io16_hw_add_gelu_upper_output_stencil_clkwrk_") and
                inst_config.get("modref") == "global.IO" and
                "_op_hcompute_hw_add_gelu_upper_output_stencil" in inst_name and
                "_write_0" in inst_name):
                # Extract clkwrk index from output IO
                clkwrk_match = re.search(r"_clkwrk_(\d+)_op_hcompute", inst_name)
                if clkwrk_match:
                    output_clkwrk_idx = int(clkwrk_match.group(1))
                    # Calculate input clkwrk_idx (output_clkwrk_idx - mu_i)
                    input_clkwrk_idx = output_clkwrk_idx - mu_i

                    # Extract stencil index from between "stencil_" and "_write"
                    # Pattern: hw_add_gelu_upper_output_stencil_<idx>_write or hw_add_gelu_upper_output_stencil_write_0 (idx=0)
                    # Look for pattern like: stencil_1_write or stencil_write_0
                    stencil_match = re.search(r"stencil_(\d+)_write", inst_name)
                    if stencil_match:
                        stencil_idx = int(stencil_match.group(1))
                    else:
                        # Check if it's just "stencil_write_0" (no index between stencil_ and _write, so 0)
                        if re.search(r"stencil_write_0", inst_name):
                            stencil_idx = 0
                        else:
                            continue  # Skip if pattern doesn't match

                    # Find corresponding input IO
                    if input_clkwrk_idx in input_io_instances:
                        input_io_info = input_io_instances[input_clkwrk_idx]
                        output_io_info[stencil_idx] = {
                            "clkwrk_idx": input_clkwrk_idx,
                            "output_io_name": inst_name,
                            "input_io_info": input_io_info
                        }

        # Sort by stencil index
        sorted_stencil_indices = sorted(output_io_info.keys())

        # Divide lanes into two halves based on stencil index
        first_half_stencil_indices = [idx for idx in sorted_stencil_indices if idx < len(sorted_stencil_indices) // 2]
        second_half_stencil_indices = [idx for idx in sorted_stencil_indices if idx >= len(sorted_stencil_indices) // 2]

        # Assert that both halves have the same length
        assert len(first_half_stencil_indices) == len(second_half_stencil_indices), \
            f"First half ({len(first_half_stencil_indices)}) and second half ({len(second_half_stencil_indices)}) must have the same length"

        # Map back to clkwrk indices for easier processing
        first_half_indices = [output_io_info[idx]["clkwrk_idx"] for idx in first_half_stencil_indices]
        second_half_indices = [output_io_info[idx]["clkwrk_idx"] for idx in second_half_stencil_indices]

        # Create output IO template
        output_io_tpl = {
            "modref": "global.IO",
            "modargs": {"mode": ["String", "out"]},
            "metadata": {
                "in2glb_0": {
                    "cycle_starting_addr": [0],
                    "cycle_stride": [1],
                    "dimensionality": 1,
                    "extent": [extent],
                    "write_data_starting_addr": [0],
                    "write_data_stride": [1]
                }
            }
        }

        # Helper function to add connection only if it doesn't exist
        def add_conn_once(src, dst):
            pair = [src, dst]
            if pair not in connections:
                connections.append(pair)

        # Helper function to remove connection
        # Connections can be in [src, dst] or [dst, src] format
        def remove_conn(src, dst):
            pair1 = [src, dst]
            pair2 = [dst, src]
            if pair1 in connections:
                connections.remove(pair1)
            elif pair2 in connections:
                connections.remove(pair2)

        # For second half: remove add PE and gelu compute pipeline, pass through mu_input via dummy nop PE
        for stencil_idx in second_half_stencil_indices:
            io_info = output_io_info[stencil_idx]
            clkwrk_idx = io_info["clkwrk_idx"]
            input_io_info = io_info["input_io_info"]
            input_io_name = input_io_info["name"]
            old_output_io_name = io_info["output_io_name"]

            # Find the add PE that takes mu_input and input_psum0
            # Pattern: op_hcompute_output_add_gelu_upper_cgra_stencil*$inner_compute$float_DW_fp_add_*
            # The add PE has .data0 connected to mu_input and .data1 connected to input_psum0
            add_pe_name = None
            add_pe_connections_to_remove = []

            # Find add PE connected to mu_input via .data0
            input_io_sources = [f"{input_io_name}.out", f"MU_{input_io_name}.out"]
            for conn in connections:
                a, b = conn[0], conn[1]
                if ((a.endswith(".data0") and b in input_io_sources) or
                    (b.endswith(".data0") and a in input_io_sources)):
                    pe_port = a if a.endswith(".data0") else b
                    pe_inst = pe_port.split(".")[0]
                    if ("op_hcompute_output_add_gelu_upper_cgra_stencil" in pe_inst and
                        "$inner_compute$float_DW_fp_add" in pe_inst):
                        add_pe_name = pe_inst
                        break

            if add_pe_name is None:
                raise ValueError(f"[ERROR]: Could not find add PE for clkwrk_idx {clkwrk_idx}.")

            # Find all connections involving the add PE
            # Also find the input_psum0_host_stencil IO connected to add PE's .data1
            input_psum0_io_name = None
            for conn in connections:
                src, dst = conn[0], conn[1]
                src_inst = src.split(".")[0] if "." in src else src
                dst_inst = dst.split(".")[0] if "." in dst else dst
                if src_inst == add_pe_name or dst_inst == add_pe_name:
                    add_pe_connections_to_remove.append(conn)
                    # Check if this connection is to input_psum0_host_stencil IO (.data1)
                    if (src_inst == add_pe_name and src.endswith(".data1")) or (dst_inst == add_pe_name and dst.endswith(".data1")):
                        other_end = dst if src_inst == add_pe_name else src
                        if other_end.startswith("io16in_input_psum0_host_stencil"):
                            input_psum0_io_name = other_end.split(".")[0]

            # Pattern to match gelu compute pipeline instances (everything after the add)
            # We need to find all instances that are part of the gelu compute pipeline
            gelu_compute_instances = set()
            gelu_compute_connections_to_remove = []

            # Start from the add PE output and trace all gelu compute instances
            instances_to_check = [add_pe_name]
            checked_instances = set()

            while instances_to_check:
                current_inst = instances_to_check.pop(0)
                if current_inst in checked_instances:
                    continue
                checked_instances.add(current_inst)

                # Find all connections involving this instance
                for conn in connections:
                    src, dst = conn[0], conn[1]
                    src_inst = src.split(".")[0] if "." in src else src
                    dst_inst = dst.split(".")[0] if "." in dst else dst

                    # Check if this connection involves the current instance
                    if src_inst == current_inst or dst_inst == current_inst:
                        # Check if the other end is part of gelu compute
                        other_inst = dst_inst if src_inst == current_inst else src_inst

                        # Skip if it's the add PE, input IO, output IO, or self
                        if (other_inst == add_pe_name or
                            other_inst.startswith("io16in_") or
                            other_inst.startswith("io16_hw_") or
                            other_inst.startswith("self.") or
                            other_inst.startswith("op_hcompute_hw_add_gelu_upper_output_stencil") or
                            other_inst.startswith("op_hcompute_hw_psum1_lower_output_stencil")):
                            continue

                        # Check if it's part of gelu compute pipeline
                        if ("op_hcompute_output_add_gelu_upper_cgra_stencil" in other_inst and
                            ("$inner_compute$float_DW_fp_mul" in other_inst or
                             "$inner_compute$fp_" in other_inst or
                             "$inner_compute$i" in other_inst or
                             "$inner_compute$c" in other_inst)):
                            gelu_compute_instances.add(other_inst)
                            gelu_compute_connections_to_remove.append(conn)
                            if other_inst not in checked_instances:
                                instances_to_check.append(other_inst)

            # Remove all connections involving gelu compute pipeline
            for conn in gelu_compute_connections_to_remove:
                remove_conn(conn[0], conn[1])

            # Remove all connections involving the add PE
            for conn in add_pe_connections_to_remove:
                remove_conn(conn[0], conn[1])

            # Remove gelu compute instances that are no longer connected
            for inst_name in gelu_compute_instances:
                # Check if this instance is still referenced in any connection
                still_referenced = False
                for conn in connections:
                    src, dst = conn[0], conn[1]
                    src_inst = src.split(".")[0] if "." in src else src
                    dst_inst = dst.split(".")[0] if "." in dst else dst
                    if inst_name == src_inst or inst_name == dst_inst:
                        still_referenced = True
                        break

                # Only remove if not referenced in any connection
                if not still_referenced:
                    if inst_name in instances:
                        del instances[inst_name]

            # Remove add PE if it's no longer connected
            add_pe_still_referenced = False
            for conn in connections:
                src, dst = conn[0], conn[1]
                src_inst = src.split(".")[0] if "." in src else src
                dst_inst = dst.split(".")[0] if "." in dst else dst
                if add_pe_name == src_inst or add_pe_name == dst_inst:
                    add_pe_still_referenced = True
                    break

            if not add_pe_still_referenced:
                if add_pe_name in instances:
                    del instances[add_pe_name]

            # Remove input_psum0_host_stencil IO - it's no longer needed since we removed the add PE
            # We need to remove: 1) add PE connection (already done above), 2) self connection, 3) the IO instance
            if input_psum0_io_name:
                # Find the port name (remove io16in_ prefix)
                port_name = input_psum0_io_name.replace("io16in_", "")
                # Remove self connections for input_psum0 (e.g., self.input_psum0_host_stencil_clkwrk_18_...)
                connections_to_remove = []
                for conn in connections:
                    src, dst = conn[0], conn[1]
                    # Remove connections with self.port_name format
                    if src == f"self.{port_name}" or dst == f"self.{port_name}":
                        connections_to_remove.append(conn)
                    # Also remove any remaining connections that reference the instance name directly
                    elif input_psum0_io_name == src or input_psum0_io_name == dst:
                        connections_to_remove.append(conn)
                    elif src.startswith(f"{input_psum0_io_name}.") or dst.startswith(f"{input_psum0_io_name}."):
                        connections_to_remove.append(conn)
                for conn in connections_to_remove:
                    remove_conn(conn[0], conn[1])
                # Remove the instance
                if input_psum0_io_name in instances:
                    del instances[input_psum0_io_name]
                # Remove from type_fields if it exists
                for i, field in enumerate(type_fields):
                    if field[0] == port_name:
                        del type_fields[i]
                        break

            # Rename output IO from hw_add_gelu_upper_output to hw_psum1_lower_output
            # Extract the stencil suffix from old_output_io_name
            stencil_suffix = "" if stencil_idx == 0 else f"_{stencil_idx}"
            output_clkwrk_idx = clkwrk_idx + mu_i
            output_io_name = f"io16_hw_psum1_lower_output_stencil_clkwrk_{output_clkwrk_idx}_op_hcompute_hw_psum1_lower_output_stencil{stencil_suffix}_write_0"

            # Rename output IO from hw_add_gelu_upper_output to hw_psum1_lower_output
            if old_output_io_name in instances:
                # Create new output IO instance with new name
                instances[output_io_name] = copy.deepcopy(instances[old_output_io_name])
                instances[output_io_name]["metadata"]["in2glb_0"]["extent"] = [extent]

                # Remove old output IO instance
                del instances[old_output_io_name]

                # Update type_fields: replace old port name with new port name
                old_port_name = old_output_io_name.replace("io16_", "")
                new_port_name = output_io_name.replace("io16_", "")
                for field in type_fields:
                    if field[0] == old_port_name:
                        field[0] = new_port_name
                        break

                # Update connections: replace old IO name with new IO name
                # Also update self. connections that reference the port name
                for conn in connections:
                    if old_output_io_name in conn[0]:
                        conn[0] = conn[0].replace(old_output_io_name, output_io_name)
                    if old_output_io_name in conn[1]:
                        conn[1] = conn[1].replace(old_output_io_name, output_io_name)
                    # Update self. connections
                    if conn[0] == f"self.{old_port_name}":
                        conn[0] = f"self.{new_port_name}"
                    if conn[1] == f"self.{old_port_name}":
                        conn[1] = f"self.{new_port_name}"

                # Remove existing connection from gelu compute or add PE to output IO
                connections_to_remove = []
                for conn in connections:
                    src, dst = conn[0], conn[1]
                    if dst == f"{output_io_name}.in" or src == f"{output_io_name}.in":
                        # Check if source is part of gelu compute or add PE
                        other_end = dst if src == f"{output_io_name}.in" else src
                        other_inst = other_end.split(".")[0] if "." in other_end else other_end
                        if other_inst in gelu_compute_instances or other_inst == add_pe_name:
                            connections_to_remove.append(conn)

                for conn in connections_to_remove:
                    remove_conn(conn[0], conn[1])

                # Create dummy nop PE and constant
                dummy_pe_name = f"dummy_nop_psum1_lower_clkwrk_{clkwrk_idx}_pe"
                dummy_const_name = f"dummy_nop_psum1_lower_clkwrk_{clkwrk_idx}_const"

                # Create const instruction instance
                if dummy_const_name not in instances:
                    instances[dummy_const_name] = {
                        "genref": "coreir.const",
                        "genargs": {"width": ["Int", 84]},
                        "modargs": {"value": [["BitVector", 84], self.DUMMY_MAX_NOP_INSTR]},
                    }

                # Create PE instance
                if dummy_pe_name not in instances:
                    instances[dummy_pe_name] = {"modref": "global.PE"}

                # Wire up: mu_input IO -> dummy PE -> output IO
                add_conn_once(f"{dummy_pe_name}.data0", f"{input_io_name}.out")
                add_conn_once(f"{dummy_pe_name}.inst", f"{dummy_const_name}.out")
                add_conn_once(f"{output_io_name}.in", f"{dummy_pe_name}.O0")
            else:
                raise ValueError(f"[ERROR]: Output IO {old_output_io_name} not found in instances.")

        # For first half: insert input buffer MEM between add output and gelu compute
        # First, find or create shared clock constant
        shared_clk_const_name = f"{top_module}_clk_en_const"
        if shared_clk_const_name not in instances:
            instances[shared_clk_const_name] = copy.deepcopy(self.const_clk_tpl)

        # Update extents for first half output IOs (io16_hw_add_gelu_upper_output_stencil) and insert MEM buffers
        for stencil_idx in first_half_stencil_indices:
            io_info = output_io_info[stencil_idx]
            clkwrk_idx = io_info["clkwrk_idx"]
            input_io_info = io_info["input_io_info"]
            input_io_name = input_io_info["name"]
            output_io_name = io_info["output_io_name"]

            if output_io_name in instances:
                instances[output_io_name]["metadata"]["in2glb_0"]["extent"] = [extent]

            # Find the add PE that takes mu_input and input_psum0
            add_pe_name = None
            add_pe_output_conn = None

            # Check both with and without "MU_" prefix for input IO
            input_io_sources = [f"{input_io_name}.out", f"MU_{input_io_name}.out"]

            # Find add PE connected to mu_input via .data0
            for conn in connections:
                a, b = conn[0], conn[1]
                if ((a.endswith(".data0") and b in input_io_sources) or
                    (b.endswith(".data0") and a in input_io_sources)):
                    pe_port = a if a.endswith(".data0") else b
                    pe_inst = pe_port.split(".")[0]
                    if ("op_hcompute_output_add_gelu_upper_cgra_stencil" in pe_inst and
                        "$inner_compute$float_DW_fp_add" in pe_inst):
                        add_pe_name = pe_inst
                        # Find connection from add PE output (.O0) to gelu compute
                        for conn2 in connections:
                            a2, b2 = conn2[0], conn2[1]
                            if ((a2 == f"{add_pe_name}.O0") or (b2 == f"{add_pe_name}.O0")):
                                # Check if this connection goes to gelu compute (not output IO)
                                other_end = a2 if b2 == f"{add_pe_name}.O0" else b2
                                if ("float_DW_fp_mul" in other_end or "fp_getmant" in other_end or
                                    "fp_subexp" in other_end or "fp_addiexp" in other_end or
                                    "fp_getffrac" in other_end or "fp_getfint" in other_end):
                                    add_pe_output_conn = conn2
                                    break
                        break

            if add_pe_name is None:
                raise ValueError(f"[ERROR]: Could not find add PE for clkwrk_idx {clkwrk_idx}.")
            if add_pe_output_conn is None:
                raise ValueError(f"[ERROR]: Could not find add PE output connection to gelu compute for clkwrk_idx {clkwrk_idx}.")

            # Find both fp mul PEs connected to add PE output
            # First fp mul PE: has .data0 connected to add.O0
            # Final fp mul PE: has .data1 connected to add.O0 and .O0 connected to output IO
            first_fp_mul_pe_name = None
            final_fp_mul_pe_name = None
            add_pe_output = f"{add_pe_name}.O0"

            # Find first fp mul PE connected to add.O0 via .data0
            for conn in connections:
                a, b = conn[0], conn[1]
                if ((a.endswith(".data0") and b == add_pe_output) or
                    (b.endswith(".data0") and a == add_pe_output)):
                    pe_port = a if a.endswith(".data0") else b
                    pe_inst = pe_port.split(".")[0]
                    if ("op_hcompute_output_add_gelu_upper_cgra_stencil" in pe_inst and
                        "$inner_compute$float_DW_fp_mul" in pe_inst):
                        first_fp_mul_pe_name = pe_inst
                        break

            # Find final fp mul PE connected to output IO via .O0
            for conn in connections:
                a, b = conn[0], conn[1]
                if ((a.endswith(".O0") and b == f"{output_io_name}.in") or
                    (b.endswith(".O0") and a == f"{output_io_name}.in")):
                    pe_port = a if a.endswith(".O0") else b
                    pe_inst = pe_port.split(".")[0]
                    if ("op_hcompute_output_add_gelu_upper_cgra_stencil" in pe_inst and
                        "$inner_compute$float_DW_fp_mul" in pe_inst):
                        final_fp_mul_pe_name = pe_inst
                        break

            if first_fp_mul_pe_name is None:
                raise ValueError(f"[ERROR]: Could not find first fp mul PE (data0) for clkwrk_idx {clkwrk_idx}.")
            if final_fp_mul_pe_name is None:
                raise ValueError(f"[ERROR]: Could not find final fp mul PE (O0->output) for clkwrk_idx {clkwrk_idx}.")

            # Verify final fp mul PE's .data1 is connected to add.O0
            final_fp_mul_data1_conn = None
            for conn in connections:
                a, b = conn[0], conn[1]
                if ((a == f"{final_fp_mul_pe_name}.data1" and b == add_pe_output) or
                    (b == f"{final_fp_mul_pe_name}.data1" and a == add_pe_output)):
                    final_fp_mul_data1_conn = conn
                    break

            if final_fp_mul_data1_conn is None:
                raise ValueError(f"[ERROR]: Final fp mul PE {final_fp_mul_pe_name} for clkwrk_idx {clkwrk_idx} does not have .data1 connected to add.O0.")

            # Find connection from add.O0 to first fp mul PE's data0
            first_fp_mul_data0_conn = None
            for conn in connections:
                a, b = conn[0], conn[1]
                if ((a == f"{first_fp_mul_pe_name}.data0" and b == add_pe_output) or
                    (b == f"{first_fp_mul_pe_name}.data0" and a == add_pe_output)):
                    first_fp_mul_data0_conn = conn
                    break

            if first_fp_mul_data0_conn is None:
                raise ValueError(f"[ERROR]: First fp mul PE {first_fp_mul_pe_name} for clkwrk_idx {clkwrk_idx} does not have .data0 connected to add.O0.")

            # Create input buffer MEM instance
            input_buffer_mem_name = f"{top_module}_input_buffer_mem_clkwrk_{clkwrk_idx}"
            if input_buffer_mem_name not in instances:
                instances[input_buffer_mem_name] = copy.deepcopy(self.mem_tpl)
                instances[input_buffer_mem_name]["genargs"]["ID"][1] = input_buffer_mem_name

            # Break existing connections from add.O0 to both fp mul PEs
            remove_conn(first_fp_mul_data0_conn[0], first_fp_mul_data0_conn[1])
            remove_conn(final_fp_mul_data1_conn[0], final_fp_mul_data1_conn[1])

            # Wire up MEM: add.O0 -> MEM -> both fp mul PEs
            add_conn_once(add_pe_output, f"{input_buffer_mem_name}.data_in_0")
            add_conn_once(f"{input_buffer_mem_name}.data_out_0", f"{first_fp_mul_pe_name}.data0")
            add_conn_once(f"{input_buffer_mem_name}.data_out_1", f"{final_fp_mul_pe_name}.data1")

            # Connect clock enable
            add_conn_once(f"{input_buffer_mem_name}.clk_en", f"{shared_clk_const_name}.out")

            # Insert input buffer MEM between mu_input IO and add PE's .data0
            # Find the mu_input IO connected to add PE's .data0
            mu_input_io_name_for_buf = None
            mu_input_conn = None
            for conn in connections:
                a, b = conn[0], conn[1]
                if ((a == f"{add_pe_name}.data0" and b.startswith("io16in_mu_input_host_stencil")) or
                    (b == f"{add_pe_name}.data0" and a.startswith("io16in_mu_input_host_stencil"))):
                    mu_input_conn = conn
                    mu_input_io_name_for_buf = (b if a == f"{add_pe_name}.data0" else a).split(".")[0]
                    break

            if mu_input_io_name_for_buf is None:
                raise ValueError(f"[ERROR]: Could not find mu_input IO connected to add PE {add_pe_name}.data0 for clkwrk_idx {clkwrk_idx}.")

            # Create MEM buffer for mu_input
            mu_buf_mem_name = f"{top_module}_mu_buffer_mem_clkwrk_{clkwrk_idx}"
            if mu_buf_mem_name not in instances:
                instances[mu_buf_mem_name] = copy.deepcopy(self.mem_tpl)
                instances[mu_buf_mem_name]["genargs"]["ID"][1] = mu_buf_mem_name

            # Break existing connection: mu_input IO.out -> add_pe.data0
            remove_conn(mu_input_conn[0], mu_input_conn[1])

            # Wire up MEM: mu_input IO.out -> MEM.data_in_0 -> MEM.data_out_0 -> add_pe.data0
            add_conn_once(f"{mu_input_io_name_for_buf}.out", f"{mu_buf_mem_name}.data_in_0")
            add_conn_once(f"{mu_buf_mem_name}.data_out_0", f"{add_pe_name}.data0")

            # Connect clock enable
            add_conn_once(f"{mu_buf_mem_name}.clk_en", f"{shared_clk_const_name}.out")

        # Overwrite the JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

        # Update design_meta_halide.json
        design_meta_path = os.path.join(bin_path, "design_meta_halide.json")
        with open(design_meta_path, "r") as f:
            design_meta = json.load(f)

        # Add new output to design_meta_halide.json
        design_meta["IOs"]["outputs"].append({
            "bitwidth": 16,
            "datafile": "hw_psum1_lower_output.raw",
            "name": "hw_psum1_lower_output_stencil",
            "shape": [vec_len // 2, num_vecs]
        })

        # Modify existing output shapes
        for output in design_meta["IOs"]["outputs"]:
            output["shape"] = [vec_len // 2, num_vecs]

        with open(design_meta_path, "w") as f:
            json.dump(design_meta, f, indent=2)

    def hack_for_gelu_pass2_fp_rv(self, json_path, bin_path):

        with open(json_path, "r") as f:
            design = json.load(f)

        top_module = "gelu_pass2_fp"
        global_modules = design["namespaces"]["global"]["modules"]
        if top_module not in global_modules:
            print(f"WARNING: Module '{top_module}' not found in design. No hack applied.")
            return
        gelu_pass2_fp = global_modules[top_module]

        instances = gelu_pass2_fp["instances"]
        connections = gelu_pass2_fp["connections"]

        # Find all input IO instances matching the pattern
        input_io_instances = []
        for inst_name, inst_config in instances.items():
            if (inst_name.startswith("io16in_input_host_stencil") and
                inst_config.get("modref") == "global.IO" and
                "_clkwrk_" in inst_name and
                "_op_hcompute_input_glb_stencil" in inst_name):
                input_io_instances.append(inst_name)

        # Sort IO instances by clkwrk index for consistent ordering
        def extract_clkwrk_idx(name):
            match = re.search(r"_clkwrk_(\d+)_", name)
            return int(match.group(1)) if match else 0
        input_io_instances.sort(key=extract_clkwrk_idx)

        # Helper function to add connection only if it doesn't exist
        def add_conn_once(src, dst):
            pair = [src, dst]
            if pair not in connections:
                connections.append(pair)

        # Helper function to remove connection
        def remove_conn(src, dst):
            pair1 = [src, dst]
            pair2 = [dst, src]
            if pair1 in connections:
                connections.remove(pair1)
            elif pair2 in connections:
                connections.remove(pair2)

        # Find or create shared clock constant
        shared_clk_const_name = f"{top_module}_clk_en_const"
        if shared_clk_const_name not in instances:
            instances[shared_clk_const_name] = copy.deepcopy(self.const_clk_tpl)

        # For each input IO, insert MEM buffer between IO and broadcasted PEs
        for input_io_name in input_io_instances:
            # Find the two PEs connected to this input IO
            pe_data0_name = None
            pe_data1_name = None
            data0_conn = None
            data1_conn = None

            input_io_out = f"{input_io_name}.out"

            # Find PE with .data0 connected to input IO
            for conn in connections:
                a, b = conn[0], conn[1]
                if ((a.endswith(".data0") and b == input_io_out) or
                    (b.endswith(".data0") and a == input_io_out)):
                    pe_port = a if a.endswith(".data0") else b
                    pe_inst = pe_port.split(".")[0]
                    if ("op_hcompute_output_cgra_stencil" in pe_inst and
                        "float_DW_fp_mul" in pe_inst):
                        pe_data0_name = pe_inst
                        data0_conn = conn
                        break

            # Find PE with .data1 connected to input IO
            for conn in connections:
                a, b = conn[0], conn[1]
                if ((a.endswith(".data1") and b == input_io_out) or
                    (b.endswith(".data1") and a == input_io_out)):
                    pe_port = a if a.endswith(".data1") else b
                    pe_inst = pe_port.split(".")[0]
                    if ("op_hcompute_output_cgra_stencil" in pe_inst and
                        "float_DW_fp_mul" in pe_inst):
                        pe_data1_name = pe_inst
                        data1_conn = conn
                        break

            if pe_data0_name is None:
                raise ValueError(f"[ERROR]: Could not find PE with .data0 connected to {input_io_name}.")
            if pe_data1_name is None:
                raise ValueError(f"[ERROR]: Could not find PE with .data1 connected to {input_io_name}.")

            # Create input buffer MEM instance
            # Extract clkwrk index for naming
            clkwrk_idx = extract_clkwrk_idx(input_io_name)
            input_buffer_mem_name = f"{top_module}_input_buffer_mem_clkwrk_{clkwrk_idx}"
            if input_buffer_mem_name not in instances:
                instances[input_buffer_mem_name] = copy.deepcopy(self.mem_tpl)
                instances[input_buffer_mem_name]["genargs"]["ID"][1] = input_buffer_mem_name

            # Break existing connections from input IO to both PEs
            remove_conn(data0_conn[0], data0_conn[1])
            remove_conn(data1_conn[0], data1_conn[1])

            # Wire up MEM: input IO -> MEM -> both PEs
            add_conn_once(input_io_out, f"{input_buffer_mem_name}.data_in_0")
            add_conn_once(f"{input_buffer_mem_name}.data_out_0", f"{pe_data0_name}.data0")
            add_conn_once(f"{input_buffer_mem_name}.data_out_1", f"{pe_data1_name}.data1")

            # Connect clock enable
            add_conn_once(f"{input_buffer_mem_name}.clk_en", f"{shared_clk_const_name}.out")

        # Overwrite the JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

    def hack_for_add_gelu_pass2_fp_rv(self, json_path, bin_path):

        with open(json_path, "r") as f:
            design = json.load(f)

        top_module = "add_gelu_pass2_fp"
        global_modules = design["namespaces"]["global"]["modules"]
        if top_module not in global_modules:
            print(f"WARNING: Module '{top_module}' not found in design. No hack applied.")
            return
        add_gelu_pass2_fp = global_modules[top_module]

        instances = add_gelu_pass2_fp["instances"]
        connections = add_gelu_pass2_fp["connections"]

        # Find all input IO instances matching the pattern
        input_io_instances = []
        for inst_name, inst_config in instances.items():
            if (inst_name.startswith("io16in_input_host_stencil") and
                inst_config.get("modref") == "global.IO" and
                "_clkwrk_" in inst_name and
                "_op_hcompute_input_glb_stencil" in inst_name):
                input_io_instances.append(inst_name)

        # Sort IO instances by clkwrk index for consistent ordering
        def extract_clkwrk_idx(name):
            match = re.search(r"_clkwrk_(\d+)_", name)
            return int(match.group(1)) if match else 0
        input_io_instances.sort(key=extract_clkwrk_idx)

        # Helper function to add connection only if it doesn't exist
        def add_conn_once(src, dst):
            pair = [src, dst]
            if pair not in connections:
                connections.append(pair)

        # Helper function to remove connection
        def remove_conn(src, dst):
            pair1 = [src, dst]
            pair2 = [dst, src]
            if pair1 in connections:
                connections.remove(pair1)
            elif pair2 in connections:
                connections.remove(pair2)

        # Find or create shared clock constant
        shared_clk_const_name = f"{top_module}_clk_en_const"
        if shared_clk_const_name not in instances:
            instances[shared_clk_const_name] = copy.deepcopy(self.const_clk_tpl)

        # For each input IO, find fp_add PE, then insert MEM buffer between fp_add and broadcasted fp_mul PEs
        for input_io_name in input_io_instances:
            # Find the fp_add PE connected to this input IO
            fp_add_pe_name = None
            fp_add_conn = None
            input_io_out = f"{input_io_name}.out"

            # Find fp_add PE with .data0 connected to input IO
            for conn in connections:
                a, b = conn[0], conn[1]
                if ((a.endswith(".data0") and b == input_io_out) or
                    (b.endswith(".data0") and a == input_io_out)):
                    pe_port = a if a.endswith(".data0") else b
                    pe_inst = pe_port.split(".")[0]
                    if ("op_hcompute_output_cgra_stencil" in pe_inst and
                        "float_DW_fp_add" in pe_inst):
                        fp_add_pe_name = pe_inst
                        fp_add_conn = conn
                        break

            if fp_add_pe_name is None:
                raise ValueError(f"[ERROR]: Could not find fp_add PE with .data0 connected to {input_io_name}.")

            # Find the two fp_mul PEs connected to fp_add PE's output
            fp_mul_pe_data0_name = None
            fp_mul_pe_data1_name = None
            fp_mul_data0_conn = None
            fp_mul_data1_conn = None
            fp_add_out = f"{fp_add_pe_name}.O0"

            # Find fp_mul PE with .data0 connected to fp_add.O0
            for conn in connections:
                a, b = conn[0], conn[1]
                if ((a.endswith(".data0") and b == fp_add_out) or
                    (b.endswith(".data0") and a == fp_add_out)):
                    pe_port = a if a.endswith(".data0") else b
                    pe_inst = pe_port.split(".")[0]
                    if ("op_hcompute_output_cgra_stencil" in pe_inst and
                        "float_DW_fp_mul" in pe_inst):
                        fp_mul_pe_data0_name = pe_inst
                        fp_mul_data0_conn = conn
                        break

            # Find fp_mul PE with .data1 connected to fp_add.O0
            for conn in connections:
                a, b = conn[0], conn[1]
                if ((a.endswith(".data1") and b == fp_add_out) or
                    (b.endswith(".data1") and a == fp_add_out)):
                    pe_port = a if a.endswith(".data1") else b
                    pe_inst = pe_port.split(".")[0]
                    if ("op_hcompute_output_cgra_stencil" in pe_inst and
                        "float_DW_fp_mul" in pe_inst):
                        fp_mul_pe_data1_name = pe_inst
                        fp_mul_data1_conn = conn
                        break

            if fp_mul_pe_data0_name is None:
                raise ValueError(f"[ERROR]: Could not find fp_mul PE with .data0 connected to {fp_add_pe_name}.O0.")
            if fp_mul_pe_data1_name is None:
                raise ValueError(f"[ERROR]: Could not find fp_mul PE with .data1 connected to {fp_add_pe_name}.O0.")

            # Create input buffer MEM instance
            # Extract clkwrk index for naming
            clkwrk_idx = extract_clkwrk_idx(input_io_name)
            input_buffer_mem_name = f"{top_module}_input_buffer_mem_clkwrk_{clkwrk_idx}"
            if input_buffer_mem_name not in instances:
                instances[input_buffer_mem_name] = copy.deepcopy(self.mem_tpl)
                instances[input_buffer_mem_name]["genargs"]["ID"][1] = input_buffer_mem_name

            # Break existing connections from fp_add.O0 to both fp_mul PEs
            remove_conn(fp_mul_data0_conn[0], fp_mul_data0_conn[1])
            remove_conn(fp_mul_data1_conn[0], fp_mul_data1_conn[1])

            # Wire up MEM: fp_add.O0 -> MEM -> both fp_mul PEs
            add_conn_once(fp_add_out, f"{input_buffer_mem_name}.data_in_0")
            add_conn_once(f"{input_buffer_mem_name}.data_out_0", f"{fp_mul_pe_data0_name}.data0")
            add_conn_once(f"{input_buffer_mem_name}.data_out_1", f"{fp_mul_pe_data1_name}.data1")

            # Connect clock enable
            add_conn_once(f"{input_buffer_mem_name}.clk_en", f"{shared_clk_const_name}.out")

        # Overwrite the JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

    def hack_for_avgpool_layer_fp_rv(self, json_path, bin_path):
        """
        Replace the Halide-generated design_top.json with a strait-generated
        per-lane spatial-average accumulator graph.

        unroll = glb_i = glb_o (number of parallel output lanes).
        vec_length = in_img * in_img (HW spatial reduction length per lane
        per channel; matches ZIRCON_INPUT_ACT_PADDING_WORKAROUND that pads
        to in_img^2).
        num_vecs = n_ic / glb_i (channels per lane).
        scale = 1 / (in_img - pad)^2 (bf16 constant premultiply).
        """
        from strait.coreir_backend.templates.accumulator_avg_bf16 import (
            emit_accumulator_avg_bf16_design,
        )

        top_module = "avgpool_layer_fp"
        unroll = int(self.halide_gen_args_dict["glb_i"])
        in_img = int(self.halide_gen_args_dict["in_img"])
        pad = int(self.halide_gen_args_dict["pad"])
        n_ic = int(self.halide_gen_args_dict["n_ic"])
        vec_length = in_img * in_img
        num_vecs = n_ic // unroll
        scale_bf16_val = 1.0 / float((in_img - pad) * (in_img - pad))

        print(f"\033[94m[INFO] Generating strait {top_module} (per-lane avg accumulator) design: "
              f"unroll={unroll}, vec_length={vec_length}, num_vecs={num_vecs}, scale={scale_bf16_val}\033[0m")
        emit_accumulator_avg_bf16_design(
            unroll, vec_length, num_vecs, bin_path,
            scale_bf16_val=scale_bf16_val, top_module=top_module,
        )
        print(f"\033[92m[INFO] Replaced design_top.json at {json_path}\033[0m")

        design_meta_path = os.path.join(bin_path, "design_meta_halide.json")
        with open(design_meta_path, "r") as f:
            design_meta = json.load(f)
        assert len(design_meta["IOs"]["inputs"]) == 1, "Expect only one input in avgpool_layer_fp"
        design_meta["IOs"]["inputs"][0]["shape"] = [n_ic, in_img, in_img]
        assert len(design_meta["IOs"]["outputs"]) == 1, "Expect only one output in avgpool_layer_fp"
        design_meta["IOs"]["outputs"][0]["shape"] = [n_ic, 1, 1]
        with open(design_meta_path, "w") as f:
            json.dump(design_meta, f, indent=2)

        self._assert_strait_names_match_halide_meta(bin_path)

    def hack_for_get_e8m0_scale_tree_mu_input_rv(self, json_path, bin_path):
        """
        Replace the Halide-generated design_top.json with a strait-generated
        bf16 get-shared-exp reduction + unquantized passthrough graph.

        unroll = mu_i (parallel MU lanes, = vec_width_fake, = tree fan-in).
        num_vecs = vec_height (image size per channel).
        total_channels = vec_width (all channels across passes).
        """
        from strait.coreir_backend.templates.reduction_get_scale_bf16 import (
            emit_reduction_get_scale_bf16_design,
        )

        top_module = "get_e8m0_scale_tree_mu_input"
        unroll = int(self.halide_gen_args_dict["mu_i"])
        num_vecs = int(self.halide_gen_args_dict["vec_height"])
        total_channels = int(self.halide_gen_args_dict["vec_width"])
        num_lines = total_channels // unroll
        number_of_blocks = total_channels // (unroll * 2)

        print(f"\033[94m[INFO] Generating strait {top_module} (get-e8m0-scale tree) design: "
              f"unroll={unroll}, num_vecs={num_vecs}, total_channels={total_channels}\033[0m")
        emit_reduction_get_scale_bf16_design(
            unroll, num_vecs, total_channels, bin_path,
            top_module=top_module,
        )
        print(f"\033[92m[INFO] Replaced design_top.json at {json_path}\033[0m")

        design_meta_path = os.path.join(bin_path, "design_meta_halide.json")
        with open(design_meta_path, "r") as f:
            design_meta = json.load(f)
        mu_inputs = design_meta["IOs"].setdefault("mu_inputs", [])
        if not any(x.get("name") == "mu_input_host_stencil" for x in mu_inputs):
            mu_inputs.append({
                "bitwidth": 16,
                "datafile": "mu_input_host_stencil.raw",
                "name": "mu_input_host_stencil",
                "shape": [total_channels, num_vecs],
            })
        else:
            for inp in mu_inputs:
                if inp.get("name") == "mu_input_host_stencil":
                    inp["shape"] = [total_channels, num_vecs]
        design_meta["IOs"]["inputs"] = []
        outputs = [
            {
                "bitwidth": 16,
                "datafile": "hw_scale_output.raw",
                "name": "hw_scale_output_stencil",
                "shape": [num_vecs * number_of_blocks],
            },
            {
                "bitwidth": 16,
                "datafile": "unquantized_output_stencil.raw",
                "name": "unquantized_output_stencil",
                "shape": [unroll, num_vecs * num_lines],
            },
        ]
        design_meta["IOs"]["outputs"] = outputs
        with open(design_meta_path, "w") as f:
            json.dump(design_meta, f, indent=2)

        self._assert_strait_names_match_halide_meta(bin_path)

    def hack_for_get_e8m0_scale_tree_gb_input_rv(self, json_path, bin_path):
        """
        Replace the Halide-generated design_top.json with a strait-generated
        bf16 get-shared-exp reduction graph (GLB input variant).

        unroll = glb_i (parallel GLB lanes = tree fan-in).
        num_vecs = vec_height.
        total_channels = vec_width.
        """
        from strait.coreir_backend.templates.reduction_get_scale_bf16 import (
            emit_reduction_get_scale_bf16_design,
        )

        top_module = "get_e8m0_scale_tree_gb_input"
        unroll = int(self.halide_gen_args_dict["glb_i"])
        num_vecs = int(self.halide_gen_args_dict["vec_height"])
        total_channels = int(self.halide_gen_args_dict["vec_width"])
        num_lines = total_channels // unroll
        number_of_blocks = total_channels // (unroll * 2)

        print(f"\033[94m[INFO] Generating strait {top_module} (get-e8m0-scale tree, GLB input) design: "
              f"unroll={unroll}, num_vecs={num_vecs}, total_channels={total_channels}\033[0m")
        emit_reduction_get_scale_bf16_design(
            unroll, num_vecs, total_channels, bin_path,
            mode="gb", top_module=top_module,
        )
        print(f"\033[92m[INFO] Replaced design_top.json at {json_path}\033[0m")

        design_meta_path = os.path.join(bin_path, "design_meta_halide.json")
        with open(design_meta_path, "r") as f:
            design_meta = json.load(f)
        inputs = design_meta["IOs"]["inputs"]
        assert len(inputs) == 1, "Expected only one input"
        inputs[0]["shape"] = [total_channels, num_vecs]
        outputs = design_meta["IOs"]["outputs"]
        for output in outputs:
            if output["name"] == "hw_scale_output_stencil":
                output["datafile"] = "hw_scale_output.raw"
                output["shape"] = [num_vecs * number_of_blocks]
                break
        with open(design_meta_path, "w") as f:
            json.dump(design_meta, f, indent=2)

        self._assert_strait_names_match_halide_meta(bin_path)

    def hack_for_get_e8m0_scale_accum_gb_input_rv(self, json_path, bin_path):
        """
        Replace the Halide-generated design_top.json with a strait-generated
        per-lane abs_max accumulator + get_shared_exp + byte-pack graph.

        unroll = glb_i (number of parallel input lanes; glb_o = unroll/2 after packing).
        head_dim, seq_heads_prod: from halide_gen_args, used for IO extents.
        block_size = 64 (inner spatial reduction length per output pixel).
        """
        from strait.coreir_backend.templates.accumulator_get_scale_bf16 import (
            emit_accumulator_get_scale_bf16_design,
        )

        top_module = "get_e8m0_scale_accum_gb_input"
        unroll = int(self.halide_gen_args_dict["glb_i"])
        head_dim = int(self.halide_gen_args_dict["head_dim"])
        seq_heads_prod = int(self.halide_gen_args_dict["seq_heads_prod"])
        block_size = 64

        print(f"\033[94m[INFO] Generating strait {top_module} (abs-max accum + pack) design: "
              f"unroll={unroll}, head_dim={head_dim}, seq_heads_prod={seq_heads_prod}, block_size={block_size}\033[0m")
        emit_accumulator_get_scale_bf16_design(
            unroll, head_dim, seq_heads_prod, bin_path,
            block_size=block_size, top_module=top_module,
        )
        print(f"\033[92m[INFO] Replaced design_top.json at {json_path}\033[0m")

        design_meta_path = os.path.join(bin_path, "design_meta_halide.json")
        with open(design_meta_path, "r") as f:
            design_meta = json.load(f)
        inputs = design_meta["IOs"]["inputs"]
        outputs = design_meta["IOs"]["outputs"]
        assert len(inputs) == 1, "Expect exactly one input in design_meta_halide.json"
        assert len(outputs) == 1, "Expect exactly one output in design_meta_halide.json"
        inputs[0]["shape"] = [head_dim, seq_heads_prod]
        outputs[0]["shape"] = [head_dim // 2, seq_heads_prod // block_size]
        with open(design_meta_path, "w") as f:
            json.dump(design_meta, f, indent=2)

        self._assert_strait_names_match_halide_meta(bin_path)

    def hack_for_apply_e8m0_scale_single_IO_rv(self, json_path, bin_path):
        """
        Replace the Halide-generated design_top.json with a strait-generated
        e8m0-quant + byte-pack + scale-packing graph.

        unroll = glb_i (bf_act input lanes; glb_o = unroll/2 packed mxint8 outputs).
        num_pixels = vec_height, num_channels = vec_width.
        """
        from strait.coreir_backend.templates.elementwise_quant_pack_single_IO_bf16 import (
            emit_elementwise_quant_pack_single_IO_bf16_design,
        )

        top_module = "apply_e8m0_scale_single_IO"
        unroll = int(self.halide_gen_args_dict["glb_i"])
        num_pixels = int(self.halide_gen_args_dict["vec_height"])
        num_channels = int(self.halide_gen_args_dict["vec_width"])
        num_blocks = num_channels // (unroll * 2)

        print(f"\033[94m[INFO] Generating strait {top_module} (e8m0_quant + pack) design: "
              f"unroll={unroll}, num_pixels={num_pixels}, num_channels={num_channels}\033[0m")
        emit_elementwise_quant_pack_single_IO_bf16_design(
            unroll, num_pixels, num_channels, bin_path,
            top_module=top_module,
        )
        print(f"\033[92m[INFO] Replaced design_top.json at {json_path}\033[0m")

        design_meta_path = os.path.join(bin_path, "design_meta_halide.json")
        with open(design_meta_path, "r") as f:
            design_meta = json.load(f)
        ios = design_meta.setdefault("IOs", {})
        inputs = ios.setdefault("inputs", [])
        outputs = ios.setdefault("outputs", [])
        for entry in inputs:
            if entry.get("name") == "input_bf_act_host_stencil":
                entry["shape"] = [num_channels, num_pixels]
            elif entry.get("name") == "input_scale_host_stencil":
                entry["shape"] = [num_blocks, num_pixels]
        scale_output_shape = [num_blocks // 2, num_pixels] if num_blocks >= 2 else [num_pixels // 2]
        hw_output_entry = next((e for e in outputs if e.get("name") == "hw_output_mxint8_act_stencil"), None)
        if hw_output_entry is not None:
            hw_output_entry["datafile"] = "hw_output_mxint8_act.raw"
            hw_output_entry["shape"] = [num_channels // 2, num_pixels]
        scale_output_entry = next((e for e in outputs if e.get("name") == "hw_scale_packed_output_stencil"), None)
        if scale_output_entry is None:
            outputs.append({
                "bitwidth": 16,
                "datafile": "hw_scale_packed_output.raw",
                "name": "hw_scale_packed_output_stencil",
                "shape": scale_output_shape,
            })
        else:
            scale_output_entry["datafile"] = "hw_scale_packed_output.raw"
            scale_output_entry["shape"] = scale_output_shape
        with open(design_meta_path, "w") as f:
            json.dump(design_meta, f, indent=2)

        self._assert_strait_names_match_halide_meta(bin_path)

    def hack_for_apply_e8m0_scale_multi_IOs_rv(self, json_path, bin_path):
        """
        Replace the Halide-generated design_top.json with a strait-generated
        unpack-scale + e8m0_quant + byte-pack graph whose PE/IO naming mirrors
        bin_gold exactly so the mapper's alphabetical block-id assignment
        (netlist_util.py CreateIDs) and thunder's igraph clustering produce
        the same placement.
        """
        from strait.coreir_backend.templates.elementwise_unpack_quant_pack_multi_IOs_bf16 import (
            emit_elementwise_unpack_quant_pack_multi_IOs_bf16_design,
        )

        top_module = "apply_e8m0_scale_multi_IOs"
        unroll = int(self.halide_gen_args_dict["glb_i"])
        head_dim = int(self.halide_gen_args_dict["head_dim"])
        seq_heads_prod = int(self.halide_gen_args_dict["seq_heads_prod"])
        block_size = int(self.halide_gen_args_dict.get("block_size", 64))

        print(f"\033[94m[INFO] Generating strait {top_module} (unpack-scale + e8m0_quant + pack) design: "
              f"unroll={unroll}, head_dim={head_dim}, seq_heads_prod={seq_heads_prod}, block_size={block_size}\033[0m")
        emit_elementwise_unpack_quant_pack_multi_IOs_bf16_design(
            unroll, head_dim, seq_heads_prod, bin_path,
            block_size=block_size, top_module=top_module,
        )
        print(f"\033[92m[INFO] Replaced design_top.json at {json_path}\033[0m")

        design_meta_path = os.path.join(bin_path, "design_meta_halide.json")
        with open(design_meta_path, "r") as f:
            design_meta = json.load(f)
        ios = design_meta.setdefault("IOs", {})
        inputs = ios.setdefault("inputs", [])
        outputs = ios.setdefault("outputs", [])
        for entry in inputs:
            if entry.get("name") == "input_bf_act_host_stencil":
                entry["shape"] = [head_dim, seq_heads_prod]
            elif entry.get("name") == "input_scale_host_stencil":
                entry["shape"] = [head_dim // 2, seq_heads_prod // block_size]
        for entry in outputs:
            if entry.get("name") == "hw_output_mxint8_act_stencil":
                entry["shape"] = [head_dim // 2, seq_heads_prod]
                entry["datafile"] = "hw_output_mxint8_act.raw"
        with open(design_meta_path, "w") as f:
            json.dump(design_meta, f, indent=2)

        self._assert_strait_names_match_halide_meta(bin_path)

    def hack_for_maxpooling_dense_rv_fp_rv(self, json_path, bin_path):
        '''
        Unhacked compute graph consists of unroll number of PE chains with IOs and MEMs servring as line buffers.
        Some chain use one MEM and some use two, while one MEM per chain is enough.
        To handle multiple channels per lane, unhacked graph uses n_ic // unroll FIFOs between adjacent PEs to interleave across channels.
        Dense RV maxpooling is not compilable with clockwork, so there are redundant FIFOs for compute delay matching.
        This hack collapses all redundant FIFOs, removes redundant MEMs and constant PEs, hardcodes the first max PE instruction
        with DUMMY_MAX_NOP_INSTR, and configures GLB DMA to handle multiple channels per lane.
        '''
        with open(json_path, "r") as f:
            design = json.load(f)

        top_module_name = "maxpooling_dense_rv_fp"
        module = design["namespaces"]["global"]["modules"][top_module_name]
        instances = module["instances"]
        connections = module["connections"]

        # -----Collapse all shift FIFO $d_reg chains-----
        # Define helpers to identify shift chains
        def is_shift(edge_point: str) -> bool:
            return "$d_reg" in edge_point

        def is_shift_in(edge_point: str) -> bool:
            return is_shift(edge_point) and edge_point.endswith(".in")

        def is_shift_out(edge_point: str) -> bool:
            return is_shift(edge_point) and edge_point.endswith(".out")

        def inst_of(edge_point: str) -> str:
            return edge_point.rsplit(".", 1)[0]

        # Collect directed views of shift chains
        shift_in_driver = {}
        shift_out_fanout = defaultdict(set)
        for a, b in connections:
            if is_shift_out(a): shift_out_fanout[a].add(b)
            if is_shift_out(b): shift_out_fanout[b].add(a)
            if is_shift_in(a): shift_in_driver[a] = b
            if is_shift_in(b): shift_in_driver[b] = a

        head_in_ports = [ip for ip, drv in shift_in_driver.items() if not is_shift_out(drv)]

        bridged = set()
        for head_in in head_in_ports:
            upstream_src = shift_in_driver[head_in]
            head_out = f"{inst_of(head_in)}.out"
            stack = [head_out]
            visited_out = set()
            sinks = set()
            while stack:
                outp = stack.pop()
                if outp in visited_out:
                    continue
                visited_out.add(outp)
                for nxt in shift_out_fanout.get(outp, []):
                    if is_shift_in(nxt):
                        stack.append(f"{inst_of(nxt)}.out")
                    else:
                        sinks.add(nxt)
            for dst in sinks:
                bridged.add((dst, upstream_src))

        kept = []
        for a, b in connections:
            if is_shift(a) or is_shift(b):
                continue
            kept.append([a, b])

        tmp = []
        seen = set()
        for d, s in kept + [[d, s] for (d, s) in sorted(bridged)]:
            key = (d, s)
            if key in seen: continue
            seen.add(key)
            tmp.append([d, s])
        connections = tmp

        for name in list(instances.keys()):
            if "$d_reg" in name:
                del instances[name]

        # -----Collect PE chains-----
        # Define patterns for PEs, MEMs, and IOs. ChatGPT generated regexes.
        floatmax_pat = re.compile(
            r"^(?P<base>op_hcompute_max_pooling_inner_stencil_(?P<chain>\d+)"
            r"\$inner_compute\$float_max_[^\.]+)\.(?P<pin>.+)$"
        )
        const_pat = re.compile(
            r"^(?P<base>op_hcompute_max_pooling_inner_stencil(?:_(?P<chain>\d+))?"
            r"\$inner_compute\$const_i\d+_i\d+)\.(?P<pin>.+)$"
        )
        const_inst_pat = re.compile(
            r"^op_hcompute_max_pooling_inner_stencil(?:_\d+)?\$inner_compute\$c\d+\.out$"
        )
        io_out_pat = re.compile(r"^io16in_input_host_stencil_clkwrk_\d+_.+_read_0\.out$")
        mem_out_pat = re.compile(
            r"^(?P<mem>input_host_global_wrapper_global_wrapper_stencil"
            r"\$ub_input_host_global_wrapper_global_wrapper_stencil_[^\.]+_garnet)\.data_out_(?P<port>[01])$"
        )
        mem_any_pat = re.compile(
            r"^(?P<mem>input_host_global_wrapper_global_wrapper_stencil"
            r"\$ub_input_host_global_wrapper_global_wrapper_stencil_[^\.]+_garnet)\."
        )

        # Collect all max PEs per chain with O0->data1 conns
        chain_pe_set = defaultdict(set)
        pe_next = defaultdict(dict)
        pe_prev = defaultdict(dict)

        for a, b in connections:
            for ep in (a, b):
                m = floatmax_pat.match(ep)
                if m:
                    chain_pe_set[int(m.group("chain"))].add(m.group("base"))
            for src, dst in ((a, b), (b, a)):
                ms = floatmax_pat.match(src)
                md = floatmax_pat.match(dst)
                if not (ms and md):
                    continue
                if ms.group("pin") != "O0" or md.group("pin") != "data1":
                    continue
                c = int(ms.group("chain"))
                if c != int(md.group("chain")):
                    continue
                u = ms.group("base")
                v = md.group("base")
                pe_next[c][u] = v
                pe_prev[c][v] = u

        # Identify head max PE from const.O0 -> max.data0
        chain_head_max = {}
        chain_const_base = {}
        for a, b in connections:
            for src, dst in ((a, b), (b, a)):
                mc = const_pat.match(src)
                md = floatmax_pat.match(dst)
                if not (mc and md):
                    continue
                if mc.group("pin") != "O0" or md.group("pin") != "data0":
                    continue
                chain = int(mdst_chain := md.group("chain"))
                chain_head_max[chain] = md.group("base")
                chain_const_base[chain] = mc.group("base")

        # Order PEs: walk from first max PE via O0->data1
        chain_to_ordered_pes = {}
        for chain, pes in chain_pe_set.items():
            head_max = chain_head_max.get(chain)
            if not head_max:
                head_candidates = [p for p in pes if p not in pe_prev[chain]]
                head_max = sorted(head_candidates)[0] if head_candidates else sorted(pes)[0]
            order = []
            cur = head_max
            visited = set()
            while cur and cur not in visited:
                order.append(cur)
                visited.add(cur)
                cur = pe_next[chain].get(cur)
            chain_to_ordered_pes[chain] = order

        chain_ids = [c for c in sorted(chain_to_ordered_pes) if len(chain_to_ordered_pes[c]) >= 1]

        # Identify first max PE per chain and collect old const instruction instances connected to them
        first_pe_per_chain = {}
        old_const_inst_to_delete = set()
        for c in chain_ids:
            ordered = chain_to_ordered_pes[c]
            if not ordered:
                continue
            first_pe = ordered[0]
            first_pe_per_chain[c] = first_pe
            # Find const instruction instances connected to first PE's .inst port
            for a, b in connections:
                for src, dst in ((a, b), (b, a)):
                    if const_inst_pat.match(src) and dst == first_pe + ".inst":
                        # Extract node name
                        const_inst_base = src.rsplit(".", 1)[0]
                        old_const_inst_to_delete.add(const_inst_base)

        # Allowed data1 edges: max PE cascade O0->data1
        allowed_d1 = set()
        for c in chain_to_ordered_pes:
            ordered = chain_to_ordered_pes[c]
            if not ordered:
                continue
            # PEk.O0 -> PE(k+1).data1
            for u, v in zip(ordered[:-1], ordered[1:]):
                allowed_d1.add((v + ".data1", u + ".O0"))

        # -----Identify IO and MEMs per chain and only keep one MEM per chain-----
        chain_io = {}
        chain_mems = defaultdict(Counter)
        for a, b in connections:
            for src, dst in ((a, b), (b, a)):
                if io_out_pat.match(src):
                    md = floatmax_pat.match(dst)
                    if md and md.group("pin") == "data0":
                        chain_io[int(md.group("chain"))] = src
                mout = mem_out_pat.match(src)
                mdst = floatmax_pat.match(dst)
                if mout and mdst and mdst.group("pin") == "data0":
                    chain = int(mdst.group("chain"))
                    chain_mems[chain][mout.group("mem")] += 1

        chain_mem_keep = {}
        for c in chain_ids:
            if chain_mems[c]:
                chain_mem_keep[c] = chain_mems[c].most_common(1)[0][0]
            else:
                any_mem = next((n for n in instances if mem_any_pat.match(n)), None)
                if any_mem:
                    chain_mem_keep[c] = any_mem

        # -----Remove old feeds into PE.data0 and mark MEMs to delete-----
        to_delete_mems = set()
        for c in chain_ids:
            keep = chain_mem_keep.get(c)
            if keep:
                for mname in chain_mems[c]:
                    if mname != keep:
                        to_delete_mems.add(mname)

        # Determine compute PEs
        pe_data0_targets = set()
        for c in chain_ids:
            ordered = chain_to_ordered_pes[c]
            for base in ordered:
                pe_data0_targets.add(base + ".data0")

        filtered = []
        for a, b in connections:
            drop = False

            # Drop edges with deleted MEMs
            for ep in (a, b):
                ma = mem_any_pat.match(ep)
                if ma and ma.group("mem") in to_delete_mems:
                    drop = True
                    break
            if drop:
                continue

            # Drop edges with constant PEs (will be removed)
            for ep in (a, b):
                if const_pat.match(ep):
                    drop = True
                    break
            if drop:
                continue

            # Drop edges from old const instruction instances to first max PE.inst
            for src, dst in ((a, b), (b, a)):
                if const_inst_pat.match(src):
                    const_inst_base = src.rsplit(".", 1)[0]
                    if const_inst_base in old_const_inst_to_delete and dst.endswith(".inst"):
                        drop = True
                        break
            if drop:
                continue

            # Drop edges with compute PE.data0 targets waiting to be rewired
            if a in pe_data0_targets or b in pe_data0_targets:
                continue

            # Drop edges with MEM.data_out_* -> PE.data0 (even for kept MEMs and waiting to be rewired)
            for src, dst in ((a, b), (b, a)):
                if mem_out_pat.match(src) and dst in pe_data0_targets:
                    drop = True
                    break
            if drop:
                continue

            # Drop edges into max PE.data1 unless it is explicitly allowed
            def ends_at_disallowed_d1(x, y):
                return (x.endswith(".data1") and floatmax_pat.match(x) and (x, y) not in allowed_d1)

            if ends_at_disallowed_d1(a, b) or ends_at_disallowed_d1(b, a):
                continue

            filtered.append([a, b])
        connections = filtered

        # -----Rename kept MEMs (mem_c{chain}) and update endpoints-----
        def replace_endpoint_prefix(ep: str, old: str, new: str) -> str:
            return ep.replace(old + ".", new + ".") if ep.startswith(old + ".") else ep

        rename_map = {}
        for c in chain_ids:
            old = chain_mem_keep.get(c)
            if not old:
                continue
            new = f"mem_c{c}"
            unique = new
            k = 0
            while unique in instances and unique != old:
                k += 1
                unique = f"{new}_{k}"
            if unique != old:
                instances[unique] = instances.pop(old)
                rename_map[old] = unique

        if rename_map:
            updated = []
            for a, b in connections:
                na, nb = a, b
                for old, new in rename_map.items():
                    na = replace_endpoint_prefix(na, old, new)
                    nb = replace_endpoint_prefix(nb, old, new)
                updated.append([na, nb])
            connections = updated

        def mem_data_out(c: int, port: int) -> str:
            base = rename_map.get(chain_mem_keep[c], chain_mem_keep[c])
            return f"{base}.data_out_{port}"

        # -----Instantiate six FIFOs per chain and wire the line buffer graph-----
        shift_fifo_tpl = {
            "genref": "coreir.reg",
            "genargs": {"width": ["Int", 16]},
            "modargs": {"clk_posedge": ["Bool", True], "init": [["BitVector", 16], "16'h0000"]},
            "metadata": {"extra_data": 1},
        }

        # Define helper to create shift FIFO names
        def create_shift_fifo_name(base: str) -> str:
            if base not in instances:
                return base
            idx = 1
            while f"{base}_{idx}" in instances:
                idx += 1
            return f"{base}_{idx}"

        # Define helper to add connections
        def add_conn(dst: str, src: str):
            connections.append([dst, src])

        for c in chain_ids:
            ordered = chain_to_ordered_pes[c]
            if not ordered:
                continue

            compute_pes = ordered

            # Group PEs by IO, MEM port1, and MEM port0
            group_io = compute_pes[0:3]
            group_mem_port1 = compute_pes[3:6]
            group_mem_port0 = compute_pes[6:9]

            io_src = chain_io.get(c)
            if not io_src:
                # Pick any io.out in design
                for a, b in connections:
                    if io_out_pat.match(a): io_src = a; break
                    if io_out_pat.match(b): io_src = b; break
            if not io_src:
                continue

            # Create const instruction for first max PE
            first_pe = group_io[0] if group_io else None
            if first_pe:
                const_inst_name = f"first_pe_c{c}_inst"
                if const_inst_name not in instances:
                    instances[const_inst_name] = {
                        "genref": "coreir.const",
                        "genargs": {"width": ["Int", 84]},
                        "modargs": {"value": [["BitVector", 84], self.DUMMY_MAX_NOP_INSTR]},
                    }
                add_conn(first_pe + ".inst", const_inst_name + ".out")

            # Create six FIFOs per chain
            names = [
                create_shift_fifo_name(f"fifo_c{c}_io_0"),
                create_shift_fifo_name(f"fifo_c{c}_io_1"),
                create_shift_fifo_name(f"fifo_c{c}_p1_0"),
                create_shift_fifo_name(f"fifo_c{c}_p1_1"),
                create_shift_fifo_name(f"fifo_c{c}_p0_0"),
                create_shift_fifo_name(f"fifo_c{c}_p0_1"),
            ]
            for fname in names:
                instances[fname] = copy.deepcopy(shift_fifo_tpl)

            fifo_io0, fifo_io1, fifo_p10, fifo_p11, fifo_p00, fifo_p01 = names

            # IO path: IO->PE1, IO->fifo0->PE2, IO->fifo0->fifo1->PE3
            if len(group_io) >= 1: add_conn(group_io[0] + ".data0", io_src)
            add_conn(fifo_io0 + ".in", io_src)
            if len(group_io) >= 2: add_conn(group_io[1] + ".data0", fifo_io0 + ".out")
            add_conn(fifo_io1 + ".in", fifo_io0 + ".out")
            if len(group_io) >= 3: add_conn(group_io[2] + ".data0", fifo_io1 + ".out")

            # MEM port1: port1->PE4, ->fifo2->PE5, ->fifo2->fifo3->PE6
            if group_mem_port1:
                p1_src = mem_data_out(c, 1)
                add_conn(group_mem_port1[0] + ".data0", p1_src)
                add_conn(fifo_p10 + ".in", p1_src)
                if len(group_mem_port1) >= 2: add_conn(group_mem_port1[1] + ".data0", fifo_p10 + ".out")
                add_conn(fifo_p11 + ".in", fifo_p10 + ".out")
                if len(group_mem_port1) >= 3: add_conn(group_mem_port1[2] + ".data0", fifo_p11 + ".out")

            # MEM port0: port0->PE7, ->fifo4->PE8, ->fifo4->fifo5->PE9
            if group_mem_port0:
                p0_src = mem_data_out(c, 0)
                add_conn(group_mem_port0[0] + ".data0", p0_src)
                add_conn(fifo_p00 + ".in", p0_src)
                if len(group_mem_port0) >= 2: add_conn(group_mem_port0[1] + ".data0", fifo_p00 + ".out")
                add_conn(fifo_p01 + ".in", fifo_p00 + ".out")
                if len(group_mem_port0) >= 3: add_conn(group_mem_port0[2] + ".data0", fifo_p01 + ".out")

        # -----Delete unused MEMs and constant PEs, drop dangling edges-----
        for m in to_delete_mems:
            if m in instances:
                del instances[m]

        # Delete constant PEs
        for name in list(instances.keys()):
            if const_pat.match(name):
                del instances[name]

        # Delete only old const instruction instances connected to first max PE in each chain
        for name in old_const_inst_to_delete:
            if name in instances:
                del instances[name]

        deleted_prefixes = tuple(m + "." for m in to_delete_mems)
        pruned = []
        seen = set()
        for d, s in connections:
            if d.startswith(deleted_prefixes) or s.startswith(deleted_prefixes):
                continue
            key = (d, s)
            if key in seen:
                continue
            seen.add(key)
            pruned.append([d, s])

        module["connections"] = pruned

        # -----Add dummy_max_nop PEs at the end of each PE chain lane before output IOs-----
        dummy_max_nop = int(self.halide_gen_args_dict.get("dummy_max_nop", 0))
        if dummy_max_nop > 0:
            # Pattern to match output IOs (for maxpooling_dense_rv_fp)
            output_io_pat = re.compile(r"^io16.*hw_output.*\.in$")

            # Find the last PE in each chain (the one without a next PE)
            chain_last_pe = {}
            for c in chain_ids:
                ordered = chain_to_ordered_pes[c]
                if not ordered:
                    continue
                if ordered:
                    # Last PE is the last one in the compute PEs
                    chain_last_pe[c] = ordered[-1]

            # Find connections from last PE.O0 to output IO.in
            pe_to_io_connections = []
            for idx, conn in enumerate(pruned):
                dst, src = conn[0], conn[1]
                # Check if src is a last PE's O0 and dst is an output IO
                for chain, last_pe in chain_last_pe.items():
                    if src == f"{last_pe}.O0" and output_io_pat.match(dst):
                        pe_to_io_connections.append((idx, chain, last_pe, dst))
                        break
                    # Also check reverse direction
                    if dst == f"{last_pe}.O0" and output_io_pat.match(src):
                        pe_to_io_connections.append((idx, chain, last_pe, src))
                        break

            # Remove connections to be rewired (process in reverse order to maintain indices)
            indices_to_remove = sorted([idx for idx, _, _, _ in pe_to_io_connections], reverse=True)
            for idx in indices_to_remove:
                pruned.pop(idx)

            # Create dummy PEs and rewire connections
            for _, chain, last_pe, io_in_port in pe_to_io_connections:

                # Create dummy_max_nop PEs for this chain
                dummy_pe_names = []
                dummy_const_names = []
                for i in range(dummy_max_nop):
                    dummy_pe_name = f"dummy_max_nop_c{chain}_pe{i}"
                    dummy_const_name = f"dummy_max_nop_c{chain}_const{i}"

                    # Create const instruction instance
                    if dummy_const_name not in instances:
                        instances[dummy_const_name] = {
                            "genref": "coreir.const",
                            "genargs": {"width": ["Int", 84]},
                            "modargs": {"value": [["BitVector", 84], self.DUMMY_MAX_NOP_INSTR]},
                        }

                    # Create PE instance
                    if dummy_pe_name not in instances:
                        instances[dummy_pe_name] = {"modref": "global.PE"}

                    dummy_pe_names.append(dummy_pe_name)
                    dummy_const_names.append(dummy_const_name)

                # Wire up the chain: last_pe.O0 -> first_dummy.data0
                if dummy_pe_names:
                    pruned.append([f"{dummy_pe_names[0]}.data0", f"{last_pe}.O0"])
                    pruned.append([f"{dummy_pe_names[0]}.inst", f"{dummy_const_names[0]}.out"])

                    # Wire up dummy PEs in chain: dummy[i].O0 -> dummy[i+1].data0
                    for i in range(len(dummy_pe_names) - 1):
                        pruned.append([f"{dummy_pe_names[i+1]}.data0", f"{dummy_pe_names[i]}.O0"])
                        pruned.append([f"{dummy_pe_names[i+1]}.inst", f"{dummy_const_names[i+1]}.out"])

                    # Wire last dummy PE to output IO
                    pruned.append([io_in_port, f"{dummy_pe_names[-1]}.O0"])
                else:
                    # If dummy_max_nop is 0, just reconnect (shouldn't happen due to check above)
                    pruned.append([io_in_port, f"{last_pe}.O0"])

            # Update module connections
            module["connections"] = pruned

        # -----Configure input and output IOs DMA-----
        img_size = int(self.halide_gen_args_dict["in_img"])
        n_ic = int(self.halide_gen_args_dict["n_ic"])
        ksize = int(self.halide_gen_args_dict["ksize"])
        stride = int(self.halide_gen_args_dict["stride"])
        unroll = int(self.halide_gen_args_dict["unroll"])
        channel_per_lane = n_ic // unroll
        out_img_size = (img_size - ksize) // stride + 1
        cycle_stride_y = stride * ((img_size // stride) + (ksize - 1))
        row_tail_cycles = (out_img_size - 1) * stride
        cycle_stride_c = row_tail_cycles + stride * cycle_stride_y - img_size
        for io_instance in instances:
            # Two cases:
            # 1. n_ic == unroll, then each IO stores data continously
            # 2. n_ic // unroll > 1, then needs n_ic // unroll blocks with read/write data stride
            if "io16in_input_host_stencil" in io_instance:
                if n_ic == unroll:
                    instances[io_instance]["metadata"]["glb2out_0"]["cycle_starting_addr"] = [0]
                    instances[io_instance]["metadata"]["glb2out_0"]["cycle_stride"] = [1]
                    instances[io_instance]["metadata"]["glb2out_0"]["dimensionality"] = 1
                    instances[io_instance]["metadata"]["glb2out_0"]["extent"] = [img_size * img_size]
                    instances[io_instance]["metadata"]["glb2out_0"]["read_data_starting_addr"] = [0]
                    instances[io_instance]["metadata"]["glb2out_0"]["read_data_stride"] = [1]
                else:
                    assert n_ic % unroll == 0, "n_ic must be divisible by unroll"
                    instances[io_instance]["metadata"]["glb2out_0"]["cycle_starting_addr"] = [0]
                    instances[io_instance]["metadata"]["glb2out_0"]["cycle_stride"] = [1, 1]
                    instances[io_instance]["metadata"]["glb2out_0"]["dimensionality"] = 2
                    instances[io_instance]["metadata"]["glb2out_0"]["extent"] = [(img_size - 1) * img_size, channel_per_lane]
                    instances[io_instance]["metadata"]["glb2out_0"]["read_data_starting_addr"] = [0]
                    instances[io_instance]["metadata"]["glb2out_0"]["read_data_stride"] = [channel_per_lane, 1 - channel_per_lane * ((img_size - 1) * img_size - 1)]

            elif "io16_hw_output" in io_instance:
                if n_ic == unroll:
                    # Skip dummy data for line buffer shifting at the beginning
                    # Which is two lines of data plus the kernel size - 1
                    instances[io_instance]["metadata"]["in2glb_0"]["cycle_starting_addr"] = [img_size * 2 + ksize - 1]
                    # instances[io_instance]["metadata"]["in2glb_0"]["cycle_stride"] = [stride, img_size * stride]
                    # Directly use "hardware-friendly" cycle stride
                    instances[io_instance]["metadata"]["in2glb_0"]["cycle_stride"] = [stride, img_size * stride - (out_img_size - 1) * stride]
                    instances[io_instance]["metadata"]["in2glb_0"]["dimensionality"] = 2
                    instances[io_instance]["metadata"]["in2glb_0"]["extent"] = [out_img_size, out_img_size]
                    instances[io_instance]["metadata"]["in2glb_0"]["write_data_starting_addr"] = [0]
                    instances[io_instance]["metadata"]["in2glb_0"]["write_data_stride"] = [1, out_img_size]
                else:
                    assert n_ic % unroll == 0, "n_ic must be divisible by unroll"
                    instances[io_instance]["metadata"]["in2glb_0"]["cycle_starting_addr"] = [img_size * 2 + ksize - 1]
                    # instances[io_instance]["metadata"]["in2glb_0"]["cycle_stride"] = [stride, img_size * stride - (out_img_size - 1) * stride, img_size * 2 + ksize]
                    instances[io_instance]["metadata"]["in2glb_0"]["cycle_stride"] = [stride, img_size * stride - row_tail_cycles, cycle_stride_c]
                    instances[io_instance]["metadata"]["in2glb_0"]["dimensionality"] = 3
                    instances[io_instance]["metadata"]["in2glb_0"]["extent"] = [out_img_size, out_img_size, channel_per_lane]
                    instances[io_instance]["metadata"]["in2glb_0"]["write_data_starting_addr"] = [0]
                    instances[io_instance]["metadata"]["in2glb_0"]["write_data_stride"] = [channel_per_lane, channel_per_lane, 1 - channel_per_lane * (out_img_size * out_img_size - 1)]

        # -----Overwrite the JSON-----
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

        # -----Update design_meta_halide.json with correct input and output shapes-----
        design_meta_path = os.path.join(bin_path, "design_meta_halide.json")
        with open(design_meta_path, "r") as f:
            design_meta = json.load(f)
        assert len(design_meta["IOs"]["inputs"]) == 1, "Expected only one input"
        assert len(design_meta["IOs"]["outputs"]) == 1, "Expected only one output"
        design_meta["IOs"]["inputs"][0]["shape"] = [n_ic, img_size, img_size]
        design_meta["IOs"]["outputs"][0]["shape"] = [n_ic, (img_size - ksize) // stride + 1, (img_size - ksize) // stride + 1]

        with open(design_meta_path, "w") as f:
            json.dump(design_meta, f, indent=2)

    def hack_for_tanh_fp_rv(self, json_path, bin_path):
        """
        Insert output buffer MEM tiles between the final fp_mul PE (hardcoded to float_DW_fp_mul_i3142i11_i1325)
        and each output IO to handle backpressure and avoid F2G errors.
        """
        # Load the JSON file
        with open(json_path, "r") as f:
            design = json.load(f)

        # Find the top module
        design_name = "tanh_fp"
        top_module = design["namespaces"]["global"]["modules"][design_name]

        instances = top_module["instances"]
        connections = top_module["connections"]

        # The final fp_mul PE instance name
        original_pe_name = "op_hcompute_output_cgra_stencil$inner_compute$float_DW_fp_mul_i3142i11_i1325"

        if original_pe_name not in instances:
            raise RuntimeError(f"[ERROR] PE {original_pe_name} not found.")

        # Find all output IOs that connect to this PE's output
        # Connections can be [src, dst] or [dst, src] format
        output_io_connections = []
        for conn in connections:
            a, b = conn
            # Check if this connection goes from the PE output to an output IO
            if a.startswith(f"{original_pe_name}.O0") and "io16_hw_output_stencil_clkwrk" in b:
                # Format: [src, dst] - PE output is source
                io_inst_name = b.split(".in")[0]
                output_io_connections.append((a, b, io_inst_name))
            elif b.startswith(f"{original_pe_name}.O0") and "io16_hw_output_stencil_clkwrk" in a:
                # Format: [dst, src] - PE output is source (reversed)
                io_inst_name = a.split(".in")[0]
                output_io_connections.append((b, a, io_inst_name))

        # Get unique output IOs
        unique_output_ios = list(set(io_inst_name for _, _, io_inst_name in output_io_connections))
        num_output_ios = len(unique_output_ios)

        if num_output_ios == 0:
            raise RuntimeError(f"[ERROR] No output IOs found connected to {original_pe_name}.")

        # Create MEM buffer instances and update connections
        new_instances = {}
        new_connections = []
        connections_to_keep = []

        # Process each connection from PE to output IO
        for src, dst, io_inst_name in output_io_connections:
            # Create a unique MEM buffer name for this output IO
            mem_buffer_name = f"{original_pe_name}_output_buffer_{io_inst_name}"

            # Create MEM buffer instance
            if mem_buffer_name not in instances:
                mem_inst = copy.deepcopy(self.mem_tpl)
                new_instances[mem_buffer_name] = mem_inst

            # Add new connections: PE -> MEM -> IO
            new_connections.append([f"{original_pe_name}.O0", f"{mem_buffer_name}.data_in_0"])
            new_connections.append([f"{mem_buffer_name}.data_out_0", f"{io_inst_name}.in"])

        # Keep all connections except the ones from PE output to output IOs
        for conn in connections:
            a, b = conn
            # Skip connections from PE output directly to output IOs (we're inserting buffers)
            is_direct_pe_to_io = (
                (a.startswith(f"{original_pe_name}.O0") and "io16_hw_output_stencil_clkwrk" in b) or
                (b.startswith(f"{original_pe_name}.O0") and "io16_hw_output_stencil_clkwrk" in a)
            )
            if not is_direct_pe_to_io:
                connections_to_keep.append(conn)

        # Add new MEM instances
        instances.update(new_instances)

        # Update connections: keep existing (minus removed direct connections) and add new ones
        top_module["connections"] = connections_to_keep + new_connections

        # Configure io16_hw_output_stencil metadata with default 1D schedule
        vec_len = int(self.halide_gen_args_dict["vec_len"])
        glb_o = int(self.halide_gen_args_dict["glb_o"])
        extent = vec_len // glb_o

        for inst_name, inst_config in instances.items():
            if "io16_hw_output_stencil" in inst_name and inst_config.get("modref") == "global.IO":
                if "metadata" not in inst_config:
                    inst_config["metadata"] = {}
                if "in2glb_0" not in inst_config["metadata"]:
                    inst_config["metadata"]["in2glb_0"] = {}
                md = inst_config["metadata"]["in2glb_0"]
                md["cycle_starting_addr"] = [0]
                md["cycle_stride"] = [1]
                md["dimensionality"] = 1
                md["extent"] = [extent]
                md["write_data_starting_addr"] = [0]
                md["write_data_stride"] = [1]

        # Write back the JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

        print(f"\033[92m[INFO] Inserted {num_output_ios} output buffer MEM tiles between {original_pe_name} and output IOs\033[0m")

    def hack_for_maxpooling_dense_rv_mem_buf_fp_rv(self, json_path, bin_path):
        '''
        Unhacked compute graph consists of unroll number of PE chains with IOs and MEMs servring as line buffers.
        Some chain use one MEM and some use two, while one MEM per chain is enough.
        To handle multiple channels per lane, unhacked graph uses n_ic // unroll FIFOs between adjacent PEs to interleave across channels.
        Dense RV maxpooling is not compilable with clockwork, so there are redundant FIFOs for compute delay matching.
        This hack collapses all redundant FIFOs, removes redundant MEMs and constant PEs, hardcodes the first max PE instruction
        with DUMMY_MAX_NOP_INSTR, and configures GLB DMA to handle multiple channels per lane.
        Add mem buffering to use different ports for each PE to avoid path imbalance.
        '''
        with open(json_path, "r") as f:
            design = json.load(f)

        top_module_name = "maxpooling_dense_rv_mem_buf_fp"
        module = design["namespaces"]["global"]["modules"][top_module_name]
        instances = module["instances"]
        connections = module["connections"]

        # -----Collapse all shift FIFO $d_reg chains-----
        # Define helpers to identify shift chains
        def is_shift(edge_point: str) -> bool:
            return "$d_reg" in edge_point

        def is_shift_in(edge_point: str) -> bool:
            return is_shift(edge_point) and edge_point.endswith(".in")

        def is_shift_out(edge_point: str) -> bool:
            return is_shift(edge_point) and edge_point.endswith(".out")

        def inst_of(edge_point: str) -> str:
            return edge_point.rsplit(".", 1)[0]

        # Collect directed views of shift chains
        shift_in_driver = {}
        shift_out_fanout = defaultdict(set)
        for a, b in connections:
            if is_shift_out(a): shift_out_fanout[a].add(b)
            if is_shift_out(b): shift_out_fanout[b].add(a)
            if is_shift_in(a): shift_in_driver[a] = b
            if is_shift_in(b): shift_in_driver[b] = a

        head_in_ports = [ip for ip, drv in shift_in_driver.items() if not is_shift_out(drv)]

        bridged = set()
        for head_in in head_in_ports:
            upstream_src = shift_in_driver[head_in]
            head_out = f"{inst_of(head_in)}.out"
            stack = [head_out]
            visited_out = set()
            sinks = set()
            while stack:
                outp = stack.pop()
                if outp in visited_out:
                    continue
                visited_out.add(outp)
                for nxt in shift_out_fanout.get(outp, []):
                    if is_shift_in(nxt):
                        stack.append(f"{inst_of(nxt)}.out")
                    else:
                        sinks.add(nxt)
            for dst in sinks:
                bridged.add((dst, upstream_src))

        kept = []
        for a, b in connections:
            if is_shift(a) or is_shift(b):
                continue
            kept.append([a, b])

        tmp = []
        seen = set()
        for d, s in kept + [[d, s] for (d, s) in sorted(bridged)]:
            key = (d, s)
            if key in seen: continue
            seen.add(key)
            tmp.append([d, s])
        connections = tmp

        for name in list(instances.keys()):
            if "$d_reg" in name:
                del instances[name]

        # -----Collect PE chains-----
        # Define patterns for PEs, MEMs, and IOs. ChatGPT generated regexes.
        floatmax_pat = re.compile(
            r"^(?P<base>op_hcompute_max_pooling_inner_stencil_(?P<chain>\d+)"
            r"\$inner_compute\$float_max_[^\.]+)\.(?P<pin>.+)$"
        )
        const_pat = re.compile(
            r"^(?P<base>op_hcompute_max_pooling_inner_stencil(?:_(?P<chain>\d+))?"
            r"\$inner_compute\$const_i\d+_i\d+)\.(?P<pin>.+)$"
        )
        const_inst_pat = re.compile(
            r"^op_hcompute_max_pooling_inner_stencil(?:_\d+)?\$inner_compute\$c\d+\.out$"
        )
        io_out_pat = re.compile(r"^io16in_input_host_stencil_clkwrk_\d+_.+_read_0\.out$")
        mem_out_pat = re.compile(
            r"^(?P<mem>input_host_global_wrapper_global_wrapper_stencil"
            r"\$ub_input_host_global_wrapper_global_wrapper_stencil_[^\.]+_garnet)\.data_out_(?P<port>[01])$"
        )
        mem_any_pat = re.compile(
            r"^(?P<mem>input_host_global_wrapper_global_wrapper_stencil"
            r"\$ub_input_host_global_wrapper_global_wrapper_stencil_[^\.]+_garnet)\."
        )

        # Collect all max PEs per chain with O0->data1 conns
        chain_pe_set = defaultdict(set)
        pe_next = defaultdict(dict)
        pe_prev = defaultdict(dict)

        for a, b in connections:
            for ep in (a, b):
                m = floatmax_pat.match(ep)
                if m:
                    chain_pe_set[int(m.group("chain"))].add(m.group("base"))
            for src, dst in ((a, b), (b, a)):
                ms = floatmax_pat.match(src)
                md = floatmax_pat.match(dst)
                if not (ms and md):
                    continue
                if ms.group("pin") != "O0" or md.group("pin") != "data1":
                    continue
                c = int(ms.group("chain"))
                if c != int(md.group("chain")):
                    continue
                u = ms.group("base")
                v = md.group("base")
                pe_next[c][u] = v
                pe_prev[c][v] = u

        # Identify head max PE from const.O0 -> max.data0
        chain_head_max = {}
        chain_const_base = {}
        for a, b in connections:
            for src, dst in ((a, b), (b, a)):
                mc = const_pat.match(src)
                md = floatmax_pat.match(dst)
                if not (mc and md):
                    continue
                if mc.group("pin") != "O0" or md.group("pin") != "data0":
                    continue
                chain = int(mdst_chain := md.group("chain"))
                chain_head_max[chain] = md.group("base")
                chain_const_base[chain] = mc.group("base")

        # Order PEs: walk from first max PE via O0->data1
        chain_to_ordered_pes = {}
        for chain, pes in chain_pe_set.items():
            head_max = chain_head_max.get(chain)
            if not head_max:
                head_candidates = [p for p in pes if p not in pe_prev[chain]]
                head_max = sorted(head_candidates)[0] if head_candidates else sorted(pes)[0]
            order = []
            cur = head_max
            visited = set()
            while cur and cur not in visited:
                order.append(cur)
                visited.add(cur)
                cur = pe_next[chain].get(cur)
            chain_to_ordered_pes[chain] = order

        chain_ids = [c for c in sorted(chain_to_ordered_pes) if len(chain_to_ordered_pes[c]) >= 1]

        # Identify first max PE per chain and collect old const instruction instances connected to them
        first_pe_per_chain = {}
        old_const_inst_to_delete = set()
        for c in chain_ids:
            ordered = chain_to_ordered_pes[c]
            if not ordered:
                continue
            first_pe = ordered[0]
            first_pe_per_chain[c] = first_pe
            # Find const instruction instances connected to first PE's .inst port
            for a, b in connections:
                for src, dst in ((a, b), (b, a)):
                    if const_inst_pat.match(src) and dst == first_pe + ".inst":
                        # Extract node name
                        const_inst_base = src.rsplit(".", 1)[0]
                        old_const_inst_to_delete.add(const_inst_base)

        # Allowed data1 edges: max PE cascade O0->data1
        allowed_d1 = set()
        for c in chain_to_ordered_pes:
            ordered = chain_to_ordered_pes[c]
            if not ordered:
                continue
            # PEk.O0 -> PE(k+1).data1
            for u, v in zip(ordered[:-1], ordered[1:]):
                allowed_d1.add((v + ".data1", u + ".O0"))

        # -----Identify IO and MEMs per chain and only keep one MEM per chain-----
        chain_io = {}
        chain_mems = defaultdict(Counter)
        for a, b in connections:
            for src, dst in ((a, b), (b, a)):
                if io_out_pat.match(src):
                    md = floatmax_pat.match(dst)
                    if md and md.group("pin") == "data0":
                        chain_io[int(md.group("chain"))] = src
                mout = mem_out_pat.match(src)
                mdst = floatmax_pat.match(dst)
                if mout and mdst and mdst.group("pin") == "data0":
                    chain = int(mdst.group("chain"))
                    chain_mems[chain][mout.group("mem")] += 1

        chain_mem_keep = {}
        for c in chain_ids:
            if chain_mems[c]:
                chain_mem_keep[c] = chain_mems[c].most_common(1)[0][0]
            else:
                any_mem = next((n for n in instances if mem_any_pat.match(n)), None)
                if any_mem:
                    chain_mem_keep[c] = any_mem

        # -----Remove old feeds into PE.data0 and mark MEMs to delete-----
        # Since we're creating new MEMs, mark all old MEMs for deletion
        to_delete_mems = set()
        for c in chain_ids:
            # Delete all old MEMs for this chain (we'll create new ones)
            for mname in chain_mems[c]:
                to_delete_mems.add(mname)

        # Determine compute PEs
        pe_data0_targets = set()
        for c in chain_ids:
            ordered = chain_to_ordered_pes[c]
            for base in ordered:
                pe_data0_targets.add(base + ".data0")

        filtered = []
        for a, b in connections:
            drop = False

            # Drop edges with deleted MEMs
            for ep in (a, b):
                ma = mem_any_pat.match(ep)
                if ma and ma.group("mem") in to_delete_mems:
                    drop = True
                    break
            if drop:
                continue

            # Drop edges with constant PEs (will be removed)
            for ep in (a, b):
                if const_pat.match(ep):
                    drop = True
                    break
            if drop:
                continue

            # Drop edges from old const instruction instances to first max PE.inst
            for src, dst in ((a, b), (b, a)):
                if const_inst_pat.match(src):
                    const_inst_base = src.rsplit(".", 1)[0]
                    if const_inst_base in old_const_inst_to_delete and dst.endswith(".inst"):
                        drop = True
                        break
            if drop:
                continue

            # Drop edges with compute PE.data0 targets waiting to be rewired
            if a in pe_data0_targets or b in pe_data0_targets:
                continue

            # Drop edges with MEM.data_out_* -> PE.data0 (even for kept MEMs and waiting to be rewired)
            for src, dst in ((a, b), (b, a)):
                if mem_out_pat.match(src) and dst in pe_data0_targets:
                    drop = True
                    break
            if drop:
                continue

            # Drop edges into max PE.data1 unless it is explicitly allowed
            def ends_at_disallowed_d1(x, y):
                return (x.endswith(".data1") and floatmax_pat.match(x) and (x, y) not in allowed_d1)

            if ends_at_disallowed_d1(a, b) or ends_at_disallowed_d1(b, a):
                continue

            filtered.append([a, b])
        connections = filtered

        # -----Create four MEMs per chain and wire connections without branching-----
        # Define helper to add connections
        def add_conn(dst: str, src: str):
            connections.append([dst, src])

        # Get a template MEM from existing MEMs to use as base
        template_mem = None
        # First try to get from chain_mem_keep
        for c in chain_ids:
            old = chain_mem_keep.get(c)
            if old and old in instances:
                template_mem = instances[old]
                break
        # If not found, try to get any MEM from instances
        if not template_mem:
            for inst_name, inst_data in instances.items():
                if mem_any_pat.match(inst_name):
                    template_mem = inst_data
                    break
        # Final fallback: use mem_tpl
        if not template_mem:
            template_mem = copy.deepcopy(self.mem_tpl)

        for c in chain_ids:
            ordered = chain_to_ordered_pes[c]
            if not ordered:
                continue

            compute_pes = ordered

            io_src = chain_io.get(c)
            if not io_src:
                # Pick any io.out in design
                for a, b in connections:
                    if io_out_pat.match(a): io_src = a; break
                    if io_out_pat.match(b): io_src = b; break
            if not io_src:
                continue

            # Create const instruction for first max PE
            first_pe = compute_pes[0] if compute_pes else None
            if first_pe:
                const_inst_name = f"first_pe_c{c}_inst"
                if const_inst_name not in instances:
                    instances[const_inst_name] = {
                        "genref": "coreir.const",
                        "genargs": {"width": ["Int", 84]},
                        "modargs": {"value": [["BitVector", 84], self.DUMMY_MAX_NOP_INSTR]},
                    }
                add_conn(first_pe + ".inst", const_inst_name + ".out")

            # Create four MEMs per chain: mem_c{c}_0, mem_c{c}_1, mem_c{c}_2, mem_c{c}_3
            mem_names = []
            clk_en_names = []
            for mem_idx in range(4):
                mem_name = f"mem_c{c}_{mem_idx}"
                clk_en_name = f"mem_c{c}_{mem_idx}_clk_en_const"

                # Create MEM instance
                if mem_name not in instances:
                    instances[mem_name] = copy.deepcopy(template_mem)

                # Create clk_en_const for this MEM
                if clk_en_name not in instances:
                    instances[clk_en_name] = copy.deepcopy(self.const_clk_tpl)

                mem_names.append(mem_name)
                clk_en_names.append(clk_en_name)

                # Connect clk_en
                add_conn(mem_name + ".clk_en", clk_en_name + ".out")

                # Connect IO to MEM data_in_0
                add_conn(mem_name + ".data_in_0", io_src)

            # -----Add dummy_max_nop_in PEs at the beginning of each PE chain-----
            dummy_max_nop_in = int(self.halide_gen_args_dict.get("dummy_max_nop_in", 0))
            first_pe_data0_src = io_src  # Default: connect PE0 directly to IO

            if dummy_max_nop_in > 0 and first_pe:
                # Create dummy_max_nop_in PEs for this chain
                dummy_pe_names_in = []
                dummy_const_names_in = []
                for i in range(dummy_max_nop_in):
                    dummy_pe_name = f"dummy_max_nop_in_c{c}_pe{i}"
                    dummy_const_name = f"dummy_max_nop_in_c{c}_const{i}"

                    # Create const instruction instance
                    if dummy_const_name not in instances:
                        instances[dummy_const_name] = {
                            "genref": "coreir.const",
                            "genargs": {"width": ["Int", 84]},
                            "modargs": {"value": [["BitVector", 84], self.DUMMY_MAX_NOP_INSTR]},
                        }

                    # Create PE instance
                    if dummy_pe_name not in instances:
                        instances[dummy_pe_name] = {"modref": "global.PE"}

                    dummy_pe_names_in.append(dummy_pe_name)
                    dummy_const_names_in.append(dummy_const_name)

                # Wire IO to first dummy PE
                if dummy_pe_names_in:
                    add_conn(dummy_pe_names_in[0] + ".data0", io_src)
                    add_conn(dummy_pe_names_in[0] + ".inst", dummy_const_names_in[0] + ".out")

                    # Wire up dummy PEs in chain: dummy[i].O0 -> dummy[i+1].data0
                    for i in range(len(dummy_pe_names_in) - 1):
                        add_conn(dummy_pe_names_in[i+1] + ".data0", dummy_pe_names_in[i] + ".O0")
                        add_conn(dummy_pe_names_in[i+1] + ".inst", dummy_const_names_in[i+1] + ".out")

                    # Last dummy PE's O0 will connect to PE0's data0
                    first_pe_data0_src = dummy_pe_names_in[-1] + ".O0"

            # Wire PEs: PE0 from IO (or last dummy PE if dummy_max_nop_in > 0), PE1-8 from MEM outputs
            # PE0: from IO or last dummy PE
            if len(compute_pes) >= 1:
                add_conn(compute_pes[0] + ".data0", first_pe_data0_src)

            # PE1: from mem_c{c}_0.data_out_0
            if len(compute_pes) >= 2:
                add_conn(compute_pes[1] + ".data0", mem_names[0] + ".data_out_0")

            # PE2: from mem_c{c}_0.data_out_1
            if len(compute_pes) >= 3:
                add_conn(compute_pes[2] + ".data0", mem_names[0] + ".data_out_1")

            # PE3: from mem_c{c}_1.data_out_0
            if len(compute_pes) >= 4:
                add_conn(compute_pes[3] + ".data0", mem_names[1] + ".data_out_0")

            # PE4: from mem_c{c}_1.data_out_1
            if len(compute_pes) >= 5:
                add_conn(compute_pes[4] + ".data0", mem_names[1] + ".data_out_1")

            # PE5: from mem_c{c}_2.data_out_0
            if len(compute_pes) >= 6:
                add_conn(compute_pes[5] + ".data0", mem_names[2] + ".data_out_0")

            # PE6: from mem_c{c}_2.data_out_1
            if len(compute_pes) >= 7:
                add_conn(compute_pes[6] + ".data0", mem_names[2] + ".data_out_1")

            # PE7: from mem_c{c}_3.data_out_0
            if len(compute_pes) >= 8:
                add_conn(compute_pes[7] + ".data0", mem_names[3] + ".data_out_0")

            # PE8: from mem_c{c}_3.data_out_1
            if len(compute_pes) >= 9:
                add_conn(compute_pes[8] + ".data0", mem_names[3] + ".data_out_1")

        # -----Delete unused MEMs and constant PEs, drop dangling edges-----
        for m in to_delete_mems:
            if m in instances:
                del instances[m]

        # Delete constant PEs
        for name in list(instances.keys()):
            if const_pat.match(name):
                del instances[name]

        # Delete only old const instruction instances connected to first max PE in each chain
        for name in old_const_inst_to_delete:
            if name in instances:
                del instances[name]

        deleted_prefixes = tuple(m + "." for m in to_delete_mems)
        pruned = []
        seen = set()
        for d, s in connections:
            if d.startswith(deleted_prefixes) or s.startswith(deleted_prefixes):
                continue
            key = (d, s)
            if key in seen:
                continue
            seen.add(key)
            pruned.append([d, s])

        module["connections"] = pruned

        # -----Add dummy_max_nop PEs at the end of each PE chain lane before output IOs-----
        dummy_max_nop = int(self.halide_gen_args_dict.get("dummy_max_nop", 0))
        if dummy_max_nop > 0:
            # Pattern to match output IOs (for maxpooling_dense_rv_mem_buf_fp)
            output_io_pat = re.compile(r"^io16.*hw_output.*\.in$")

            # Find the last PE in each chain (the one without a next PE)
            chain_last_pe = {}
            for c in chain_ids:
                ordered = chain_to_ordered_pes[c]
                if not ordered:
                    continue
                if ordered:
                    # Last PE is the last one in the compute PEs
                    chain_last_pe[c] = ordered[-1]

            # Find connections from last PE.O0 to output IO.in
            pe_to_io_connections = []
            for idx, conn in enumerate(pruned):
                dst, src = conn[0], conn[1]
                # Check if src is a last PE's O0 and dst is an output IO
                for chain, last_pe in chain_last_pe.items():
                    if src == f"{last_pe}.O0" and output_io_pat.match(dst):
                        pe_to_io_connections.append((idx, chain, last_pe, dst))
                        break
                    # Also check reverse direction
                    if dst == f"{last_pe}.O0" and output_io_pat.match(src):
                        pe_to_io_connections.append((idx, chain, last_pe, src))
                        break

            # Remove connections to be rewired (process in reverse order to maintain indices)
            indices_to_remove = sorted([idx for idx, _, _, _ in pe_to_io_connections], reverse=True)
            for idx in indices_to_remove:
                pruned.pop(idx)

            # Create dummy PEs and rewire connections
            for _, chain, last_pe, io_in_port in pe_to_io_connections:

                # Create dummy_max_nop PEs for this chain
                dummy_pe_names = []
                dummy_const_names = []
                for i in range(dummy_max_nop):
                    dummy_pe_name = f"dummy_max_nop_c{chain}_pe{i}"
                    dummy_const_name = f"dummy_max_nop_c{chain}_const{i}"

                    # Create const instruction instance
                    if dummy_const_name not in instances:
                        instances[dummy_const_name] = {
                            "genref": "coreir.const",
                            "genargs": {"width": ["Int", 84]},
                            "modargs": {"value": [["BitVector", 84], self.DUMMY_MAX_NOP_INSTR]},
                        }

                    # Create PE instance
                    if dummy_pe_name not in instances:
                        instances[dummy_pe_name] = {"modref": "global.PE"}

                    dummy_pe_names.append(dummy_pe_name)
                    dummy_const_names.append(dummy_const_name)

                # Wire up the chain: last_pe.O0 -> first_dummy.data0
                if dummy_pe_names:
                    pruned.append([f"{dummy_pe_names[0]}.data0", f"{last_pe}.O0"])
                    pruned.append([f"{dummy_pe_names[0]}.inst", f"{dummy_const_names[0]}.out"])

                    # Wire up dummy PEs in chain: dummy[i].O0 -> dummy[i+1].data0
                    for i in range(len(dummy_pe_names) - 1):
                        pruned.append([f"{dummy_pe_names[i+1]}.data0", f"{dummy_pe_names[i]}.O0"])
                        pruned.append([f"{dummy_pe_names[i+1]}.inst", f"{dummy_const_names[i+1]}.out"])

                    # Wire last dummy PE to output IO
                    pruned.append([io_in_port, f"{dummy_pe_names[-1]}.O0"])
                else:
                    # If dummy_max_nop is 0, just reconnect (shouldn't happen due to check above)
                    pruned.append([io_in_port, f"{last_pe}.O0"])

            # Update module connections
            module["connections"] = pruned

        # -----Configure input and output IOs DMA-----
        img_size = int(self.halide_gen_args_dict["in_img"])
        n_ic = int(self.halide_gen_args_dict["n_ic"])
        ksize = int(self.halide_gen_args_dict["ksize"])
        stride = int(self.halide_gen_args_dict["stride"])
        unroll = int(self.halide_gen_args_dict["unroll"])
        channel_per_lane = n_ic // unroll
        out_img_size = (img_size - ksize) // stride + 1
        cycle_stride_y = stride * ((img_size // stride) + (ksize - 1))
        row_tail_cycles = (out_img_size - 1) * stride
        cycle_stride_c = row_tail_cycles + stride * cycle_stride_y - img_size
        for io_instance in instances:
            # Two cases:
            # 1. n_ic == unroll, then each IO stores data continously
            # 2. n_ic // unroll > 1, then needs n_ic // unroll blocks with read/write data stride
            if "io16in_input_host_stencil" in io_instance:
                if n_ic == unroll:
                    instances[io_instance]["metadata"]["glb2out_0"]["cycle_starting_addr"] = [0]
                    instances[io_instance]["metadata"]["glb2out_0"]["cycle_stride"] = [1]
                    instances[io_instance]["metadata"]["glb2out_0"]["dimensionality"] = 1
                    instances[io_instance]["metadata"]["glb2out_0"]["extent"] = [img_size * img_size]
                    instances[io_instance]["metadata"]["glb2out_0"]["read_data_starting_addr"] = [0]
                    instances[io_instance]["metadata"]["glb2out_0"]["read_data_stride"] = [1]
                else:
                    assert n_ic % unroll == 0, "n_ic must be divisible by unroll"
                    instances[io_instance]["metadata"]["glb2out_0"]["cycle_starting_addr"] = [0]
                    instances[io_instance]["metadata"]["glb2out_0"]["cycle_stride"] = [1, 1]
                    instances[io_instance]["metadata"]["glb2out_0"]["dimensionality"] = 2
                    instances[io_instance]["metadata"]["glb2out_0"]["extent"] = [(img_size - 1) * img_size, channel_per_lane]
                    instances[io_instance]["metadata"]["glb2out_0"]["read_data_starting_addr"] = [0]
                    instances[io_instance]["metadata"]["glb2out_0"]["read_data_stride"] = [channel_per_lane, 1 - channel_per_lane * ((img_size - 1) * img_size - 1)]

            elif "io16_hw_output" in io_instance:
                if n_ic == unroll:
                    # Skip dummy data for line buffer shifting at the beginning
                    # Which is two lines of data plus the kernel size - 1
                    instances[io_instance]["metadata"]["in2glb_0"]["cycle_starting_addr"] = [img_size * 2 + ksize - 1]
                    # instances[io_instance]["metadata"]["in2glb_0"]["cycle_stride"] = [stride, img_size * stride]
                    # Directly use "hardware-friendly" cycle stride
                    instances[io_instance]["metadata"]["in2glb_0"]["cycle_stride"] = [stride, img_size * stride - (out_img_size - 1) * stride]
                    instances[io_instance]["metadata"]["in2glb_0"]["dimensionality"] = 2
                    instances[io_instance]["metadata"]["in2glb_0"]["extent"] = [out_img_size, out_img_size]
                    instances[io_instance]["metadata"]["in2glb_0"]["write_data_starting_addr"] = [0]
                    instances[io_instance]["metadata"]["in2glb_0"]["write_data_stride"] = [1, out_img_size]
                else:
                    assert n_ic % unroll == 0, "n_ic must be divisible by unroll"
                    instances[io_instance]["metadata"]["in2glb_0"]["cycle_starting_addr"] = [img_size * 2 + ksize - 1]
                    # instances[io_instance]["metadata"]["in2glb_0"]["cycle_stride"] = [stride, img_size * stride - (out_img_size - 1) * stride, img_size * 2 + ksize]
                    instances[io_instance]["metadata"]["in2glb_0"]["cycle_stride"] = [stride, img_size * stride - row_tail_cycles, cycle_stride_c]
                    instances[io_instance]["metadata"]["in2glb_0"]["dimensionality"] = 3
                    instances[io_instance]["metadata"]["in2glb_0"]["extent"] = [out_img_size, out_img_size, channel_per_lane]
                    instances[io_instance]["metadata"]["in2glb_0"]["write_data_starting_addr"] = [0]
                    instances[io_instance]["metadata"]["in2glb_0"]["write_data_stride"] = [channel_per_lane, channel_per_lane, 1 - channel_per_lane * (out_img_size * out_img_size - 1)]

        # -----Overwrite the JSON-----
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

        # -----Update design_meta_halide.json with correct input and output shapes-----
        design_meta_path = os.path.join(bin_path, "design_meta_halide.json")
        with open(design_meta_path, "r") as f:
            design_meta = json.load(f)
        assert len(design_meta["IOs"]["inputs"]) == 1, "Expected only one input"
        assert len(design_meta["IOs"]["outputs"]) == 1, "Expected only one output"
        design_meta["IOs"]["inputs"][0]["shape"] = [n_ic, img_size, img_size]
        design_meta["IOs"]["outputs"][0]["shape"] = [n_ic, (img_size - ksize) // stride + 1, (img_size - ksize) // stride + 1]

        with open(design_meta_path, "w") as f:
            json.dump(design_meta, f, indent=2)

    def hack_for_rope_fp_rv(self, json_path, bin_path):
        """
        Replace the Halide-generated design_top.json with a strait-generated
        RoPE graph that routes through input/sin/cos/output buffer MEMs and
        uses fp_mul, fp_sub, fp_add PEs.

        Reads glb_i (== glb_o) from HALIDE_GEN_ARGS to determine unroll factor.
        Shape is derived from head_dim_half, seq_len, n_heads in halide_gen_args.
        """
        from strait.coreir_backend.templates.rope_bf16 import emit_rope_bf16_design

        unroll = int(self.halide_gen_args_dict.get("glb_i", 16))
        head_dim_half = int(self.halide_gen_args_dict.get("head_dim_half", 32))
        seq_len = int(self.halide_gen_args_dict.get("seq_len", 512))
        n_heads = int(self.halide_gen_args_dict.get("n_heads", 32))

        print(f"\033[94m[INFO] Generating strait RoPE design: "
              f"unroll={unroll}, (head_dim_half={head_dim_half}, seq_len={seq_len}, n_heads={n_heads})\033[0m")
        emit_rope_bf16_design(unroll, head_dim_half, seq_len, n_heads, bin_path)
        print(f"\033[92m[INFO] Replaced design_top.json at {json_path}\033[0m")

        self._assert_strait_names_match_halide_meta(bin_path)

    def hack_for_add_gelu_pass1_mu_input_fp_rv(self, json_path, bin_path):
        """
        Replace the Halide-generated design_top.json with a strait-generated
        half-add-swish + half-nop graph.

        Upper half of lanes: fp_add(mu_input, input_psum0) -> MEM -> swish -> hw_add_gelu_upper_output.
        Lower half of lanes: fp_max(mu_input, -inf) -> hw_psum1_lower_output (dummy passthrough).

        Unroll = mu_i (total lane count). Also splits design_meta_halide.json so the
        outputs list matches strait's IO names.
        """
        from strait.coreir_backend.templates.elementwise_half_add_swish_half_nop_bf16 import (
            emit_elementwise_half_add_swish_half_nop_bf16_design,
        )

        vec_width = int(self.halide_gen_args_dict["vec_width"])
        vec_height = int(self.halide_gen_args_dict["vec_height"])
        unroll = int(self.halide_gen_args_dict["mu_i"])
        beta = float(self.halide_gen_args_dict.get("swish_beta", 1.702))
        vector_len = vec_width * vec_height

        print(f"\033[94m[INFO] Generating strait half-add-swish/half-nop design: "
              f"unroll={unroll}, vector_len={vector_len} (vec_width={vec_width}, vec_height={vec_height}), beta={beta}\033[0m")
        emit_elementwise_half_add_swish_half_nop_bf16_design(unroll, vector_len, bin_path, beta=beta)
        print(f"\033[92m[INFO] Replaced design_top.json at {json_path}\033[0m")

        # Split design_meta_halide.json outputs: upper (hw_add_gelu_upper_output) + lower (hw_psum1_lower_output),
        # each at half width. Must run before the strait/halide name assertion.
        design_meta_path = os.path.join(bin_path, "design_meta_halide.json")
        with open(design_meta_path, "r") as f:
            design_meta = json.load(f)
        if not any(o.get("name") == "hw_psum1_lower_output_stencil" for o in design_meta["IOs"]["outputs"]):
            design_meta["IOs"]["outputs"].append({
                "bitwidth": 16,
                "datafile": "hw_psum1_lower_output.raw",
                "name": "hw_psum1_lower_output_stencil",
                "shape": [vec_width // 2, vec_height],
            })
        for output in design_meta["IOs"]["outputs"]:
            output["shape"] = [vec_width // 2, vec_height]
        with open(design_meta_path, "w") as f:
            json.dump(design_meta, f, indent=2)

        self._assert_strait_names_match_halide_meta(bin_path)

    def _emit_strait_nop_design(self, json_path, bin_path, tensor_size):
        """
        Shared helper for zircon_nop variants: select unroll per E64 env vars,
        emit the strait nop_bf16 design_top.json, and verify IO names.
        """
        from strait.coreir_backend.templates.nop_bf16 import emit_nop_bf16_design

        myunroll = int(self.halide_gen_args_dict.get("myunroll", 1))
        myunroll_E64 = int(self.halide_gen_args_dict.get("myunroll_E64", 16))
        myunroll_E64_MB = int(self.halide_gen_args_dict.get("myunroll_E64_MB", 32))
        if os.environ.get("E64_MULTI_BANK_MODE_ON", "0") == "1":
            unroll = myunroll_E64_MB
        elif os.environ.get("E64_MODE_ON", "0") == "1":
            unroll = myunroll_E64
        else:
            unroll = myunroll

        print(f"\033[94m[INFO] Generating strait NOP design: unroll={unroll}, tensor_size={tensor_size}\033[0m")
        emit_nop_bf16_design(unroll, tensor_size, bin_path)
        print(f"\033[92m[INFO] Replaced design_top.json at {json_path}\033[0m")

        self._assert_strait_names_match_halide_meta(bin_path)

    def hack_for_zircon_nop_rv(self, json_path, bin_path):
        """3D nop: tensor_size = n_oc * out_img * out_img."""
        out_img = int(self.halide_gen_args_dict.get("out_img", 14))
        n_oc = int(self.halide_gen_args_dict.get("n_oc", 256))
        self._emit_strait_nop_design(json_path, bin_path, n_oc * out_img * out_img)

    def hack_for_zircon_2d_nop_rv(self, json_path, bin_path):
        """2D nop: tensor_size = hidden_dim * seq_len."""
        hidden_dim = int(self.halide_gen_args_dict.get("hidden_dim", 128))
        seq_len = int(self.halide_gen_args_dict.get("seq_len", 64))
        self._emit_strait_nop_design(json_path, bin_path, hidden_dim * seq_len)

    def _emit_strait_psum_reduction_design(self, json_path, bin_path, tensor_size):
        """
        Shared helper for zircon_psum_reduction_fp variants: select unroll per E64 env vars,
        emit the strait elementwise_add_bf16 design_top.json in mu_plus_glb mode, and verify IO names.
        """
        from strait.coreir_backend.templates.elementwise_add_bf16 import emit_elementwise_add_bf16_design

        myunroll = int(self.halide_gen_args_dict.get("myunroll", 1))
        myunroll_E64 = int(self.halide_gen_args_dict.get("myunroll_E64", 16))
        myunroll_E64_MB = int(self.halide_gen_args_dict.get("myunroll_E64_MB", 32))
        if os.environ.get("E64_MULTI_BANK_MODE_ON", "0") == "1":
            unroll = myunroll_E64_MB
        elif os.environ.get("E64_MODE_ON", "0") == "1":
            unroll = myunroll_E64
        else:
            unroll = myunroll

        print(f"\033[94m[INFO] Generating strait PSUM reduction design: unroll={unroll}, tensor_size={tensor_size}\033[0m")
        emit_elementwise_add_bf16_design(unroll, tensor_size, bin_path, mode="mu_plus_glb")
        print(f"\033[92m[INFO] Replaced design_top.json at {json_path}\033[0m")

        self._assert_strait_names_match_halide_meta(bin_path)

    def hack_for_zircon_psum_reduction_fp_rv(self, json_path, bin_path):
        """3D psum reduction: tensor_size = n_oc * out_img * out_img."""
        out_img = int(self.halide_gen_args_dict.get("out_img", 56))
        n_oc = int(self.halide_gen_args_dict.get("n_oc", 64))
        self._emit_strait_psum_reduction_design(json_path, bin_path, n_oc * out_img * out_img)

    def hack_for_zircon_2d_psum_reduction_fp_rv(self, json_path, bin_path):
        """2D psum reduction: tensor_size = hidden_dim * seq_len."""
        hidden_dim = int(self.halide_gen_args_dict.get("hidden_dim", 128))
        seq_len = int(self.halide_gen_args_dict.get("seq_len", 64))
        self._emit_strait_psum_reduction_design(json_path, bin_path, hidden_dim * seq_len)

    def hack_for_zircon_dequant_fp_rv(self, json_path, bin_path):
        """
        Dequantize (MU input * scalar constant): emit strait elementwise_mul_bf16 in mu_x_const mode.

        tensor_size = n_oc * out_img * out_img. The scalar dequant constant comes from the
        DEQUANT_SCALE env var (matches the Halide generator default 0.00006341934204101562).
        """
        from strait.coreir_backend.templates.elementwise_mul_bf16 import emit_elementwise_mul_bf16_design

        myunroll = int(self.halide_gen_args_dict.get("myunroll", 1))
        myunroll_E64 = int(self.halide_gen_args_dict.get("myunroll_E64", 16))
        myunroll_E64_MB = int(self.halide_gen_args_dict.get("myunroll_E64_MB", 32))
        if os.environ.get("E64_MULTI_BANK_MODE_ON", "0") == "1":
            unroll = myunroll_E64_MB
        elif os.environ.get("E64_MODE_ON", "0") == "1":
            unroll = myunroll_E64
        else:
            unroll = myunroll

        out_img = int(self.halide_gen_args_dict.get("out_img", 56))
        n_oc = int(self.halide_gen_args_dict.get("n_oc", 64))
        tensor_size = n_oc * out_img * out_img
        dequant_scale = float(os.environ.get("DEQUANT_SCALE", 0.00006341934204101562))

        print(f"\033[94m[INFO] Generating strait dequant design: unroll={unroll}, tensor_size={tensor_size} (out_img={out_img}, n_oc={n_oc}), dequant_scale={dequant_scale}\033[0m")
        emit_elementwise_mul_bf16_design(unroll, tensor_size, bin_path, mode="mu_x_const", mul_const_val_bf16=dequant_scale)
        print(f"\033[92m[INFO] Replaced design_top.json at {json_path}\033[0m")

        self._assert_strait_names_match_halide_meta(bin_path)

    def hack_for_zircon_scale_add_fp_rv(self, json_path, bin_path):
        """
        Scale-then-add ((MU_input * attn_scale) + GLB_attn_mask_input): emit strait
        elementwise_mul_add_bf16 in mu_x_const_plus_vector mode.

        tensor_size = seq_len * seq_len. The scalar attn_scale comes from the ATTN_SCALE env var
        (set at runtime by map.py when ATTENTION_SCALING=1; Halide generator default is 0.5).
        """
        from strait.coreir_backend.templates.elementwise_mul_add_bf16 import emit_elementwise_mul_add_bf16_design

        myunroll = int(self.halide_gen_args_dict.get("myunroll", 4))
        myunroll_E64 = int(self.halide_gen_args_dict.get("myunroll_E64", 16))
        myunroll_E64_MB = int(self.halide_gen_args_dict.get("myunroll_E64_MB", 16))
        if os.environ.get("E64_MULTI_BANK_MODE_ON", "0") == "1":
            unroll = myunroll_E64_MB
        elif os.environ.get("E64_MODE_ON", "0") == "1":
            unroll = myunroll_E64
        else:
            unroll = myunroll

        seq_len = int(self.halide_gen_args_dict.get("seq_len", 64))
        tensor_size = seq_len * seq_len
        attn_scale = float(os.environ.get("ATTN_SCALE", 0.5))

        print(f"\033[94m[INFO] Generating strait scale_add design: unroll={unroll}, tensor_size={tensor_size} (seq_len={seq_len}), attn_scale={attn_scale}\033[0m")
        emit_elementwise_mul_add_bf16_design(unroll, tensor_size, bin_path, mode="mu_x_const_plus_vector", mul_const_val_bf16=attn_scale)
        print(f"\033[92m[INFO] Replaced design_top.json at {json_path}\033[0m")

        self._assert_strait_names_match_halide_meta(bin_path)

    def _emit_strait_mul_add_relu_design(self, json_path, bin_path, tensor_size, mode, mul_const_val_bf16):
        """
        Shared helper for mul-add-ReLU variants (zircon_residual_relu_fp, zircon_deq_ResReLU_fp,
        zircon_dequantize_relu_fp): select unroll per E64 env vars, emit the strait
        elementwise_mul_add_relu_bf16 design_top.json, and verify IO names.
        """
        from strait.coreir_backend.templates.elementwise_mul_add_relu_bf16 import emit_elementwise_mul_add_relu_bf16_design

        myunroll = int(self.halide_gen_args_dict.get("myunroll", 1))
        myunroll_E64 = int(self.halide_gen_args_dict.get("myunroll_E64", 16))
        myunroll_E64_MB = int(self.halide_gen_args_dict.get("myunroll_E64_MB", 32))
        if os.environ.get("E64_MULTI_BANK_MODE_ON", "0") == "1":
            unroll = myunroll_E64_MB
        elif os.environ.get("E64_MODE_ON", "0") == "1":
            unroll = myunroll_E64
        else:
            unroll = myunroll

        print(f"\033[94m[INFO] Generating strait mul_add_relu design: mode={mode}, unroll={unroll}, tensor_size={tensor_size}, mul_const={mul_const_val_bf16}\033[0m")
        emit_elementwise_mul_add_relu_bf16_design(unroll, tensor_size, bin_path, mode=mode, mul_const_val_bf16=mul_const_val_bf16)
        print(f"\033[92m[INFO] Replaced design_top.json at {json_path}\033[0m")

        self._assert_strait_names_match_halide_meta(bin_path)

    def hack_for_zircon_res_deq_ReLU_quant_fp_rv(self, json_path, bin_path):
        """
        Residual-add + ReLU + fused scale + e8m0_quant + bit8_pack
        (zircon_res_deq_ReLU_quant_fp): emit strait elementwise_add_mul_relu_mul_quant_bf16.

        Per input lane: Halide's optimizer fuses the two muls
        `max((MU+GLB)*dequant_scale, 0) * quant_scale == (dequant_scale*quant_scale) * max(MU+GLB, 0)`,
        so the strait template uses a single post-ReLU mul by `scale = dequant_scale*quant_scale`
        to match the gold flow's 4-PE-per-lane graph. Output lanes = unroll / 2.
        """
        from strait.coreir_backend.templates.elementwise_add_relu_mul_quant_bf16 import emit_elementwise_add_mul_relu_mul_quant_bf16_design

        myunroll = int(self.halide_gen_args_dict.get("myunroll", 1))
        myunroll_E64 = int(self.halide_gen_args_dict.get("myunroll_E64", 16))
        myunroll_E64_MB = int(self.halide_gen_args_dict.get("myunroll_E64_MB", 32))
        if os.environ.get("E64_MULTI_BANK_MODE_ON", "0") == "1":
            unroll = myunroll_E64_MB
        elif os.environ.get("E64_MODE_ON", "0") == "1":
            unroll = myunroll_E64
        else:
            unroll = myunroll

        out_img = int(self.halide_gen_args_dict.get("out_img", 14))
        n_oc = int(self.halide_gen_args_dict.get("n_oc", 64))
        tensor_size = n_oc * out_img * out_img
        dequant_scale = float(os.environ.get("DEQUANT_SCALE", 0.5))
        quant_scale = float(os.environ.get("QUANT_SCALE", 0.5))
        scale = dequant_scale * quant_scale

        print(f"\033[94m[INFO] Generating strait res_deq_ReLU_quant design: unroll={unroll}, tensor_size={tensor_size} (out_img={out_img}, n_oc={n_oc}), scale={scale} (dequant={dequant_scale}, quant={quant_scale})\033[0m")
        emit_elementwise_add_mul_relu_mul_quant_bf16_design(unroll, tensor_size, bin_path, scale=scale)
        print(f"\033[92m[INFO] Replaced design_top.json at {json_path}\033[0m")

        self._assert_strait_names_match_halide_meta(bin_path)

    def hack_for_zircon_deq_ResReLU_quant_fp_rv(self, json_path, bin_path):
        """
        Dequantize-mul + residual + ReLU + quantize-mul + e8m0_quant + bit8_pack
        (zircon_deq_ResReLU_quant_fp): emit strait elementwise_mul_add_relu_mul_quant_bf16.

        Per input lane: max(MU * dequant_scale + GLB, 0) * quant_scale -> e8m0_quant(..., 127).
        Output lanes = unroll / 2 (bit8_pack combines two quantized channels).
        """
        from strait.coreir_backend.templates.elementwise_mul_add_relu_mul_quant_bf16 import emit_elementwise_mul_add_relu_mul_quant_bf16_design

        myunroll = int(self.halide_gen_args_dict.get("myunroll", 1))
        myunroll_E64 = int(self.halide_gen_args_dict.get("myunroll_E64", 16))
        myunroll_E64_MB = int(self.halide_gen_args_dict.get("myunroll_E64_MB", 32))
        if os.environ.get("E64_MULTI_BANK_MODE_ON", "0") == "1":
            unroll = myunroll_E64_MB
        elif os.environ.get("E64_MODE_ON", "0") == "1":
            unroll = myunroll_E64
        else:
            unroll = myunroll

        out_img = int(self.halide_gen_args_dict.get("out_img", 14))
        n_oc = int(self.halide_gen_args_dict.get("n_oc", 64))
        tensor_size = n_oc * out_img * out_img
        dequant_scale = float(os.environ.get("DEQUANT_SCALE", 0.5))
        quant_scale = float(os.environ.get("QUANT_SCALE", 0.5))

        print(f"\033[94m[INFO] Generating strait deq_ResReLU_quant design: unroll={unroll}, tensor_size={tensor_size} (out_img={out_img}, n_oc={n_oc}), dequant_scale={dequant_scale}, quant_scale={quant_scale}\033[0m")
        emit_elementwise_mul_add_relu_mul_quant_bf16_design(
            unroll, tensor_size, bin_path,
            dequant_scale=dequant_scale, quant_scale=quant_scale,
        )
        print(f"\033[92m[INFO] Replaced design_top.json at {json_path}\033[0m")

        self._assert_strait_names_match_halide_meta(bin_path)

    def hack_for_zircon_deq_q_relu_fp_rv(self, json_path, bin_path):
        """
        Dequantize-mul -> ReLU -> e8m0_quant -> bit8_pack (zircon_deq_q_relu_fp): emit strait
        elementwise_mul_relu_quant_pack_bf16 design_top.json.

        Input MU tensor size = n_oc * out_img * out_img (input lanes = `unroll`).
        Output lanes = unroll / 2. The mul constant is dequant_scale * quant_scale, both
        sourced from their respective env vars (Halide generator defaults 0.5 each).
        """
        from strait.coreir_backend.templates.elementwise_mul_relu_quant_pack_bf16 import emit_elementwise_mul_relu_quant_pack_bf16_design

        myunroll = int(self.halide_gen_args_dict.get("myunroll", 1))
        myunroll_E64 = int(self.halide_gen_args_dict.get("myunroll_E64", 16))
        myunroll_E64_MB = int(self.halide_gen_args_dict.get("myunroll_E64_MB", 32))
        if os.environ.get("E64_MULTI_BANK_MODE_ON", "0") == "1":
            unroll = myunroll_E64_MB
        elif os.environ.get("E64_MODE_ON", "0") == "1":
            unroll = myunroll_E64
        else:
            unroll = myunroll

        out_img = int(self.halide_gen_args_dict.get("out_img", 14))
        n_oc = int(self.halide_gen_args_dict.get("n_oc", 64))
        tensor_size = n_oc * out_img * out_img
        dequant_scale = float(os.environ.get("DEQUANT_SCALE", 0.5))
        quant_scale = float(os.environ.get("QUANT_SCALE", 0.5))
        scale = dequant_scale * quant_scale

        print(f"\033[94m[INFO] Generating strait deq_q_relu design: unroll={unroll}, tensor_size={tensor_size} (out_img={out_img}, n_oc={n_oc}), scale={scale} (dequant={dequant_scale}, quant={quant_scale})\033[0m")
        emit_elementwise_mul_relu_quant_pack_bf16_design(unroll, tensor_size, bin_path, scale=scale)
        print(f"\033[92m[INFO] Replaced design_top.json at {json_path}\033[0m")

        self._assert_strait_names_match_halide_meta(bin_path)

    def hack_for_zircon_quant_fp_rv(self, json_path, bin_path):
        """
        Quantize pipeline (GLB_input * dequant_scale -> e8m0_quant -> bit8_pack): emit strait
        elementwise_mul_quant_pack_bf16 design_top.json.

        Input tensor size = n_oc * out_img * out_img (unroll input lanes = `unroll`).
        Output lanes are unroll / 2 (each pack PE combines two quantized inputs).
        The scalar dequant constant comes from the QUANT_SCALE env var (Halide generator default 0.5).
        """
        from strait.coreir_backend.templates.elementwise_mul_quant_pack_bf16 import emit_elementwise_mul_quant_pack_bf16_design

        myunroll = int(self.halide_gen_args_dict.get("myunroll", 2))
        myunroll_E64 = int(self.halide_gen_args_dict.get("myunroll_E64", 16))
        myunroll_E64_MB = int(self.halide_gen_args_dict.get("myunroll_E64_MB", 32))
        if os.environ.get("E64_MULTI_BANK_MODE_ON", "0") == "1":
            unroll = myunroll_E64_MB
        elif os.environ.get("E64_MODE_ON", "0") == "1":
            unroll = myunroll_E64
        else:
            unroll = myunroll

        out_img = int(self.halide_gen_args_dict.get("out_img", 14))
        n_oc = int(self.halide_gen_args_dict.get("n_oc", 64))
        tensor_size = n_oc * out_img * out_img
        dequant_scale = float(os.environ.get("QUANT_SCALE", 0.5))

        print(f"\033[94m[INFO] Generating strait quant design: unroll={unroll}, tensor_size={tensor_size} (out_img={out_img}, n_oc={n_oc}), dequant_scale={dequant_scale}\033[0m")
        emit_elementwise_mul_quant_pack_bf16_design(unroll, tensor_size, bin_path, dequant_scale=dequant_scale)
        print(f"\033[92m[INFO] Replaced design_top.json at {json_path}\033[0m")

        self._assert_strait_names_match_halide_meta(bin_path)

    def hack_for_zircon_dequantize_relu_fp_rv(self, json_path, bin_path):
        """
        Dequantize + ReLU (max(MU_input * scale, 0)): emit strait
        elementwise_mul_add_relu_bf16 in mu_x_const_relu mode (mul + max, 2 PEs per lane).

        tensor_size = n_oc * out_img * out_img. The scale is hardcoded in the Halide generator
        (0.000065326690674f); matches that as the default, with DEQUANT_SCALE env var override.
        """
        out_img = int(self.halide_gen_args_dict.get("out_img", 56))
        n_oc = int(self.halide_gen_args_dict.get("n_oc", 64))
        dequant_scale = float(os.environ.get("DEQUANT_SCALE", 0.000065326690674))
        self._emit_strait_mul_add_relu_design(
            json_path, bin_path,
            tensor_size=n_oc * out_img * out_img,
            mode="mu_x_const_relu",
            mul_const_val_bf16=dequant_scale,
        )

    def hack_for_zircon_residual_relu_fp_rv(self, json_path, bin_path):
        """
        Residual-add + ReLU (max(MU_input + GLB_residual_input, 0)): emit strait
        elementwise_mul_add_relu_bf16 in mu_plus_vector_relu mode (add + max, 2 PEs per lane).

        tensor_size = n_oc * out_img * out_img.
        """
        out_img = int(self.halide_gen_args_dict.get("out_img", 56))
        n_oc = int(self.halide_gen_args_dict.get("n_oc", 64))
        self._emit_strait_mul_add_relu_design(
            json_path, bin_path,
            tensor_size=n_oc * out_img * out_img,
            mode="mu_plus_vector_relu",
            mul_const_val_bf16=0.0,
        )

    def hack_for_zircon_deq_ResReLU_fp_rv(self, json_path, bin_path):
        """
        Dequantize + residual-add + ReLU (max(MU_input * dequant_scale + GLB_residual_input, 0)):
        emit strait elementwise_mul_add_relu_bf16 in mu_x_const_plus_vector_relu mode
        (mul + add + max, 3 PEs per lane).

        tensor_size = n_oc * out_img * out_img. The scalar dequant constant comes from the
        DEQUANT_SCALE env var (Halide generator default is 0.5).
        """
        out_img = int(self.halide_gen_args_dict.get("out_img", 14))
        n_oc = int(self.halide_gen_args_dict.get("n_oc", 256))
        dequant_scale = float(os.environ.get("DEQUANT_SCALE", 0.5))
        self._emit_strait_mul_add_relu_design(
            json_path, bin_path,
            tensor_size=n_oc * out_img * out_img,
            mode="mu_x_const_plus_vector_relu",
            mul_const_val_bf16=dequant_scale,
        )

    def _assert_strait_names_match_halide_meta(self, bin_path):
        """
        Fail if logical IO names in strait's design_top.json diverge from the
        Halide-emitted design_meta_halide.json. Enforces that strait templates
        follow Halide's convention so parse_design_meta.findIO
        can resolve IO instances and the correct .raw files feed the correct
        ports at runtime.
        """
        from strait.coreir_backend.coreir_backend import _io_logical_names_from_design_top

        strait_in, strait_out = _io_logical_names_from_design_top(os.path.join(bin_path, "design_top.json"))
        with open(os.path.join(bin_path, "design_meta_halide.json")) as f:
            meta = json.load(f)
        halide_in = {e["name"] for e in meta["IOs"].get("inputs", [])} | {e["name"] for e in meta["IOs"].get("mu_inputs", [])}
        halide_out = {e["name"] for e in meta["IOs"].get("outputs", [])}

        if set(strait_in) != halide_in or set(strait_out) != halide_out:
            details = f"inputs strait={sorted(set(strait_in))} halide={sorted(halide_in)}; outputs strait={sorted(set(strait_out))} halide={sorted(halide_out)}"
            raise AssertionError(f"\033[91m[ERROR] strait/halide IO name mismatch: {details}. Align strait template's io16[in]_<name>_clkwrk_... with Halide Func names.\033[0m")
        print(f"\033[92m[INFO] strait/halide IO names agree (inputs={sorted(set(strait_in))}, outputs={sorted(set(strait_out))})\033[0m")


class GlobalDesignHacker:
    """
    A class to handle design JSON modifications (aka 'hacks') for all apps.
    """

    def __init__(self):
        # Provide pond template for path balancing
        self.pond_tpl = {
            "genref": "cgralib.Pond",
            "genargs": {
                "ID": ["String", ""],
                "has_stencil_valid": ["Bool", True],
                "num_inputs": ["Int", 2],
                "num_outputs": ["Int", 2],
                "width": ["Int", 16],
            },
            "modargs": {"config": ["Json", {}], "mode": ["String", "pond"]},
            "metadata": {"config": {}, "mode": "pond"},
        }
        # PE template for dummy_max_nop insertion
        self.pe_tpl = {
            "modref": "global.PE"
        }
        # Dummy max NOP instruction constant
        self.DUMMY_MAX_NOP_INSTR = "84'h0010005fefe0800400092"
        # No filtering by apps
        pass

    def remove_stencil_mem_rv(self, json_path):
        """
        Remove "port_controller" instances and "io1_*_write_valid" instances
        (and all their connections) from the design JSON.
        Should only be used for dense RV apps by checking DENSE_READY_VALID
        """
        # Load the JSON file
        with open(json_path, "r") as f:
            design = json.load(f)

        # For each module, remove the targeted instances and their connections
        modules = design["namespaces"]["global"]["modules"]
        for mod_name, mod_def in modules.items():
            # Skip modules that do not have 'instances' or 'connections'
            if "instances" not in mod_def or "connections" not in mod_def:
                continue

            # Prepare to remove any instance whose name contains:
            #  - "port_controller"
            #  - or matches "io1_.*_write_valid"
            instances_dict = mod_def["instances"]
            to_remove = []

            # Collect all instance names we want to remove
            for inst_name in instances_dict.keys():
                if "port_controller" in inst_name:
                    to_remove.append(inst_name)
                # Matches "io1_<anything>_write_valid"
                elif re.search(r"io1_.*_write_valid", inst_name):
                    to_remove.append(inst_name)

            # Remove them from the instance dictionary
            for inst_name in to_remove:
                del instances_dict[inst_name]

            # Also remove connections referencing any of those instance names
            new_connections = []
            for conn in mod_def["connections"]:
                lhs, rhs = conn
                # If the left or right side references a removed instance, skip
                if any(inst_name in lhs or inst_name in rhs for inst_name in to_remove):
                    continue
                new_connections.append(conn)

            mod_def["connections"] = new_connections

        # Overwrite the JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

    def add_mu_prefix_to_io(self, json_path):
        """
        Detect any instance name containing 'io16in' AND '_mu_' in the same name;
        rename that instance with a 'MU_' prefix, and also fix up all references
        in that module's connections.
        """
        # Load the JSON
        with open(json_path, "r") as f:
            design = json.load(f)

        # Access the global modules
        modules = design["namespaces"]["global"]["modules"]

        # For each module, find instances to rename
        for mod_name, mod_def in modules.items():
            if "instances" not in mod_def or "connections" not in mod_def:
                continue  # Skip modules without instances or connections

            instances_dict = mod_def["instances"]
            connections_list = mod_def["connections"]

            # Build a rename map: old_name -> new_name
            rename_map = {}
            for inst_name in list(instances_dict.keys()):
                # Condition: "io16in" in name AND "_mu_" in name
                if "io16in" in inst_name and "_mu_" in inst_name:
                    print(f"Renaming instance: {inst_name} with prefix 'MU_'")
                    new_name = "MU_" + inst_name
                    rename_map[inst_name] = new_name

            # Apply renaming to the instances themselves
            for old_name, new_name in rename_map.items():
                instances_dict[new_name] = instances_dict.pop(old_name)

            # Update connections referencing these old instance names
            new_connections = []
            for conn in connections_list:
                lhs, rhs = conn
                for old_name, new_name in rename_map.items():
                    if old_name in lhs:
                        lhs = lhs.replace(old_name, new_name)
                    if old_name in rhs:
                        rhs = rhs.replace(old_name, new_name)
                new_connections.append([lhs, rhs])

            mod_def["connections"] = new_connections

        # Overwrite the JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

    def hack_for_pond_path_balancing(self, json_path, bin_path):
        with open(json_path, "r") as f:
            design = json.load(f)

        modules = design["namespaces"]["global"]["modules"]

        pes_balanced = 0
        for mod_name, mod_def in modules.items():
            if "instances" not in mod_def or "connections" not in mod_def:
                continue  # Skip modules without instances or connections

            instances = mod_def["instances"]

            # For all path_balancing PEs, create a path balancing pond instance and modify the corresponding connections
            path_balancing_json = f"{bin_path}/path_balancing.json"
            assert os.path.exists(path_balancing_json), f"Expected path_balancing.json at {path_balancing_json}"
            with open(path_balancing_json, "r") as f:
                path_balancing_info = json.load(f)

            balance_lengths = path_balancing_info["balance_lengths"]
            name_to_id = path_balancing_info["name_to_id"]
            pe_to_pond_dict = path_balancing_info["pe_to_pond"]
            num_balance_pes = len(balance_lengths)

            connections = mod_def["connections"]
            # connections_iter = copy.deepcopy(connections)

            for pe in balance_lengths:
                pe_to_pond_bool = pe_to_pond_dict[pe][0]  # True if PE output to pond, False if pond to PE input
                pe_to_pond_port = pe_to_pond_dict[pe][1] # PE input port (data0, data1, or data2) if pond to PE input

                found_matching_edge = False
                for edge in connections:
                    left, right = edge[0], edge[1]
                    left_instance_name = left.split(".")[0]
                    left_port = left.split(".")[1] if "." in left else ""
                    right_instance_name = right.split(".")[0]
                    right_port = right.split(".")[1] if "." in right else ""

                    # Search for PE output
                    if pe_to_pond_bool:
                        left_is_path_balance_pe_output = left_port == "O0" and left_instance_name in name_to_id and (name_to_id[left_instance_name] == pe)
                        right_is_path_balance_pe_output = right_port == "O0" and right_instance_name in name_to_id and (name_to_id[right_instance_name] == pe)

                        if left_is_path_balance_pe_output or right_is_path_balance_pe_output:
                            found_matching_edge = True
                            pes_balanced += 1
                            if left_is_path_balance_pe_output:
                                pond_name = f"{pe}_path_balance_pond"
                            else:
                                pond_name = f"{pe}_path_balance_pond"
                            pond_instance = copy.deepcopy(self.pond_tpl)
                            pond_instance["genargs"]["ID"][1] = pond_name
                            instances[pond_name] = pond_instance

                            # Found the PE output, insert pond here
                            connections.remove(edge)
                            if left_is_path_balance_pe_output:
                                connections.append([left, f"{pond_name}.data_in_pond_0"])
                                connections.append([f"{pond_name}.data_out_pond_1", right])
                            else:
                                connections.append([right, f"{pond_name}.data_in_pond_0"])
                                connections.append([f"{pond_name}.data_out_pond_1", left])

                            print(f"\033[93mINFO: Inserted pond '{pond_name}' between '{left}' and '{right}' for path balancing. Connection is PE -> Pond. \033[0m")

                    # Search for one of the PE inputs
                    else:
                        left_is_path_balance_pe_input = (left_port == pe_to_pond_port) and left_instance_name in name_to_id and (name_to_id[left_instance_name] == pe)
                        right_is_path_balance_pe_input = (right_port == pe_to_pond_port) and right_instance_name in name_to_id and (name_to_id[right_instance_name] == pe)

                        if left_is_path_balance_pe_input or right_is_path_balance_pe_input:
                            found_matching_edge = True
                            pes_balanced += 1
                            if left_is_path_balance_pe_input:
                                pond_name = f"{pe}_path_balance_pond"
                            else:
                                pond_name = f"{pe}_path_balance_pond"
                            pond_instance = copy.deepcopy(self.pond_tpl)
                            pond_instance["genargs"]["ID"][1] = pond_name
                            instances[pond_name] = pond_instance

                            # Found the PE input, insert pond here (pond drives PE input)
                            connections.remove(edge)
                            if left_is_path_balance_pe_input:
                                connections.append([f"{pond_name}.data_out_pond_0", left])
                                connections.append([right, f"{pond_name}.data_in_pond_0"])
                            else:
                                connections.append([f"{pond_name}.data_out_pond_0", right])
                                connections.append([left, f"{pond_name}.data_in_pond_0"])
                            print(f"\033[93mINFO: Inserted pond '{pond_name}' between '{left}' and '{right}' for path balancing. Connection is Pond -> PE. \033[0m")

                    if found_matching_edge:
                        break

        assert pes_balanced == num_balance_pes, f"Expected to balance {num_balance_pes} PEs, but only balanced {pes_balanced} PEs."

        # Overwrite the JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

    def insert_dummy_max_nop_between_fp_add_subexp(self, json_path):
        """
        Insert dummy_max_nop PEs between all fp_add -> fp_subexp connections
        across all modules in the design. This helps with path balancing.
        """
        with open(json_path, "r") as f:
            design = json.load(f)

        modules = design["namespaces"]["global"]["modules"]

        for mod_name, mod_def in modules.items():
            if "instances" not in mod_def or "connections" not in mod_def:
                continue  # Skip modules without instances or connections

            instances = mod_def["instances"]
            connections = mod_def["connections"]

            # Helper function to add connection only if it doesn't exist
            def add_conn_once(src, dst):
                pair = [src, dst]
                if pair not in connections:
                    connections.append(pair)

            # Helper function to remove connection
            # Connections can be in [src, dst] or [dst, src] format
            def remove_conn(src, dst):
                pair1 = [src, dst]
                pair2 = [dst, src]
                if pair1 in connections:
                    connections.remove(pair1)
                elif pair2 in connections:
                    connections.remove(pair2)

            # Find all fp_add -> fp_subexp connections
            fp_add_to_subexp_connections = []
            for conn in connections:
                left, right = conn[0], conn[1]
                if ("float_DW_fp_add" in left and left.endswith(".O0") and
                    "fp_subexp" in right and right.endswith(".data1")):
                    fp_add_to_subexp_connections.append((left, right))
                elif ("float_DW_fp_add" in right and right.endswith(".O0") and
                      "fp_subexp" in left and left.endswith(".data1")):
                    fp_add_to_subexp_connections.append((right, left))

            if not fp_add_to_subexp_connections:
                print(f"[WARNING]: No direct fp_add -> fp_subexp connections found in module '{mod_name}'; skipping dummy_max_nop insertion.")
                continue

            pe_counter = 0
            for src, dst in fp_add_to_subexp_connections:
                src_base = src.split(".O0")[0]
                dummy_pe_name = f"{src_base}_dummy_max_nop_pe_{pe_counter}"
                dummy_const_name = f"{src_base}_dummy_max_nop_const_{pe_counter}"

                if dummy_const_name not in instances:
                    instances[dummy_const_name] = {
                        "genref": "coreir.const",
                        "genargs": {"width": ["Int", 84]},
                        "modargs": {"value": [["BitVector", 84], self.DUMMY_MAX_NOP_INSTR]},
                    }

                if dummy_pe_name not in instances:
                    instances[dummy_pe_name] = copy.deepcopy(self.pe_tpl)

                remove_conn(src, dst)
                add_conn_once(src, f"{dummy_pe_name}.data0")
                add_conn_once(f"{dummy_pe_name}.inst", f"{dummy_const_name}.out")
                add_conn_once(f"{dummy_pe_name}.O0", dst)

                print(f"[INFO] Inserted dummy_max_nop PE '{dummy_pe_name}' between '{src}' and '{dst}' in module '{mod_name}'")
                pe_counter += 1

        # Overwrite the JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

    def sort_IO_instances(self, json_path):
        """
        Sort IO instances (global.IO) by the stencil index.
        Inputs use "stencil_<idx>_read" while outputs use "stencil_<idx>_write".
        """

        with open(json_path, "r") as f:
            design = json.load(f)

        namespaces = design.get("namespaces", {})

        def is_io_instance(inst_def):
            if not isinstance(inst_def, dict):
                return False
            if inst_def.get("modref") != "global.IO":
                return False
            mode_arg = inst_def.get("modargs", {}).get("mode")
            if not isinstance(mode_arg, list) or len(mode_arg) < 2:
                return False
            return mode_arg[-1] in ("in", "out")

        def extract_idx(inst_name, mode):
            if mode == "in":
                match = re.search(r"stencil_(\d+)_read", inst_name)
            else:
                match = re.search(r"stencil_(\d+)_write", inst_name)
            if not match:
                match = re.search(r"stencil_(\d+)", inst_name)
            return int(match.group(1)) if match else 0

        def flush_pending(pending, out_dict):
            if not pending:
                return
            pending.sort(
                key=lambda entry: (
                    entry["mode_direction"],
                    entry["idx"],
                    entry["name"],
                )
            )
            for entry in pending:
                out_dict[entry["name"]] = entry["inst"]
            pending.clear()

        for namespace in namespaces.values():
            modules = namespace.get("modules", {})
            for module in modules.values():
                instances = module.get("instances")
                if not isinstance(instances, dict) or not instances:
                    continue

                new_instances = OrderedDict()
                pending_ios = []

                for inst_name, inst_def in instances.items():
                    if is_io_instance(inst_def):
                        mode = inst_def["modargs"]["mode"][-1]
                        idx = extract_idx(inst_name, mode)
                        pending_ios.append(
                            {
                                "name": inst_name,
                                "inst": inst_def,
                                "idx": idx,
                                "mode_direction": 0 if mode == "in" else 1,
                            }
                        )
                    else:
                        flush_pending(pending_ios, new_instances)
                        new_instances[inst_name] = inst_def

                flush_pending(pending_ios, new_instances)

                module["instances"] = new_instances

        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

    def remove_rom_zeros(self, json_path):
        '''
        Remove redundant zeros in ROM configuration to avoid bitstream size overflow.
        '''
        with open(json_path, "r") as f:
            design = json.load(f)

        modules = design["namespaces"]["global"]["modules"]

        for mod_name, mod_def in modules.items():
            if "instances" not in mod_def:
                continue  # Skip modules without instances

            instances_dict = mod_def["instances"]

            # For each instance, check if it's a ROM Mem and remove zeros from init
            for inst_name, inst_def in instances_dict.items():
                # Check if genref is cgralib.Mem
                if inst_def.get("genref") != "cgralib.Mem":
                    continue

                # Check if metadata exists and is_rom is true
                if "metadata" not in inst_def:
                    continue

                metadata = inst_def["metadata"]
                if not metadata.get("is_rom", False):
                    continue

                # Check if init exists and is a list
                if "init" not in metadata or not isinstance(metadata["init"], list):
                    continue

                # Remove all zeros from the init list
                original_length = len(metadata["init"])
                metadata["init"] = [x for x in metadata["init"] if x != 0]
                removed_count = original_length - len(metadata["init"])

                if removed_count > 0:
                    print(f"Removed {removed_count} zeros from ROM init in instance '{inst_name}' in module '{mod_name}'")

        # Overwrite the JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))


class GlobalDesignMetaHakcer:
    """
    A class to handle design_meta_halide JSON modifications (aka 'hacks') for all apps.
    """

    def __init__(self):
        pass

    def hack_mu_inputs(self, json_path):
        """
        For each element in "inputs", if the name contains "mu_", then move that entry
        to a new "mu_inputs" list under "IOs".
        """
        with open(json_path, "r") as f:
            config = json.load(f)

        inputs_list = config["IOs"]["inputs"]

        # Collect mu_ inputs
        mu_inputs_list = []
        # If the file already have "mu_inputs", keep them
        if "mu_inputs" not in config["IOs"]:
            config["IOs"]["mu_inputs"] = []
        else:
            mu_inputs_list = config["IOs"]["mu_inputs"]

        # We build a new inputs list that excludes the mu_ items
        new_inputs_list = []
        for inp in inputs_list:
            if "mu_" in inp["name"]:
                print(f"Moving 'mu_' input to 'mu_inputs' list...")
                mu_inputs_list.append(inp)
            else:
                new_inputs_list.append(inp)

        # Update the original lists
        config["IOs"]["inputs"] = new_inputs_list
        config["IOs"]["mu_inputs"] = mu_inputs_list

        # Overwrite the JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(config))


def main():
    parser = argparse.ArgumentParser(
        description="Optionally hack a JSON file (design_top.json or design_meta_halide.json) for certain tests/apps."
    )
    parser.add_argument("--testname", type=str, required=True, help="Name of the test")
    parser.add_argument(
        "--design_top_json",
        type=str,
        help="design top JSON file to hack in-place (will be overwritten)",
        required=True,
    )
    parser.add_argument(
        "--design_meta_halide_json",
        type=str,
        help="design meta halide JSON file to hack in-place (will be overwritten)",
        required=True,
    )
    parser.add_argument(
        "--bin_dir",
        type=str,
        help="app bin directory for storing other config files",
        required=True,
    )
    args = parser.parse_args()

    # We can expand this list if more apps need hacking
    apps_needing_hacks = APPS_NEEDING_HACKS

    # ----------------- Perform selected hacks -----------------
    selected_design_top_hacker = SelectedDesignHacker(hack_apps=apps_needing_hacks)

    # Perform hack if testname is in that list, otherwise skip
    selected_design_top_hacker.hack_design_if_needed(
        args.testname, args.design_top_json, args.bin_dir
    )

    # ----------------- Perform global hacks -----------------
    ## Hack design_top.json
    # Perform global hack of design_top.json to remove stencil mem in RV mode
    global_design_top_hacker = GlobalDesignHacker()
    use_rv = os.getenv("DENSE_READY_VALID", "0") != "0"
    if use_rv:
        print("Removing stencil memory instances for RV mode...")
        global_design_top_hacker.remove_stencil_mem_rv(args.design_top_json)
        # Perform global hack of design_top.json to remove redundant zeros in ROM configuration
        # TODO: This should work for static apps as well, but camera_pipeline_2x2 seems to have pixel mismatches. Trigger this only in RV for now.
        # TODO: Need to investigate when implementing RV camera pipeline
        global_design_top_hacker.remove_rom_zeros(args.design_top_json)

    # Perform global hack of design_top.json to add MU prefix for MU IOs
    global_design_top_hacker.add_mu_prefix_to_io(args.design_top_json)
    # Perform global hack of design_top.json to sort IO instances
    global_design_top_hacker.sort_IO_instances(args.design_top_json)
    # Perform global hack of design_top.json to insert dummy_max_nop PEs between fp_add->fp_subexp connections
    global_design_top_hacker.insert_dummy_max_nop_between_fp_add_subexp(args.design_top_json)

    # Perform global hack of design_top.json to insert ponds for path balancing
    # TODO: This should NOT be set in application_parameters. It should be set by the flow on the 2nd pass
    use_pond_path_balancing = os.getenv("POND_PATH_BALANCING", "0") != "0"
    if use_pond_path_balancing:
        global_design_top_hacker.hack_for_pond_path_balancing(args.design_top_json, args.bin_dir)

    ## Hack design_meta_halide.json
    global_design_meta_hacker = GlobalDesignMetaHakcer()
    global_design_meta_hacker.hack_mu_inputs(args.design_meta_halide_json)


if __name__ == "__main__":
    main()
    # global_design_top_hacker = GlobalDesignHacker()
    # design_top = "/aha/Halide-to-Hardware/apps/hardware_benchmarks/apps/zircon_deq_ResReLU_quant_fp/bin_saved/design_top.json"
    # bin_dir = "/aha/Halide-to-Hardware/apps/hardware_benchmarks/apps/zircon_deq_ResReLU_quant_fp/bin_saved"
    # global_design_top_hacker.hack_for_pond_path_balancing(design_top, bin_dir)

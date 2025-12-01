#!/usr/bin/env python3

import os, re, argparse, json, copy
from collections import OrderedDict, defaultdict, Counter
from pretty_format_json import pretty_format_json

APPS_NEEDING_HACKS = [
    "scalar_reduction",
    "scalar_reduction_fp",
    "scalar_max_fp",
    "stable_softmax_pass1_fp",
    "stable_softmax_pass2_fp",
    "stable_softmax_pass3_fp",
    "scalar_avg_fp",
    "layer_norm_pass1_fp",
    "layer_norm_pass2_fp",
    "gelu_pass1_mu_input_fp",
    "gelu_pass2_fp",
    "silu_pass2_fp",
    "swiglu_pass2_fp",
    "vector_reduction_fp",
    "mem_transpose_test",
    "mem_slice_test",
    "mem_filter_test",
    "avgpool_layer_fp",
    "mat_vec_mul_fp",
    "get_e8m0_scale_tree_mu_input",
    "get_e8m0_scale_tree_gb_input",
    "get_e8m0_scale_accum_gb_input",
    "apply_e8m0_scale_single_IO",
    "apply_e8m0_scale_multi_IOs",
    "get_apply_e8m0_scale_fp",
    "relu_layer_multiout_fp",
    "maxpooling_dense_rv_fp",
    "fully_connected_layer_fp",
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
        if testname not in self.hack_apps:
            print(
                f"\033[92mSkipping selected hack for '{testname}', not in hack list: {self.hack_apps}\033[0m"
            )
            return

        print(f"\033[94mApplying hack for '{testname}'...\033[0m")

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




    def hack_for_scalar_reduction_static(self, json_path, bin_path):

        # TODO: Hardcode input pipelining regs for now
        INPUT_PIPELINING_REGS = 3

        with open(json_path, "r") as f:
            design = json.load(f)

        top_module = "scalar_reduction"
        tree_out_pond = "tree_3_stencil$ub_tree_3_stencil_BANK_0_garnet"
        tree_out_pond_clk = "tree_3_stencil$ub_tree_3_stencil_BANK_0_clk_en_const"

        # Const PE instance to remove
        pe_to_remove = "op_hcompute_output_cgra_stencil$inner_compute$const_"

        # Locate "scalar_reduction" module
        global_modules = design["namespaces"]["global"]["modules"]
        if top_module not in global_modules:
            print(f"WARNING: Module '{top_module}' not found in design. No hack applied.")
            return
        scalar_reduction = global_modules[top_module]

        # Remove the tile instance and its clk_en_const
        instance_dict = scalar_reduction.get("instances", {})
        if tree_out_pond in instance_dict:
            del instance_dict[tree_out_pond]
        if tree_out_pond_clk in instance_dict:
            del instance_dict[tree_out_pond_clk]

        # Remove references to the tile from connections,
        #    capturing upstream => old_input, downstream => old_output
        old_input = None
        old_output = None
        new_connections = []

        connection_list = scalar_reduction.get("connections", [])
        for conn in connection_list:
            left, right = conn[0], conn[1]

            # Check references to tile or its clk const
            refs_tile_left = (tree_out_pond in left) or (tree_out_pond_clk in left)
            refs_tile_right = (tree_out_pond in right) or (tree_out_pond_clk in right)

            if not (refs_tile_left or refs_tile_right):
                # If no reference to tile, keep it for now
                new_connections.append(conn)
                continue

            # If referencing the tile, skip it but see if it's data_in or data_out
            if tree_out_pond in left:
                if ".data_in_pond_" in left or ".data_in_" in left:
                    old_input = right
            elif tree_out_pond in right:
                if ".data_in_pond_" in right or ".data_in_" in right:
                    old_input = left

            if tree_out_pond in left:
                if ".data_out_pond_" in left or ".data_out_" in left:
                    old_output = right
            elif tree_out_pond in right:
                if ".data_out_pond_" in right or ".data_out_" in right:
                    old_output = left

        # Reconnect old_input -> old_output if both exist
        if old_input and old_output:
            new_connections.append([old_input, old_output])
        else:
            print(
                f"WARNING: Could not find both data_in and data_out references for "
                f"tile '{tree_out_pond}'. No direct reconnection added."
            )

        # Remove the PE instance "op_hcompute_output_cgra_stencil$inner_compute$const_"
        #    from the instance dictionary (if present)
        if pe_to_remove in instance_dict:
            del instance_dict[pe_to_remove]

        # Filter out any connections referencing const PE instance
        final_connections = []
        for conn in new_connections:
            left, right = conn[0], conn[1]
            if pe_to_remove in left or pe_to_remove in right:
                # skip any references to that PE
                continue
            final_connections.append(conn)

        # Update the final connection list
        scalar_reduction["connections"] = final_connections

        # Modify psum pond schedule
        psum_pond_name = "output_cgra_stencil$ub_output_cgra_stencil_BANK_0_garnet"
        if psum_pond_name in instance_dict:
            tile_inst = instance_dict[psum_pond_name]
            metadata = tile_inst.get("metadata", {})
            config = metadata.get("config", {})

            # Remove 'in2regfile_1' and 'regfile2out_1' if present
            if "in2regfile_1" in config:
                del config["in2regfile_1"]
            if "regfile2out_1" in config:
                del config["regfile2out_1"]

            # Adjust cycle_starting_addr for inputs
            assert "in2regfile_0" in config, "Error: 'in2regfile_0' not found in config."
            input_pipelining_regs = INPUT_PIPELINING_REGS
            config["in2regfile_0"]["cycle_starting_addr"] = [input_pipelining_regs]

            # For regfile2out_0, set cycle_starting_addr to in2regfile_0 + 1
            assert "regfile2out_0" in config, "Error: 'regfile2out_0' not found in config."
            config["regfile2out_0"]["cycle_starting_addr"] = [
                config["in2regfile_0"]["cycle_starting_addr"][0] + 1
            ]

            metadata["config"] = config
            tile_inst["metadata"] = metadata

        # Update output IO tile
        io_out_name = "io16_hw_output_stencil_op_hcompute_hw_output_stencil_write_0"
        HALIDE_GEN_ARGS = os.environ.get("HALIDE_GEN_ARGS")
        print(f"HALIDE_GEN_ARGS: {HALIDE_GEN_ARGS}")
        halide_args_dict = dict(item.split("=") for item in HALIDE_GEN_ARGS.split())
        # Convert values to integers where applicable
        halide_args_dict = {k: int(v) for k, v in halide_args_dict.items()}

        # Calculate cycles computing partial sum
        psum_cycles = (
            halide_args_dict["vec_width"]
            * halide_args_dict["vec_height"]
            // pow(2, halide_args_dict["tree_stages"])
        )
        assert io_out_name in instance_dict, f"Error: '{io_out_name}' not found in instance_dict."
        io16_inst = instance_dict[io_out_name]
        io16_meta = io16_inst.get("metadata", {})
        in2glb_0 = io16_meta.get("in2glb_0", {})

        # Output cycle = psum pond's in2regfile_0 cycle_starting_addr + tot_elems / tree_width
        tile_inst = instance_dict[psum_pond_name]
        tile_meta = tile_inst.get("metadata", {})
        tile_conf = tile_meta.get("config", {})

        base_addr = tile_conf["in2regfile_0"]["cycle_starting_addr"][0]
        in2glb_0["cycle_starting_addr"] = [base_addr + psum_cycles]
        # TODO: Set extent to 2 (should set this to 1 since we only have 1 output, but then can't get f2g interrupt)
        in2glb_0["extent"] = [2]

        io16_meta["in2glb_0"] = in2glb_0
        io16_inst["metadata"] = io16_meta

        # Update stencil MEM to be the same as output IO tile
        hw_portctrl_garnet = "op_hcompute_hw_output_stencil_port_controller_garnet"
        if hw_portctrl_garnet in instance_dict and io_out_name in instance_dict:
            hw_inst = instance_dict[hw_portctrl_garnet]
            hw_meta = hw_inst.get("metadata", {})
            hw_conf = hw_meta.get("config", {})
            if "stencil_valid" not in hw_conf:
                hw_conf["stencil_valid"] = {}

            stencil_valid = hw_conf["stencil_valid"]

            # Copy from io16_hw_output_stencil_op_hcompute_hw_output_stencil_write_0 -> in2glb_0
            io16_inst = instance_dict[io_out_name]
            io16_meta = io16_inst.get("metadata", {})
            in2glb_0 = io16_meta.get("in2glb_0", {})

            stencil_valid["cycle_starting_addr"] = in2glb_0["cycle_starting_addr"]
            stencil_valid["cycle_stride"] = in2glb_0["cycle_stride"]
            stencil_valid["extent"] = in2glb_0["extent"]

            hw_conf["stencil_valid"] = stencil_valid
            hw_meta["config"] = hw_conf
            hw_inst["metadata"] = hw_meta

        # Overwrite the JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

    def hack_for_scalar_reduction_fp_static(self, json_path, bin_path):

        # TODO: Hardcode input pipelining regs for now
        INPUT_PIPELINING_REGS = 3

        with open(json_path, "r") as f:
            design = json.load(f)

        top_module = "scalar_reduction_fp"
        tree_out_pond = "tree_3_stencil$ub_tree_3_stencil_BANK_0_garnet"
        tree_out_pond_clk = "tree_3_stencil$ub_tree_3_stencil_BANK_0_clk_en_const"

        # Const PE instance to remove
        pe_to_remove = "op_hcompute_output_cgra_stencil$inner_compute$const_"

        # Locate "scalar_reduction_fp" module
        global_modules = design["namespaces"]["global"]["modules"]
        if top_module not in global_modules:
            print(f"WARNING: Module '{top_module}' not found in design. No hack applied.")
            return
        scalar_reduction_fp = global_modules[top_module]

        # Remove the tile instance and its clk_en_const
        instance_dict = scalar_reduction_fp.get("instances", {})
        if tree_out_pond in instance_dict:
            del instance_dict[tree_out_pond]
        if tree_out_pond_clk in instance_dict:
            del instance_dict[tree_out_pond_clk]

        # Remove references to the tile from connections,
        #    capturing upstream => old_input, downstream => old_output
        old_input = None
        old_output = None
        new_connections = []

        connection_list = scalar_reduction_fp.get("connections", [])
        for conn in connection_list:
            left, right = conn[0], conn[1]

            # Check references to tile or its clk const
            refs_tile_left = (tree_out_pond in left) or (tree_out_pond_clk in left)
            refs_tile_right = (tree_out_pond in right) or (tree_out_pond_clk in right)

            if not (refs_tile_left or refs_tile_right):
                # If no reference to tile, keep it for now
                new_connections.append(conn)
                continue

            # If referencing the tile, skip it but see if it's data_in or data_out
            if tree_out_pond in left:
                if ".data_in_pond_" in left or ".data_in_" in left:
                    old_input = right
            elif tree_out_pond in right:
                if ".data_in_pond_" in right or ".data_in_" in right:
                    old_input = left

            if tree_out_pond in left:
                if ".data_out_pond_" in left or ".data_out_" in left:
                    old_output = right
            elif tree_out_pond in right:
                if ".data_out_pond_" in right or ".data_out_" in right:
                    old_output = left

        # Reconnect old_input -> old_output if both exist
        if old_input and old_output:
            new_connections.append([old_input, old_output])
        else:
            print(
                f"WARNING: Could not find both data_in and data_out references for "
                f"tile '{tree_out_pond}'. No direct reconnection added."
            )

        # Remove the PE instance "op_hcompute_output_cgra_stencil$inner_compute$const_"
        #    from the instance dictionary (if present)
        if pe_to_remove in instance_dict:
            del instance_dict[pe_to_remove]

        # Filter out any connections referencing const PE instance
        final_connections = []
        for conn in new_connections:
            left, right = conn[0], conn[1]
            if pe_to_remove in left or pe_to_remove in right:
                # skip any references to that PE
                continue
            final_connections.append(conn)

        # Update the final connection list
        scalar_reduction_fp["connections"] = final_connections

        # Get Halide generator arguments
        HALIDE_GEN_ARGS = os.environ.get("HALIDE_GEN_ARGS")
        print(f"HALIDE_GEN_ARGS: {HALIDE_GEN_ARGS}")
        halide_args_dict = dict(item.split("=") for item in HALIDE_GEN_ARGS.split())
        # Convert values to integers where applicable
        halide_args_dict = {k: int(v) for k, v in halide_args_dict.items()}

        # Modify psum pond schedule
        psum_pond_name = "output_cgra_stencil$ub_output_cgra_stencil_BANK_0_garnet"
        if psum_pond_name in instance_dict:
            tile_inst = instance_dict[psum_pond_name]
            metadata = tile_inst.get("metadata", {})
            config = metadata.get("config", {})

            # Remove 'in2regfile_1' and 'regfile2out_1' if present
            if "in2regfile_1" in config:
                del config["in2regfile_1"]
            if "regfile2out_1" in config:
                del config["regfile2out_1"]

            # Adjust cycle_starting_addr for inputs
            assert "in2regfile_0" in config, "Error: 'in2regfile_0' not found in config."
            input_pipelining_regs = INPUT_PIPELINING_REGS
            config["in2regfile_0"]["cycle_starting_addr"] = [input_pipelining_regs]

            # For regfile2out_0, set cycle_starting_addr to in2regfile_0 + 1
            assert "regfile2out_0" in config, "Error: 'regfile2out_0' not found in config."
            config["regfile2out_0"]["cycle_starting_addr"] = [
                config["in2regfile_0"]["cycle_starting_addr"][0] + 1
            ]

            # Set write/read data stride to 0 as we always write/read to the same addr
            config["in2regfile_0"]["write_data_stride"] = [0, 0]
            config["regfile2out_0"]["read_data_stride"] = [0, 0]

            metadata["config"] = config
            tile_inst["metadata"] = metadata

        # Update output IO tile
        io_out_name = "io16_hw_output_stencil_op_hcompute_hw_output_stencil_write_0"

        # Calculate cycles computing partial sum
        psum_cycles = (
            halide_args_dict["vec_width"]
            * halide_args_dict["vec_height"]
            // pow(2, halide_args_dict["tree_stages"])
        )
        assert io_out_name in instance_dict, f"Error: '{io_out_name}' not found in instance_dict."
        io16_inst = instance_dict[io_out_name]
        io16_meta = io16_inst.get("metadata", {})
        in2glb_0 = io16_meta.get("in2glb_0", {})

        # Output cycle = psum pond's in2regfile_0 cycle_starting_addr + tot_elems / tree_width
        tile_inst = instance_dict[psum_pond_name]
        tile_meta = tile_inst.get("metadata", {})
        tile_conf = tile_meta.get("config", {})

        base_addr = tile_conf["in2regfile_0"]["cycle_starting_addr"][0]
        in2glb_0["cycle_starting_addr"] = [base_addr + psum_cycles]
        # TODO: Set extent to 2 (should set this to 1 since we only have 1 output, but then can't get f2g interrupt)
        in2glb_0["extent"] = [2]

        io16_meta["in2glb_0"] = in2glb_0
        io16_inst["metadata"] = io16_meta

        # Update stencil MEM to be the same as output IO tile
        hw_portctrl_garnet = "op_hcompute_hw_output_stencil_port_controller_garnet"
        if hw_portctrl_garnet in instance_dict and io_out_name in instance_dict:
            hw_inst = instance_dict[hw_portctrl_garnet]
            hw_meta = hw_inst.get("metadata", {})
            hw_conf = hw_meta.get("config", {})
            if "stencil_valid" not in hw_conf:
                hw_conf["stencil_valid"] = {}

            stencil_valid = hw_conf["stencil_valid"]

            # Copy from io16_hw_output_stencil_op_hcompute_hw_output_stencil_write_0 -> in2glb_0
            io16_inst = instance_dict[io_out_name]
            io16_meta = io16_inst.get("metadata", {})
            in2glb_0 = io16_meta.get("in2glb_0", {})

            stencil_valid["cycle_starting_addr"] = in2glb_0["cycle_starting_addr"]
            stencil_valid["cycle_stride"] = in2glb_0["cycle_stride"]
            stencil_valid["extent"] = in2glb_0["extent"]

            hw_conf["stencil_valid"] = stencil_valid
            hw_meta["config"] = hw_conf
            hw_inst["metadata"] = hw_meta

        # Overwrite the JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

    def hack_for_scalar_max_fp_static(self, json_path, bin_path):

        # TODO: Hardcode input pipelining regs for now
        INPUT_PIPELINING_REGS = 3

        with open(json_path, "r") as f:
            design = json.load(f)

        top_module = "scalar_max_fp"
        tree_out_pond = "tree_3_stencil$ub_tree_3_stencil_BANK_0_garnet"
        tree_out_pond_clk = "tree_3_stencil$ub_tree_3_stencil_BANK_0_clk_en_const"

        # Const PE instance to remove
        pe_to_remove = "op_hcompute_max_output_cgra_inner_stencil$inner_compute$const"

        # Locate "scalar_max_fp" module
        global_modules = design["namespaces"]["global"]["modules"]
        if top_module not in global_modules:
            print(f"WARNING: Module '{top_module}' not found in design. No hack applied.")
            return
        scalar_max_fp = global_modules[top_module]

        # Remove the tile instance and its clk_en_const
        instance_dict = scalar_max_fp.get("instances", {})
        if tree_out_pond in instance_dict:
            del instance_dict[tree_out_pond]
        if tree_out_pond_clk in instance_dict:
            del instance_dict[tree_out_pond_clk]

        # Remove references to the tile from connections,
        #    capturing upstream => old_input, downstream => old_output
        old_input = None
        old_output = None
        new_connections = []

        connection_list = scalar_max_fp.get("connections", [])
        for conn in connection_list:
            left, right = conn[0], conn[1]

            # Check references to tile or its clk const
            refs_tile_left = (tree_out_pond in left) or (tree_out_pond_clk in left)
            refs_tile_right = (tree_out_pond in right) or (tree_out_pond_clk in right)

            if not (refs_tile_left or refs_tile_right):
                # If no reference to tile, keep it for now
                new_connections.append(conn)
                continue

            # If referencing the tile, skip it but see if it's data_in or data_out
            if tree_out_pond in left:
                if ".data_in_pond_" in left or ".data_in_" in left:
                    old_input = right
            elif tree_out_pond in right:
                if ".data_in_pond_" in right or ".data_in_" in right:
                    old_input = left

            if tree_out_pond in left:
                if ".data_out_pond_" in left or ".data_out_" in left:
                    old_output = right
            elif tree_out_pond in right:
                if ".data_out_pond_" in right or ".data_out_" in right:
                    old_output = left

        # Reconnect old_input -> old_output if both exist
        if old_input and old_output:
            new_connections.append([old_input, old_output])
        else:
            print(
                f"WARNING: Could not find both data_in and data_out references for "
                f"tile '{tree_out_pond}'. No direct reconnection added."
            )

        # Remove the PE instance "op_hcompute_output_cgra_stencil$inner_compute$const_"
        #    from the instance dictionary (if present)
        if pe_to_remove in instance_dict:
            del instance_dict[pe_to_remove]

        # Filter out any connections referencing const PE instance
        final_connections = []
        for conn in new_connections:
            left, right = conn[0], conn[1]
            if pe_to_remove in left or pe_to_remove in right:
                # skip any references to that PE
                continue
            final_connections.append(conn)

        # Update the final connection list
        scalar_max_fp["connections"] = final_connections

        # Get Halide generator arguments
        HALIDE_GEN_ARGS = os.environ.get("HALIDE_GEN_ARGS")
        print(f"HALIDE_GEN_ARGS: {HALIDE_GEN_ARGS}")
        halide_args_dict = dict(item.split("=") for item in HALIDE_GEN_ARGS.split())
        # Convert values to integers where applicable
        halide_args_dict = {k: int(v) for k, v in halide_args_dict.items()}

        # Modify psum pond schedule
        psum_pond_name = (
            "max_output_cgra_inner_stencil$ub_max_output_cgra_inner_stencil_BANK_0_garnet"
        )
        if psum_pond_name in instance_dict:
            tile_inst = instance_dict[psum_pond_name]
            metadata = tile_inst.get("metadata", {})
            config = metadata.get("config", {})

            # Remove 'in2regfile_1' and 'regfile2out_1' if present
            if "in2regfile_1" in config:
                del config["in2regfile_1"]
            if "regfile2out_1" in config:
                del config["regfile2out_1"]

            # Adjust cycle_starting_addr for inputs
            assert "in2regfile_0" in config, "Error: 'in2regfile_0' not found in config."
            input_pipelining_regs = INPUT_PIPELINING_REGS
            config["in2regfile_0"]["cycle_starting_addr"] = [input_pipelining_regs]

            # For regfile2out_0, set cycle_starting_addr to in2regfile_0 + 1
            assert "regfile2out_0" in config, "Error: 'regfile2out_0' not found in config."
            config["regfile2out_0"]["cycle_starting_addr"] = [
                config["in2regfile_0"]["cycle_starting_addr"][0] + 1
            ]

            # Set write/read data stride to 0 as we always write/read to the same addr
            config["in2regfile_0"]["write_data_stride"] = [0, 0]
            config["regfile2out_0"]["read_data_stride"] = [0, 0]

            metadata["config"] = config
            tile_inst["metadata"] = metadata

        # Update output IO tile
        io_out_name = "io16_hw_output_stencil_op_hcompute_hw_output_stencil_write_0"
        # Calculate cycles computing partial sum
        psum_cycles = (
            halide_args_dict["vec_width"]
            * halide_args_dict["vec_height"]
            // pow(2, halide_args_dict["tree_stages"])
        )
        assert io_out_name in instance_dict, f"Error: '{io_out_name}' not found in instance_dict."
        io16_inst = instance_dict[io_out_name]
        io16_meta = io16_inst.get("metadata", {})
        in2glb_0 = io16_meta.get("in2glb_0", {})

        # Output cycle = psum pond's in2regfile_0 cycle_starting_addr + tot_elems / tree_width
        tile_inst = instance_dict[psum_pond_name]
        tile_meta = tile_inst.get("metadata", {})
        tile_conf = tile_meta.get("config", {})

        base_addr = tile_conf["in2regfile_0"]["cycle_starting_addr"][0]
        in2glb_0["cycle_starting_addr"] = [base_addr + psum_cycles]
        # TODO: Set extent to 2 (should set this to 1 since we only have 1 output, but then can't get f2g interrupt)
        in2glb_0["extent"] = [2]

        io16_meta["in2glb_0"] = in2glb_0
        io16_inst["metadata"] = io16_meta

        # Update stencil MEM to be the same as output IO tile
        hw_portctrl_garnet = "op_hcompute_hw_output_stencil_port_controller_garnet"
        if hw_portctrl_garnet in instance_dict and io_out_name in instance_dict:
            hw_inst = instance_dict[hw_portctrl_garnet]
            hw_meta = hw_inst.get("metadata", {})
            hw_conf = hw_meta.get("config", {})
            if "stencil_valid" not in hw_conf:
                hw_conf["stencil_valid"] = {}

            stencil_valid = hw_conf["stencil_valid"]

            # Copy from io16_hw_output_stencil_op_hcompute_hw_output_stencil_write_0 -> in2glb_0
            io16_inst = instance_dict[io_out_name]
            io16_meta = io16_inst.get("metadata", {})
            in2glb_0 = io16_meta.get("in2glb_0", {})

            stencil_valid["cycle_starting_addr"] = in2glb_0["cycle_starting_addr"]
            stencil_valid["cycle_stride"] = in2glb_0["cycle_stride"]
            stencil_valid["extent"] = in2glb_0["extent"]

            hw_conf["stencil_valid"] = stencil_valid
            hw_meta["config"] = hw_conf
            hw_inst["metadata"] = hw_meta

        # Overwrite the JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

    def hack_for_scalar_avg_fp_static(self, json_path, bin_path):

        # TODO: Hardcode input pipelining regs for now
        INPUT_PIPELINING_REGS = 3

        with open(json_path, "r") as f:
            design = json.load(f)

        top_module = "scalar_avg_fp"
        tree_out_pond = "tree_3_stencil$ub_tree_3_stencil_BANK_0_garnet"
        tree_out_pond_clk = "tree_3_stencil$ub_tree_3_stencil_BANK_0_clk_en_const"

        # Const PE instance to remove
        pe_to_remove = "op_hcompute_output_cgra_stencil$inner_compute$const_"

        # Locate "scalar_avg_fp" module
        global_modules = design["namespaces"]["global"]["modules"]
        if top_module not in global_modules:
            print(f"WARNING: Module '{top_module}' not found in design. No hack applied.")
            return
        scalar_avg_fp = global_modules[top_module]

        # Remove the tile instance and its clk_en_const
        instance_dict = scalar_avg_fp.get("instances", {})
        if tree_out_pond in instance_dict:
            del instance_dict[tree_out_pond]
        if tree_out_pond_clk in instance_dict:
            del instance_dict[tree_out_pond_clk]

        # Remove references to the tile from connections,
        #    capturing upstream => old_input, downstream => old_output
        old_input = None
        old_output = None
        new_connections = []

        connection_list = scalar_avg_fp.get("connections", [])
        for conn in connection_list:
            left, right = conn[0], conn[1]

            # Check references to tile or its clk const
            refs_tile_left = (tree_out_pond in left) or (tree_out_pond_clk in left)
            refs_tile_right = (tree_out_pond in right) or (tree_out_pond_clk in right)

            if not (refs_tile_left or refs_tile_right):
                # If no reference to tile, keep it for now
                new_connections.append(conn)
                continue

            # If referencing the tile, skip it but see if it's data_in or data_out
            if tree_out_pond in left:
                if ".data_in_pond_" in left or ".data_in_" in left:
                    old_input = right
            elif tree_out_pond in right:
                if ".data_in_pond_" in right or ".data_in_" in right:
                    old_input = left

            if tree_out_pond in left:
                if ".data_out_pond_" in left or ".data_out_" in left:
                    old_output = right
            elif tree_out_pond in right:
                if ".data_out_pond_" in right or ".data_out_" in right:
                    old_output = left

        # Reconnect old_input -> old_output if both exist
        if old_input and old_output:
            new_connections.append([old_input, old_output])
        else:
            print(
                f"WARNING: Could not find both data_in and data_out references for "
                f"tile '{tree_out_pond}'. No direct reconnection added."
            )

        # Remove the PE instance "op_hcompute_output_cgra_stencil$inner_compute$const_"
        #    from the instance dictionary (if present)
        if pe_to_remove in instance_dict:
            del instance_dict[pe_to_remove]

        # Filter out any connections referencing const PE instance
        final_connections = []
        for conn in new_connections:
            left, right = conn[0], conn[1]
            if pe_to_remove in left or pe_to_remove in right:
                # skip any references to that PE
                continue
            final_connections.append(conn)

        # Update the final connection list
        scalar_avg_fp["connections"] = final_connections

        # Get Halide generator arguments
        HALIDE_GEN_ARGS = os.environ.get("HALIDE_GEN_ARGS")
        print(f"HALIDE_GEN_ARGS: {HALIDE_GEN_ARGS}")
        halide_args_dict = dict(item.split("=") for item in HALIDE_GEN_ARGS.split())
        # Convert values to integers where applicable
        halide_args_dict = {k: int(v) for k, v in halide_args_dict.items()}

        # Modify psum pond schedule
        psum_pond_name = "output_cgra_stencil$ub_output_cgra_stencil_BANK_0_garnet"
        if psum_pond_name in instance_dict:
            tile_inst = instance_dict[psum_pond_name]
            metadata = tile_inst.get("metadata", {})
            config = metadata.get("config", {})

            # Remove 'in2regfile_1' and 'regfile2out_1' if present
            if "in2regfile_1" in config:
                del config["in2regfile_1"]
            if "regfile2out_1" in config:
                del config["regfile2out_1"]

            # Adjust cycle_starting_addr for inputs
            assert "in2regfile_0" in config, "Error: 'in2regfile_0' not found in config."
            input_pipelining_regs = INPUT_PIPELINING_REGS
            config["in2regfile_0"]["cycle_starting_addr"] = [input_pipelining_regs]

            # For regfile2out_0, set cycle_starting_addr to in2regfile_0 + 1
            assert "regfile2out_0" in config, "Error: 'regfile2out_0' not found in config."
            config["regfile2out_0"]["cycle_starting_addr"] = [
                config["in2regfile_0"]["cycle_starting_addr"][0] + 1
            ]

            # Set write/read data stride to 0 as we always write/read to the same addr
            config["in2regfile_0"]["write_data_stride"] = [0, 0]
            config["regfile2out_0"]["read_data_stride"] = [0, 0]

            metadata["config"] = config
            tile_inst["metadata"] = metadata

        # Update output IO tile
        io_out_name = "io16_hw_output_stencil_op_hcompute_hw_output_stencil_write_0"

        # Calculate cycles computing partial sum
        psum_cycles = (
            halide_args_dict["vec_width"]
            * halide_args_dict["vec_height"]
            // pow(2, halide_args_dict["tree_stages"])
        )
        assert io_out_name in instance_dict, f"Error: '{io_out_name}' not found in instance_dict."
        io16_inst = instance_dict[io_out_name]
        io16_meta = io16_inst.get("metadata", {})
        in2glb_0 = io16_meta.get("in2glb_0", {})

        # Output cycle = psum pond's in2regfile_0 cycle_starting_addr + tot_elems / tree_width
        tile_inst = instance_dict[psum_pond_name]
        tile_meta = tile_inst.get("metadata", {})
        tile_conf = tile_meta.get("config", {})

        base_addr = tile_conf["in2regfile_0"]["cycle_starting_addr"][0]
        in2glb_0["cycle_starting_addr"] = [base_addr + psum_cycles]
        # TODO: Set extent to 2 (should set this to 1 since we only have 1 output, but then can't get f2g interrupt)
        in2glb_0["extent"] = [2]

        io16_meta["in2glb_0"] = in2glb_0
        io16_inst["metadata"] = io16_meta

        # Update stencil MEM to be the same as output IO tile
        hw_portctrl_garnet = "op_hcompute_hw_output_stencil_port_controller_garnet"
        if hw_portctrl_garnet in instance_dict and io_out_name in instance_dict:
            hw_inst = instance_dict[hw_portctrl_garnet]
            hw_meta = hw_inst.get("metadata", {})
            hw_conf = hw_meta.get("config", {})
            if "stencil_valid" not in hw_conf:
                hw_conf["stencil_valid"] = {}

            stencil_valid = hw_conf["stencil_valid"]

            # Copy from io16_hw_output_stencil_op_hcompute_hw_output_stencil_write_0 -> in2glb_0
            io16_inst = instance_dict[io_out_name]
            io16_meta = io16_inst.get("metadata", {})
            in2glb_0 = io16_meta.get("in2glb_0", {})

            stencil_valid["cycle_starting_addr"] = in2glb_0["cycle_starting_addr"]
            stencil_valid["cycle_stride"] = in2glb_0["cycle_stride"]
            stencil_valid["extent"] = in2glb_0["extent"]

            hw_conf["stencil_valid"] = stencil_valid
            hw_meta["config"] = hw_conf
            hw_inst["metadata"] = hw_meta

        # Overwrite the JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

    def hack_for_silu_pass2_fp_static(self, json_path, bin_path):

        with open(json_path, "r") as f:
            design = json.load(f)

        top_module = "silu_pass2_fp"

        # Locate "stable_softmax_pass2_fp" module
        global_modules = design["namespaces"]["global"]["modules"]
        if top_module not in global_modules:
            print(f"WARNING: Module '{top_module}' not found in design. No hack applied.")
            return
        silu_pass2_fp = global_modules[top_module]
        instance_dict = silu_pass2_fp.get("instances", {})

        # Remove replicated IOs to stream pass1_out and only keep one for broadcasting to divison PEs
        max_io_prefix = "io16in_pass1_out_host_stencil_"
        all_inst = list(instance_dict.keys())
        pass3_sum_insts = [inst for inst in all_inst if inst.startswith(max_io_prefix)]
        if len(pass3_sum_insts) > 1:
            # Sort them and keep the first
            pass3_sum_insts.sort()
            keep_inst = pass3_sum_insts[0]
            remove_insts = pass3_sum_insts[1:]

            # Remove from instance_dict
            for r in remove_insts:
                del instance_dict[r]

            # The module "type" is silu_pass2_fp["type"] = ["Record",[...]]
            type_list = silu_pass2_fp["type"][1]

            # Convert instance name -> the corresponding type prefix
            def inst_to_field_prefix(name):
                if name.startswith("io16in_"):
                    return name[len("io16in_") :]
                return name

            keep_prefix = inst_to_field_prefix(keep_inst)
            remove_prefixes = [inst_to_field_prefix(r) for r in remove_insts]

            # Update the type record, removing fields for the removed instances
            new_type_list = []
            for field_pair in type_list:
                field_name, field_type = field_pair
                if "io16in_pass1_out_host_stencil_" not in field_name:
                    # Not a vecmax field => keep
                    new_type_list.append(field_pair)
                    continue
                # It's a vecmax field => keep only if it belongs to keep_prefix
                # (i.e. starts with keep_prefix)
                if field_name.startswith(keep_prefix):
                    new_type_list.append(field_pair)
                # else skip
            silu_pass2_fp["type"][1] = new_type_list

            # Fix up connections: if referencing removed inst or fields, unify to keep inst
            new_conn = []
            for left, right in silu_pass2_fp["connections"]:
                new_left, new_right = left, right

                # If the removed instance name is in the path, unify to keep_inst
                for rinst in remove_insts:
                    if rinst in new_left:
                        new_left = new_left.replace(rinst, keep_inst)
                    if rinst in new_right:
                        new_right = new_right.replace(rinst, keep_inst)

                # If the removed field prefix is in the path, unify it to keep_prefix
                for rpref in remove_prefixes:
                    if rpref in new_left:
                        new_left = new_left.replace(rpref, keep_prefix)
                    if rpref in new_right:
                        new_right = new_right.replace(rpref, keep_prefix)

                new_conn.append([new_left, new_right])

            silu_pass2_fp["connections"] = new_conn

        # Deduplicate final connections: remove exact duplicates
        final_dedup = []
        seen = set()
        for c in silu_pass2_fp["connections"]:
            # Represent connection as tuple (left, right)
            t = tuple(c)
            if t not in seen:
                seen.add(t)
                final_dedup.append(c)

        silu_pass2_fp["connections"] = final_dedup

        # Configure the IO to read the same addr of GLB repeatedly
        # TODO: this should be replaced by reading from MEM tile instead for power efficiency
        for inst_name, inst_config in instance_dict.items():
            if (
                "io16in_pass1_out_host_stencil" in inst_name
                and inst_config["modref"] == "global.IO"
            ):
                inst_config["metadata"]["glb2out_0"]["read_data_stride"] = [0]

        # Overwrite the JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

    def hack_for_swiglu_pass2_fp_static(self, json_path, bin_path):

        with open(json_path, "r") as f:
            design = json.load(f)

        top_module = "swiglu_pass2_fp"

        # Locate "stable_softmax_pass2_fp" module
        global_modules = design["namespaces"]["global"]["modules"]
        if top_module not in global_modules:
            print(f"WARNING: Module '{top_module}' not found in design. No hack applied.")
            return
        swiglu_pass2_fp = global_modules[top_module]
        instance_dict = swiglu_pass2_fp.get("instances", {})

        # Remove replicated IOs to stream pass1_out and only keep one for broadcasting to divison PEs
        max_io_prefix = "io16in_pass1_out_host_stencil_"
        all_inst = list(instance_dict.keys())
        pass3_sum_insts = [inst for inst in all_inst if inst.startswith(max_io_prefix)]
        if len(pass3_sum_insts) > 1:
            # Sort them and keep the first
            pass3_sum_insts.sort()
            keep_inst = pass3_sum_insts[0]
            remove_insts = pass3_sum_insts[1:]

            # Remove from instance_dict
            for r in remove_insts:
                del instance_dict[r]

            # The module "type" is swiglu_pass2_fp["type"] = ["Record",[...]]
            type_list = swiglu_pass2_fp["type"][1]

            # Convert instance name -> the corresponding type prefix
            def inst_to_field_prefix(name):
                if name.startswith("io16in_"):
                    return name[len("io16in_") :]
                return name

            keep_prefix = inst_to_field_prefix(keep_inst)
            remove_prefixes = [inst_to_field_prefix(r) for r in remove_insts]

            # Update the type record, removing fields for the removed instances
            new_type_list = []
            for field_pair in type_list:
                field_name, field_type = field_pair
                if "io16in_pass1_out_host_stencil_" not in field_name:
                    # Not a vecmax field => keep
                    new_type_list.append(field_pair)
                    continue
                # It's a vecmax field => keep only if it belongs to keep_prefix
                # (i.e. starts with keep_prefix)
                if field_name.startswith(keep_prefix):
                    new_type_list.append(field_pair)
                # else skip
            swiglu_pass2_fp["type"][1] = new_type_list

            # Fix up connections: if referencing removed inst or fields, unify to keep inst
            new_conn = []
            for left, right in swiglu_pass2_fp["connections"]:
                new_left, new_right = left, right

                # If the removed instance name is in the path, unify to keep_inst
                for rinst in remove_insts:
                    if rinst in new_left:
                        new_left = new_left.replace(rinst, keep_inst)
                    if rinst in new_right:
                        new_right = new_right.replace(rinst, keep_inst)

                # If the removed field prefix is in the path, unify it to keep_prefix
                for rpref in remove_prefixes:
                    if rpref in new_left:
                        new_left = new_left.replace(rpref, keep_prefix)
                    if rpref in new_right:
                        new_right = new_right.replace(rpref, keep_prefix)

                new_conn.append([new_left, new_right])

            swiglu_pass2_fp["connections"] = new_conn

        # Deduplicate final connections: remove exact duplicates
        final_dedup = []
        seen = set()
        for c in swiglu_pass2_fp["connections"]:
            # Represent connection as tuple (left, right)
            t = tuple(c)
            if t not in seen:
                seen.add(t)
                final_dedup.append(c)

        swiglu_pass2_fp["connections"] = final_dedup

        # Configure the IO to read the same addr of GLB repeatedly
        # TODO: this should be replaced by reading from MEM tile instead for power efficiency
        for inst_name, inst_config in instance_dict.items():
            if (
                "io16in_pass1_out_host_stencil" in inst_name
                and inst_config["modref"] == "global.IO"
            ):
                inst_config["metadata"]["glb2out_0"]["read_data_stride"] = [0]

        # Overwrite the JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

    def hack_for_scalar_reduction_fp_rv(self, json_path, bin_path):

        with open(json_path, "r") as f:
            design = json.load(f)

        top_module = "scalar_reduction_fp"
        tree_out_pond = "tree_3_stencil$ub_tree_3_stencil_BANK_0_garnet"
        tree_out_pond_clk = "tree_3_stencil$ub_tree_3_stencil_BANK_0_clk_en_const"

        # Const PE instance to remove
        pe_to_remove = "op_hcompute_output_cgra_stencil$inner_compute$const_"

        # Locate "scalar_reduction_fp" module
        global_modules = design["namespaces"]["global"]["modules"]
        if top_module not in global_modules:
            print(f"WARNING: Module '{top_module}' not found in design. No hack applied.")
            return
        scalar_reduction_fp = global_modules[top_module]

        # Remove the tile instance and its clk_en_const
        instance_dict = scalar_reduction_fp.get("instances", {})
        if tree_out_pond in instance_dict:
            del instance_dict[tree_out_pond]
        if tree_out_pond_clk in instance_dict:
            del instance_dict[tree_out_pond_clk]

        # Remove references to the tile from connections,
        #    capturing upstream => old_input, downstream => old_output
        old_input = None
        old_output = None
        new_connections = []

        connection_list = scalar_reduction_fp.get("connections", [])
        for conn in connection_list:
            left, right = conn[0], conn[1]

            # Check references to tile or its clk const
            refs_tile_left = (tree_out_pond in left) or (tree_out_pond_clk in left)
            refs_tile_right = (tree_out_pond in right) or (tree_out_pond_clk in right)

            if not (refs_tile_left or refs_tile_right):
                # If no reference to tile, keep it for now
                new_connections.append(conn)
                continue

            # If referencing the tile, skip it but see if it's data_in or data_out
            if tree_out_pond in left:
                if ".data_in_pond_" in left or ".data_in_" in left:
                    old_input = right
            elif tree_out_pond in right:
                if ".data_in_pond_" in right or ".data_in_" in right:
                    old_input = left

            if tree_out_pond in left:
                if ".data_out_pond_" in left or ".data_out_" in left:
                    old_output = right
            elif tree_out_pond in right:
                if ".data_out_pond_" in right or ".data_out_" in right:
                    old_output = left

        # Reconnect old_input -> old_output if both exist
        if old_input and old_output:
            new_connections.append([old_input, old_output])
        else:
            print(
                f"WARNING: Could not find both data_in and data_out references for "
                f"tile '{tree_out_pond}'. No direct reconnection added."
            )

        # Remove the PE instance "op_hcompute_output_cgra_stencil$inner_compute$const_"
        #    from the instance dictionary (if present)
        if pe_to_remove in instance_dict:
            del instance_dict[pe_to_remove]

        # Filter out any connections referencing const PE instance
        final_connections = []
        for conn in new_connections:
            left, right = conn[0], conn[1]
            if pe_to_remove in left or pe_to_remove in right:
                # skip any references to that PE
                continue
            final_connections.append(conn)

        # Update the final connection list
        scalar_reduction_fp["connections"] = final_connections

        # Identify psum PE with Pond and generate fifo bypass config
        # Write bypass config to design_top at the top level
        psum_pe_name = "op_hcompute_output_cgra_stencil_1$inner_compute$float_DW_fp_add_"
        PE_fifos_bypass_config = {
            psum_pe_name: {
                # HACK: Assuming the PE input port connected to Pond is "data_in_0"
                # We bypass the input and output fifos on the feedback path
                "input_fifo_bypass": [1, 0, 0],
                "output_fifo_bypass": 1,
            }
        }
        PE_fifos_bypass_config_path = os.path.join(bin_path, "PE_fifos_bypass_config.json")
        print(f"Writing PE_fifos_bypass_config to {PE_fifos_bypass_config_path}")
        with open(PE_fifos_bypass_config_path, "w") as f:
            json.dump(PE_fifos_bypass_config, f, indent=2)

        # Update output IO tile extent
        # HACK: Set extent to 2 (should set this to 1 since we only have 1 output, but then can't get f2g interrupt)
        # Also have to update cycle_start_addr and stride, as it means handshake rather than cycles in dense rv scenario
        io_out_name = "io16_hw_output_stencil_op_hcompute_hw_output_stencil_write_0"
        instance_dict[io_out_name]["metadata"]["in2glb_0"]["extent"] = [2]
        # We want to accept every valid handshake starting from the first one
        instance_dict[io_out_name]["metadata"]["in2glb_0"]["cycle_starting_addr"] = [0]
        instance_dict[io_out_name]["metadata"]["in2glb_0"]["cycle_stride"] = [1]

        # Overwrite the JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

    def vector_reduction_tree_helper(
        self,
        json_path,
        bin_path,
        top_module="vector_reduction_fp",
        tree_end_pe_name="op_hcompute_output_cgra_stencil_1$inner_compute$float_DW_fp_add_",
        old_accum_pond_name = "output_cgra_stencil$ub_output_cgra_stencil_BANK_0_garnet",
        old_accum_pond_clk_name = "output_cgra_stencil$ub_output_cgra_stencil_BANK_0_clk_en_const",
        old_const_feeder_pe_name = "op_hcompute_output_cgra_stencil$inner_compute$const_i3085_i631",
    ):
        with open(json_path, "r") as f:
            design = json.load(f)

        global_modules = design["namespaces"]["global"]["modules"]
        if top_module not in global_modules:
            print(f"WARNING: Module '{top_module}' not found in design. No hack applied.")
            return

        top_module_json = global_modules[top_module]
        instance_dict = top_module_json.get("instances", {})
        original_connections = top_module_json.get("connections", [])

        # Old instances to remove
        tree_stages = int(self.halide_gen_args_dict["tree_stages"])
        old_tree_pond_instance_name = f"tree_{tree_stages}_stencil$ub_tree_{tree_stages}_stencil_BANK_0_garnet"
        old_tree_pond_clk_instance_name = f"tree_{tree_stages}_stencil$ub_tree_{tree_stages}_stencil_BANK_0_clk_en_const"
        old_single_accum_pe_name = tree_end_pe_name
        old_single_accum_pond_name = old_accum_pond_name
        old_single_accum_pond_clk = old_accum_pond_clk_name
        old_const_feeder_pe = old_const_feeder_pe_name

        # New instances to add
        filter_mem_instance_name = f"{top_module}_filter_mem"
        accum_pond_0_name = f"{top_module}_accum_pond_0"
        accum_pond_1_name = f"{top_module}_accum_pond_1"
        accum_pe_0_name = f"{top_module}_accum_pe_0"
        accum_pe_1_name = f"{top_module}_accum_pe_1"
        final_reduce_pe_name = f"{top_module}_final_reduce_pe"
        accum_pe_0_inst_const_name = f"{top_module}_accum_pe_0_inst_const"
        accum_pe_1_inst_const_name = f"{top_module}_accum_pe_1_inst_const"
        final_reduce_pe_inst_const_name = f"{top_module}_final_reduce_pe_inst_const"
        shared_clk_const_name = f"{top_module}_clk_en_const"

        tree_output_pe_partial_name = f"op_hcompute_tree_{tree_stages}_stencil$inner_compute$float_"
        tree_output_pe_name = None
        for temp_instance in instance_dict:
            if tree_output_pe_partial_name in temp_instance:
                tree_output_pe_name = temp_instance
                break
        assert tree_output_pe_name is not None, f"ERROR: Could not find tree output PE instance name for {top_module}"
        tree_output_signal = tree_output_pe_name + ".O0"
        io_out_name = "io16_hw_output_stencil_op_hcompute_hw_output_stencil_write_0"
        io_in_port = f"{io_out_name}.in"

        # Delete old instances
        for name in [
            old_tree_pond_instance_name,
            old_tree_pond_clk_instance_name,
            old_single_accum_pe_name,
            old_single_accum_pond_name,
            old_single_accum_pond_clk,
            old_const_feeder_pe,
        ]:
            if name in instance_dict:
                del instance_dict[name]

        # Shared clk const
        if shared_clk_const_name not in instance_dict:
            instance_dict[shared_clk_const_name] = copy.deepcopy(self.const_clk_tpl)

        # Filter mem
        if filter_mem_instance_name not in instance_dict:
            instance_dict[filter_mem_instance_name] = copy.deepcopy(self.mem_tpl)

        # Accum ponds
        if accum_pond_0_name not in instance_dict:
            instance_dict[accum_pond_0_name] = copy.deepcopy(self.pond_tpl)
        if accum_pond_1_name not in instance_dict:
            instance_dict[accum_pond_1_name] = copy.deepcopy(self.pond_tpl)

        # PEs
        if accum_pe_0_name not in instance_dict:
            instance_dict[accum_pe_0_name] = copy.deepcopy(self.pe_tpl)
        if accum_pe_1_name not in instance_dict:
            instance_dict[accum_pe_1_name] = copy.deepcopy(self.pe_tpl)
        if final_reduce_pe_name not in instance_dict:
            instance_dict[final_reduce_pe_name] = copy.deepcopy(self.pe_tpl)

        # Find PE instruction from reduction tree
        pe_inst_source_name = "op_hcompute_tree_1_stencil$inner_compute$c0"
        if pe_inst_source_name not in instance_dict:
            raise RuntimeError(f"[ERROR] Instruction node '{pe_inst_source_name}' not found to clone.")
        pe_inst_source = instance_dict[pe_inst_source_name]

        if accum_pe_0_inst_const_name not in instance_dict:
            instance_dict[accum_pe_0_inst_const_name] = copy.deepcopy(pe_inst_source)
        if accum_pe_1_inst_const_name not in instance_dict:
            instance_dict[accum_pe_1_inst_const_name] = copy.deepcopy(pe_inst_source)
        if final_reduce_pe_inst_const_name not in instance_dict:
            instance_dict[final_reduce_pe_inst_const_name] = copy.deepcopy(pe_inst_source)

        # Write PE fifos bypass config - only bypass tile-level output fifos in accumulator PEs
        bypass_cfg = {
            accum_pe_0_name: {"input_fifo_bypass": [0, 0, 0], "output_fifo_bypass": 1},
            accum_pe_1_name: {"input_fifo_bypass": [0, 0, 0], "output_fifo_bypass": 1},
        }
        bypass_path = os.path.join(bin_path, "PE_fifos_bypass_config.json")
        print(f"Writing PE_fifos_bypass_config to {bypass_path}")
        with open(bypass_path, "w") as f:
            json.dump(bypass_cfg, f, indent=2)

        connections = []

        # Helper function to add a connection only if it doesn't already exist
        def add_conn_once(src, dst):
            pair = [src, dst]
            if pair not in connections:
                connections.append(pair)

        removed_instances = {
            old_tree_pond_instance_name,
            old_tree_pond_clk_instance_name,
            old_single_accum_pe_name,
            old_single_accum_pond_name,
            old_single_accum_pond_clk,
            old_const_feeder_pe,
        }

        for left, right in original_connections:
            if any(rem in left or rem in right for rem in removed_instances):
                continue
            if tree_output_signal in left or tree_output_signal in right:
                tree_output_signal = tree_output_pe_name + ".O0"
            add_conn_once(left, right)

        # New connections
        add_conn_once(tree_output_signal, f"{filter_mem_instance_name}.data_in_0")

        add_conn_once(f"{shared_clk_const_name}.out", f"{filter_mem_instance_name}.clk_en")
        add_conn_once(f"{shared_clk_const_name}.out", f"{accum_pond_0_name}.clk_en")
        add_conn_once(f"{shared_clk_const_name}.out", f"{accum_pond_1_name}.clk_en")

        add_conn_once(f"{filter_mem_instance_name}.data_out_0", f"{accum_pe_0_name}.data1")
        add_conn_once(f"{filter_mem_instance_name}.data_out_1", f"{accum_pe_1_name}.data1")

        add_conn_once(f"{accum_pe_0_inst_const_name}.out", f"{accum_pe_0_name}.inst")
        add_conn_once(f"{accum_pe_1_inst_const_name}.out", f"{accum_pe_1_name}.inst")
        add_conn_once(f"{final_reduce_pe_inst_const_name}.out", f"{final_reduce_pe_name}.inst")

        add_conn_once(f"{accum_pond_0_name}.data_out_pond_0", f"{accum_pe_0_name}.data0")
        add_conn_once(f"{accum_pond_0_name}.data_in_pond_0", f"{accum_pe_0_name}.O0")
        add_conn_once(f"{accum_pond_0_name}.data_out_pond_1", f"{final_reduce_pe_name}.data0")

        add_conn_once(f"{accum_pond_1_name}.data_out_pond_0", f"{accum_pe_1_name}.data0")
        add_conn_once(f"{accum_pond_1_name}.data_in_pond_0", f"{accum_pe_1_name}.O0")
        add_conn_once(f"{accum_pond_1_name}.data_out_pond_1", f"{final_reduce_pe_name}.data1")

        add_conn_once(f"{final_reduce_pe_name}.O0", io_in_port)

        # Configure output IO metadata
        vec_length = int(self.halide_gen_args_dict["vec_width"])
        num_vecs = int(self.halide_gen_args_dict["vec_height"])
        glb_i = int(self.halide_gen_args_dict["glb_i"])
        instance_dict[io_out_name]["metadata"]["in2glb_0"]["extent"] = [num_vecs]
        instance_dict[io_out_name]["metadata"]["in2glb_0"]["cycle_starting_addr"] = [0]
        instance_dict[io_out_name]["metadata"]["in2glb_0"]["cycle_stride"] = [1]

        # Configure input IO metadata
        for inst_name, inst_conf in instance_dict.items():
            if "io16in_input_host_stencil" in inst_name and inst_conf.get("modref") == "global.IO":
                md = inst_conf["metadata"]["glb2out_0"]
                md["cycle_starting_addr"] = [0]
                md["cycle_stride"] = [1]
                md["dimensionality"] = 1
                md["extent"] = [num_vecs * vec_length // glb_i]
                md["read_data_starting_addr"] = [0]
                md["read_data_stride"] = [1]

        top_module_json["connections"] = connections

        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

        # Update design_meta_halide.json to update input and output shapes
        design_meta_path = os.path.join(bin_path, "design_meta_halide.json")
        with open(design_meta_path, "r") as f:
            design_meta = json.load(f)
        assert len(design_meta["IOs"]["inputs"]) == 1, "Expected only one input"
        assert len(design_meta["IOs"]["outputs"]) == 1, "Expected only one output"
        design_meta["IOs"]["inputs"][0]["shape"] = [vec_length, num_vecs]
        design_meta["IOs"]["outputs"][0]["shape"] = [num_vecs]
        with open(design_meta_path, "w") as f:
            json.dump(design_meta, f, indent=2)

    def hack_for_vector_reduction_fp_rv(self, json_path, bin_path, top_module="vector_reduction_fp"):
        self.vector_reduction_tree_helper(json_path, bin_path, top_module)

    def hack_for_mat_vec_mul_fp_rv(self, json_path, bin_path, top_module="mat_vec_mul_fp"):
        # Load the design JSON
        with open(json_path, "r") as f:
            design = json.load(f)

        tree_out_pond_pattern = r"^tree_\d+_stencil\$ub_tree_\d+_stencil_BANK_\d+_garnet$"
        tree_out_pond_clk_pattern = r"^tree_\d+_stencil\$ub_tree_\d+_stencil_BANK_\d+_clk_en_const$"
        const_pe_to_remove = "op_hcompute_output_cgra_stencil$inner_compute$const_"
        single_accum_pe_prefix = "op_hcompute_output_cgra_stencil_1$inner_compute$float_DW_fp_add_"
        old_output_pond = "output_cgra_stencil$ub_output_cgra_stencil_BANK_0_garnet"
        old_output_pond_clk = "output_cgra_stencil$ub_output_cgra_stencil_BANK_0_clk_en_const"
        old_output_pe_const = "op_hcompute_output_cgra_stencil_1$inner_compute$c0"
        out_io = "io16_hw_output_stencil_op_hcompute_hw_output_stencil_write_0"

        # Locate the top module
        global_modules = design["namespaces"]["global"]["modules"]
        if top_module not in global_modules:
            print(f"WARNING: Module '{top_module}' not found in design. No hack applied.")
            return
        mat = global_modules[top_module]
        instance_dict = mat.get("instances", {})
        connection_list = mat.get("connections", [])

        # Remove unncessary reduction tree pond/mem before accum PE
        # Delete instance first
        ponds = [k for k in instance_dict if re.match(tree_out_pond_pattern, k)]
        clks = [k for k in instance_dict if re.match(tree_out_pond_clk_pattern, k)]
        for k in ponds + clks:
            del instance_dict[k]
        # Hook up connection
        old_input_signal = None
        old_output_sink = None
        kept_connections = []
        for left, right in connection_list:
            left_base = left.split(".")[0]
            right_base = right.split(".")[0]

            match_left = re.match(tree_out_pond_pattern, left_base) or re.match(
                tree_out_pond_clk_pattern, left_base
            )
            match_right = re.match(tree_out_pond_pattern, right_base) or re.match(
                tree_out_pond_clk_pattern, right_base
            )

            if not (match_left or match_right):
                kept_connections.append([left, right])
                continue

            if match_left:
                if ".data_in" in left:
                    old_input_signal = right
                if ".data_out" in left:
                    old_output_sink = right
            if match_right:
                if ".data_in" in right:
                    old_input_signal = left
                if ".data_out" in right:
                    old_output_sink = left

        if not (old_input_signal and old_output_sink):
            raise RuntimeError("[ERROR] Could not find data_in and data_out for reduction pond tile.")

        # Remove const PE instance and its connections
        if const_pe_to_remove in instance_dict:
            del instance_dict[const_pe_to_remove]

        # Remove single accum PE
        single_accum_pes = [
            name for name in instance_dict if name.startswith(single_accum_pe_prefix)
        ]
        for name in single_accum_pes:
            del instance_dict[name]

        # Remove old output pond and clk
        for name in [old_output_pond, old_output_pond_clk, old_output_pe_const]:
            if name in instance_dict:
                del instance_dict[name]

        # Filter connections
        connections = []

        def add_conn_once(src, dst):
            pair = [src, dst]
            if pair not in connections:
                connections.append(pair)
        removed_instances = set(ponds + clks + [const_pe_to_remove] + single_accum_pes + [old_output_pond, old_output_pond_clk, old_output_pe_const])

        for left, right in kept_connections:
            if any(rem in left or rem in right for rem in removed_instances):
                continue
            add_conn_once(left, right)

        # Build new reduction: mem -> 2 accums -> final reduce -> IO
        shared_clk_const_name = f"{top_module}_clk_en_const"
        filter_mem_instance_name = f"{top_module}_filter_mem"
        accum_pond_0_name = f"{top_module}_accum_pond_0"
        accum_pond_1_name = f"{top_module}_accum_pond_1"
        accum_pe_0_name = f"{top_module}_accum_pe_0"
        accum_pe_1_name = f"{top_module}_accum_pe_1"
        final_reduce_pe_name = f"{top_module}_final_reduce_pe"
        accum_pe_0_inst_const_name = f"{top_module}_accum_pe_0_inst_const"
        accum_pe_1_inst_const_name = f"{top_module}_accum_pe_1_inst_const"
        final_reduce_pe_inst_const_name = f"{top_module}_final_reduce_pe_inst_const"

        # Shared clk const
        if shared_clk_const_name not in instance_dict:
            instance_dict[shared_clk_const_name] = copy.deepcopy(self.const_clk_tpl)

        # Filter MEMs
        if filter_mem_instance_name not in instance_dict:
            instance_dict[filter_mem_instance_name] = copy.deepcopy(self.mem_tpl)

        # Accum ponds
        if accum_pond_0_name not in instance_dict:
            instance_dict[accum_pond_0_name] = copy.deepcopy(self.pond_tpl)
        if accum_pond_1_name not in instance_dict:
            instance_dict[accum_pond_1_name] = copy.deepcopy(self.pond_tpl)

        # two accumulator PEs + final reducer
        if accum_pe_0_name not in instance_dict:
            instance_dict[accum_pe_0_name] = copy.deepcopy(self.pe_tpl)
        if accum_pe_1_name not in instance_dict:
            instance_dict[accum_pe_1_name] = copy.deepcopy(self.pe_tpl)
        if final_reduce_pe_name not in instance_dict:
            instance_dict[final_reduce_pe_name] = copy.deepcopy(self.pe_tpl)

        # Find PE instruction from reduction tree
        pe_inst_source_name = "op_hcompute_tree_1_stencil$inner_compute$c0"
        if pe_inst_source_name not in instance_dict:
            raise RuntimeError(f"[ERROR] Instruction node '{pe_inst_source_name}' not found to clone.")
        pe_inst_source = instance_dict[pe_inst_source_name]

        if accum_pe_0_inst_const_name not in instance_dict:
            instance_dict[accum_pe_0_inst_const_name] = copy.deepcopy(pe_inst_source)
        if accum_pe_1_inst_const_name not in instance_dict:
            instance_dict[accum_pe_1_inst_const_name] = copy.deepcopy(pe_inst_source)
        if final_reduce_pe_inst_const_name not in instance_dict:
            instance_dict[final_reduce_pe_inst_const_name] = copy.deepcopy(pe_inst_source)

        # Tree output -> filter mem
        add_conn_once(old_input_signal, f"{filter_mem_instance_name}.data_in_0")

        # Clk -> filter mem + ponds
        add_conn_once(f"{shared_clk_const_name}.out", f"{filter_mem_instance_name}.clk_en")
        add_conn_once(f"{shared_clk_const_name}.out", f"{accum_pond_0_name}.clk_en")
        add_conn_once(f"{shared_clk_const_name}.out", f"{accum_pond_1_name}.clk_en")

        # Filter mem -> two accum PEs
        add_conn_once(f"{filter_mem_instance_name}.data_out_0", f"{accum_pe_0_name}.data1")
        add_conn_once(f"{filter_mem_instance_name}.data_out_1", f"{accum_pe_1_name}.data1")

        # Per-PE insts
        add_conn_once(f"{accum_pe_0_inst_const_name}.out", f"{accum_pe_0_name}.inst")
        add_conn_once(f"{accum_pe_1_inst_const_name}.out", f"{accum_pe_1_name}.inst")
        add_conn_once(f"{final_reduce_pe_inst_const_name}.out", f"{final_reduce_pe_name}.inst")

        # Ponds feedback and spill
        add_conn_once(f"{accum_pond_0_name}.data_out_pond_0", f"{accum_pe_0_name}.data0")
        add_conn_once(f"{accum_pond_0_name}.data_in_pond_0", f"{accum_pe_0_name}.O0")
        add_conn_once(f"{accum_pond_0_name}.data_out_pond_1", f"{final_reduce_pe_name}.data0")

        add_conn_once(f"{accum_pond_1_name}.data_out_pond_0", f"{accum_pe_1_name}.data0")
        add_conn_once(f"{accum_pond_1_name}.data_in_pond_0", f"{accum_pe_1_name}.O0")
        add_conn_once(f"{accum_pond_1_name}.data_out_pond_1", f"{final_reduce_pe_name}.data1")

        # Final reduce -> output IO
        add_conn_once(f"{final_reduce_pe_name}.O0", f"{out_io}.in")

        # Write PE fifos bypass config - only bypass tile-level output fifos in accumulator PEs
        bypass_cfg = {
            accum_pe_0_name: {"input_fifo_bypass": [0, 0, 0], "output_fifo_bypass": 1},
            accum_pe_1_name: {"input_fifo_bypass": [0, 0, 0], "output_fifo_bypass": 1},
        }
        bypass_path = os.path.join(bin_path, "PE_fifos_bypass_config.json")
        print(f"Writing PE_fifos_bypass_config to {bypass_path}")
        with open(bypass_path, "w") as f:
            json.dump(bypass_cfg, f, indent=2)

        # Update the output-IO tile's metadata to match matrix_height size
        md = instance_dict[out_io]["metadata"]["in2glb_0"]
        md["extent"] = [int(self.halide_gen_args_dict["matrix_height"])]
        md["cycle_starting_addr"] = [0]
        md["cycle_stride"] = [1]

        # Update the input-IO tile's metadata to match matrix size
        in_io_pattern = "io16in_matrix_host_stencil"
        for in_io_instance in instance_dict:
            if in_io_pattern in in_io_instance:
                md = instance_dict[in_io_instance]["metadata"]["glb2out_0"]
                md["cycle_starting_addr"] = [0]
                md["cycle_stride"] = [1]
                md["dimensionality"] = 1
                md["extent"] = [
                    int(self.halide_gen_args_dict["matrix_width"])
                    * int(self.halide_gen_args_dict["matrix_height"])
                    // int(self.halide_gen_args_dict["glb_i"])
                ]
                md["read_data_starting_addr"] = [0]
                md["read_data_stride"] = [1]

        # Insert new vector-host IO instances & connections
        vector_meta = {
            "glb2out_0": {
                "cycle_starting_addr": [0, 0],
                "cycle_stride": [1, 1],
                "dimensionality": 2,
                "extent": [
                    int(self.halide_gen_args_dict["matrix_width"])
                    // int(self.halide_gen_args_dict["glb_i"]),
                    int(self.halide_gen_args_dict["matrix_height"]),
                ],
                "read_data_starting_addr": [0, 0],
                "read_data_stride": [1, 0],
            }
        }
        mul_re = re.compile(
            r"op_hcompute_tile_input_stencil(?:_(\d+))?" r"\$inner_compute\$float_DW_fp_mul"
        )
        mul_list = sorted(
            (int(m.group(1) or 0), name)
            for name in instance_dict
            for m in [mul_re.match(name)]
            if m
        )
        for idx, pe_name in mul_list:
            io_name = (f"io16in_vector_host_stencil_clkwrk_{idx}_op_hcompute_vector_glb_stencil_{idx}_read_0")
            # Add the IO instance
            instance_dict[io_name] = {
                "modref": "global.IO",
                "modargs": {"mode": ["String", "in"]},
                "metadata": vector_meta,
            }
            # Connect self.vector_host_stencil* to io.in
            add_conn_once(f"self.vector_host_stencil_clkwrk_{idx}_op_hcompute_vector_glb_stencil_{idx}_read_0", f"{io_name}.in")
            # Connect pe.data1 to io.out
            add_conn_once(f"{pe_name}.data1", f"{io_name}.out")
        # Update the module’s type record to include vector_host_stencil fields
        type_fields = mat["type"][1]
        for idx, _ in mul_list:
            type_fields.append(
                [
                    f"vector_host_stencil_clkwrk_{idx}_op_hcompute_vector_glb_stencil_{idx}_read_0",
                    ["Array", 16, "BitIn"],
                ]
            )

        # Update mul PE instruction
        # Change constant multiplier for tile_input PEs from a fixed value
        # to use two input operands.
        const_re = re.compile(r"^op_hcompute_tile_input_stencil(?:_(\d+))?" r"\$inner_compute\$c0$")
        for inst_name, inst in instance_dict.items():
            if const_re.match(inst_name):
                # ensure this is a CoreIR const instance
                if inst.get("genref") == "coreir.const":
                    # inst["modargs"]["value"] is a 2-element list:
                    # [ ["BitVector", width], hexstring ]
                    # update the hexstring to multiply two inputs instead of constant
                    inst["modargs"]["value"][1] = self.FP_MUL_INSTR
                else:
                    raise RuntimeError(f"Instance {inst_name} is not a coreir.const")

        # Reorder the instances entries for matrix & vector IOs
        matrix_ios = []
        vector_ios = []
        other_instances = []
        for instance in list(instance_dict.keys()):
            if "io16in_vector_host_stencil" in instance:
                match = re.search(r"clkwrk_(\d+)_op_hcompute_vector_glb_stencil_\1_read_0$", instance)
                idx = int(match.group(1)) if match else -1
                vector_ios.append((idx, instance))
            elif "io16in_matrix_host_stencil" in instance:
                match = re.search(r"_(\d+)(?:$|_)", instance)
                idx = int(match.group(1)) if match else -1
                matrix_ios.append((idx, instance))
            else:
                other_instances.append(instance)

        # Sort the IO instances by index
        matrix_ios.sort(key=lambda x: x[0])
        vector_ios.sort(key=lambda x: x[0])

        reordered = {}
        for instance in other_instances:
            reordered[instance] = instance_dict[instance]
        for _, instance in matrix_ios:
            reordered[instance] = instance_dict[instance]
        for _, instance in vector_ios:
            reordered[instance] = instance_dict[instance]

        # Overwrite the instance dictionary
        instance_dict.clear()
        instance_dict.update(reordered)

        # Write back connections
        mat["connections"] = connections

        # Write the patched design back out
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

        # Also update design_meta_halide.json to add vector input
        # TODO: This applies hack to design_meta as well. Do we want to create a new class?
        design_meta_path = os.path.join(bin_path, "design_meta_halide.json")
        with open(design_meta_path, "r") as f:
            design_meta = json.load(f)
        design_meta["IOs"]["inputs"].append(
            {
                "bitwidth": 16,
                "datafile": "vector_host_stencil.raw",
                "name": "vector_host_stencil",
                "shape": [int(self.halide_gen_args_dict["matrix_width"])],
            }
        )
        for input in design_meta["IOs"]["inputs"]:
            if input["name"] == "matrix_host_stencil":
                input["shape"] = [
                    int(self.halide_gen_args_dict["matrix_width"]),
                    int(self.halide_gen_args_dict["matrix_height"]),
                ]
        assert len(design_meta["IOs"]["outputs"]) == 1, "Expected only one output"
        design_meta["IOs"]["outputs"][0]["shape"] = [int(self.halide_gen_args_dict["matrix_height"])]

        with open(design_meta_path, "w") as f:
            json.dump(design_meta, f, indent=2)

    def hack_for_fully_connected_layer_fp_rv(self, json_path, bin_path):
        # Implement the hack for mat_vec_mul_fp_rv first and then add bias IO and PE later
        self.hack_for_mat_vec_mul_fp_rv(json_path, bin_path, top_module="fully_connected_layer_fp")

        # Add bias IO and PE
        with open(json_path, "r") as f:
            design = json.load(f)

        top_module = "fully_connected_layer_fp"
        instances = design["namespaces"]["global"]["modules"][top_module]["instances"]
        conns = design["namespaces"]["global"]["modules"][top_module]["connections"]
        type_fields = design["namespaces"]["global"]["modules"][top_module]["type"][1]

        # Names produced by the mat-vec hack
        final_reduce_pe_name = f"{top_module}_final_reduce_pe"
        out_io_name = "io16_hw_output_stencil_op_hcompute_hw_output_stencil_write_0"

        # Bias input IO instantiation and connection
        bias_io_name = "io16in_bias_host_stencil_clkwrk_0_op_hcompute_bias_glb_stencil_0_read_0"
        if bias_io_name not in instances:
            instances[bias_io_name] = {
                "modref": "global.IO",
                "modargs": {"mode": ["String", "in"]},
                "metadata": {
                    "glb2out_0": {
                        "cycle_starting_addr": [0],
                        "cycle_stride": [1],
                        "dimensionality": 1,
                        "extent": [int(self.halide_gen_args_dict["matrix_height"])],
                        "read_data_starting_addr": [0],
                        "read_data_stride": [1],
                    }
                },
            }

            # Module type field change for bias input IO
            type_fields.append(
                [  # keep naming consistent with your port convention
                    "bias_host_stencil_clkwrk_0_op_hcompute_bias_glb_stencil_0_read_0",
                    ["Array", 16, "BitIn"],
                ]
            )
            conns.append(
                [
                    "self.bias_host_stencil_clkwrk_0_op_hcompute_bias_glb_stencil_0_read_0",
                    f"{bias_io_name}.in",
                ]
            )

        # Bias add PE instantiation and connection
        bias_const_name = "op_hcompute_bias_add$inner_compute$c0"
        if bias_const_name not in instances:
            instances[bias_const_name] = {
                "genref": "coreir.const",
                "genargs": {"width": ["Int", 84]},
                "modargs": {
                    "value": [["BitVector", 84], self.FP_ADD_INSTR]
                },
            }

        bias_add_name = "op_hcompute_bias_add$inner_compute$float_DW_fp_add_bias"
        if bias_add_name not in instances:
            instances[bias_add_name] = {"modref": "global.PE"}

        def add_conn_once(src, dst):
            pair = [src, dst]
            if pair not in conns:
                conns.append(pair)

        # Find and remove the current final_reduce -> IO connection
        final_to_io_pairs = []
        for idx, (left, right) in enumerate(list(conns)):
            is_final_left = left == f"{final_reduce_pe_name}.O0" and right == f"{out_io_name}.in"
            is_final_right = right == f"{final_reduce_pe_name}.O0" and left == f"{out_io_name}.in"
            if is_final_left or is_final_right:
                final_to_io_pairs.append(idx)
        for idx in reversed(final_to_io_pairs):
            conns.pop(idx)

        # Insert new connections:
        # final_reduce_pe.O0 -> bias_add.data1
        # bias_io.out -> bias_add.data0
        # bias_add.O0 -> io.in
        add_conn_once(f"{bias_add_name}.inst", f"{bias_const_name}.out")
        add_conn_once(f"{bias_add_name}.data0", f"{bias_io_name}.out")
        add_conn_once(f"{bias_add_name}.data1", f"{final_reduce_pe_name}.O0")
        add_conn_once(f"{bias_add_name}.O0", f"{out_io_name}.in")

        # Overwrite the JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

        # Update design_meta to include bias
        design_meta_path = os.path.join(bin_path, "design_meta_halide.json")
        with open(design_meta_path, "r") as f:
            design_meta = json.load(f)

        # Add bias tensor description
        if not any(x.get("name") == "bias_host_stencil" for x in design_meta["IOs"]["inputs"]):
            design_meta["IOs"]["inputs"].append(
                {
                    "bitwidth": 16,
                    "datafile": "bias_host_stencil.raw",
                    "name": "bias_host_stencil",
                    "shape": [int(self.halide_gen_args_dict["matrix_height"])],
                }
            )

        with open(design_meta_path, "w") as f:
            json.dump(design_meta, f, indent=2)

    def hack_for_scalar_max_fp_rv(self, json_path, bin_path):

        with open(json_path, "r") as f:
            design = json.load(f)

        top_module = "scalar_max_fp"
        tree_out_pond = "tree_3_stencil$ub_tree_3_stencil_BANK_0_garnet"
        tree_out_pond_clk = "tree_3_stencil$ub_tree_3_stencil_BANK_0_clk_en_const"

        # Const PE instance to remove
        pe_to_remove = "op_hcompute_max_output_cgra_inner_stencil$inner_compute$const"

        # Locate "scalar_max_fp" module
        global_modules = design["namespaces"]["global"]["modules"]
        if top_module not in global_modules:
            print(f"WARNING: Module '{top_module}' not found in design. No hack applied.")
            return
        scalar_max_fp = global_modules[top_module]

        # Remove the tile instance and its clk_en_const
        instance_dict = scalar_max_fp.get("instances", {})
        if tree_out_pond in instance_dict:
            del instance_dict[tree_out_pond]
        if tree_out_pond_clk in instance_dict:
            del instance_dict[tree_out_pond_clk]

        # Remove references to the tile from connections,
        #    capturing upstream => old_input, downstream => old_output
        old_input = None
        old_output = None
        new_connections = []

        connection_list = scalar_max_fp.get("connections", [])
        for conn in connection_list:
            left, right = conn[0], conn[1]

            # Check references to tile or its clk const
            refs_tile_left = (tree_out_pond in left) or (tree_out_pond_clk in left)
            refs_tile_right = (tree_out_pond in right) or (tree_out_pond_clk in right)

            if not (refs_tile_left or refs_tile_right):
                # If no reference to tile, keep it for now
                new_connections.append(conn)
                continue

            # If referencing the tile, skip it but see if it's data_in or data_out
            if tree_out_pond in left:
                if ".data_in_pond_" in left or ".data_in_" in left:
                    old_input = right
            elif tree_out_pond in right:
                if ".data_in_pond_" in right or ".data_in_" in right:
                    old_input = left

            if tree_out_pond in left:
                if ".data_out_pond_" in left or ".data_out_" in left:
                    old_output = right
            elif tree_out_pond in right:
                if ".data_out_pond_" in right or ".data_out_" in right:
                    old_output = left

        # Reconnect old_input -> old_output if both exist
        if old_input and old_output:
            new_connections.append([old_input, old_output])
        else:
            print(
                f"WARNING: Could not find both data_in and data_out references for "
                f"tile '{tree_out_pond}'. No direct reconnection added."
            )

        # Remove the PE instance "op_hcompute_output_cgra_stencil$inner_compute$const_"
        #    from the instance dictionary (if present)
        if pe_to_remove in instance_dict:
            del instance_dict[pe_to_remove]

        # Filter out any connections referencing const PE instance
        final_connections = []
        for conn in new_connections:
            left, right = conn[0], conn[1]
            if pe_to_remove in left or pe_to_remove in right:
                # skip any references to that PE
                continue
            final_connections.append(conn)

        # Update the final connection list
        scalar_max_fp["connections"] = final_connections

        # Identify psum PE with Pond and generate fifo bypass config
        # Write bypass config to design_top at the top level
        psum_pe_name = "op_hcompute_max_output_cgra_inner_stencil_1$inner_compute$float_max_"
        PE_fifos_bypass_config = {
            psum_pe_name: {
                # HACK: Assuming the PE input port connected to Pond is "data_in_0"
                # We bypass the input and output fifos on the feedback path
                "input_fifo_bypass": [1, 0, 0],
                "output_fifo_bypass": 1,
            }
        }
        PE_fifos_bypass_config_path = os.path.join(bin_path, "PE_fifos_bypass_config.json")
        print(f"Writing PE_fifos_bypass_config to {PE_fifos_bypass_config_path}")
        with open(PE_fifos_bypass_config_path, "w") as f:
            json.dump(PE_fifos_bypass_config, f, indent=2)

        # Update output IO tile extent
        # HACK: Set extent to 2 (should set this to 1 since we only have 1 output, but then can't get f2g interrupt)
        # Also have to update cycle_start_addr and stride, as it means handshake rather than cycles in dense rv scenario
        io_out_name = "io16_hw_output_stencil_op_hcompute_hw_output_stencil_write_0"
        instance_dict[io_out_name]["metadata"]["in2glb_0"]["extent"] = [2]
        # We want to accept every valid handshake starting from the first one
        instance_dict[io_out_name]["metadata"]["in2glb_0"]["cycle_starting_addr"] = [0]
        instance_dict[io_out_name]["metadata"]["in2glb_0"]["cycle_stride"] = [1]

        # Overwrite the JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

    def hack_for_stable_softmax_pass1_fp_rv(self, json_path, bin_path):
        self.vector_reduction_tree_helper(
            json_path,
            bin_path,
            top_module="stable_softmax_pass1_fp",
            tree_end_pe_name="op_hcompute_max_output_cgra_inner_stencil_1$inner_compute$float_max_",
            old_accum_pond_name="max_output_cgra_inner_stencil$ub_max_output_cgra_inner_stencil_BANK_0_garnet",
            old_accum_pond_clk_name="max_output_cgra_inner_stencil$ub_max_output_cgra_inner_stencil_BANK_0_clk_en_const",
            old_const_feeder_pe_name="op_hcompute_max_output_cgra_inner_stencil$inner_compute$const_i3133_i631",
        )

    def hack_for_stable_softmax_pass2_fp_rv(self, json_path, bin_path):
        '''
        This hack is used to remove the redundant vector max IO instances and keep only the first one.
        It also hacks the kept vector max IO metadata to set the extent and read_data_stride.
        '''

        with open(json_path, "r") as f:
            design = json.load(f)

        top_module = "stable_softmax_pass2_fp"

        # Locate the top module
        global_modules = design["namespaces"]["global"]["modules"]
        if top_module not in global_modules:
            print(f"WARNING: Module '{top_module}' not found in design. No hack applied.")
            return
        stable_softmax_pass2_fp = global_modules[top_module]

        instance_dict = stable_softmax_pass2_fp.get("instances", {})

        # Extract halide args
        vec_len = int(self.halide_gen_args_dict["vec_width"])
        num_vecs = int(self.halide_gen_args_dict["vec_height"])
        glb_i = int(self.halide_gen_args_dict["glb_i"])

        # Find all "io16in_vec_max_host_stencil_" IO instances
        max_io_prefix = "io16in_vec_max_host_stencil_clkwrk_"
        all_inst = list(instance_dict.keys())
        vecmax_insts = [inst for inst in all_inst if inst.startswith(max_io_prefix)]

        if len(vecmax_insts) == 0:
            raise ValueError(f"No '{max_io_prefix}' IO instances found. No hack applied.")

        # Sort by the numeric index after "clkwrk_"
        def extract_clkwrk_index(inst_name):
            # Extract number after "clkwrk_" (e.g., "io16in_vec_max_host_stencil_clkwrk_4_..." -> 4)
            match = re.search(r"clkwrk_(\d+)_", inst_name)
            if match:
                return int(match.group(1))
            return 0  # fallback if pattern doesn't match

        vecmax_insts.sort(key=extract_clkwrk_index)
        keep_inst = vecmax_insts[0]
        remove_insts = vecmax_insts[1:]

        # Remove other input IO instances from instance_dict
        for r in remove_insts:
            del instance_dict[r]

        # Update the type record to remove fields for removed instances
        type_list = stable_softmax_pass2_fp.get("type", ["Record", []])[1]

        def inst_to_field_prefix(name):
            if name.startswith("io16in_"):
                return name[len("io16in_"):]
            return name

        keep_prefix = inst_to_field_prefix(keep_inst)
        remove_prefixes = [inst_to_field_prefix(r) for r in remove_insts]

        new_type_list = []
        for field_pair in type_list:
            field_name, field_type = field_pair
            if "vec_max_host_stencil_clkwrk_" not in field_name:
                # Not a vecmax field => keep
                new_type_list.append(field_pair)
                continue
            # It's a vecmax field => keep only if it belongs to keep_prefix
            if field_name.startswith(keep_prefix):
                new_type_list.append(field_pair)
            # else skip (removed instance)

        stable_softmax_pass2_fp["type"][1] = new_type_list

        # Fix up connections: broadcast keep_inst to all PEs that were connected to removed insts
        connection_list = stable_softmax_pass2_fp.get("connections", [])
        new_conn = []
        for left, right in connection_list:
            new_left, new_right = left, right

            # If the removed instance name is in the path, replace with keep_inst
            for rinst in remove_insts:
                if rinst in new_left:
                    new_left = new_left.replace(rinst, keep_inst)
                if rinst in new_right:
                    new_right = new_right.replace(rinst, keep_inst)

            # If the removed field prefix is in the path, replace with keep_prefix
            for rpref in remove_prefixes:
                if rpref in new_left:
                    new_left = new_left.replace(rpref, keep_prefix)
                if rpref in new_right:
                    new_right = new_right.replace(rpref, keep_prefix)

            new_conn.append([new_left, new_right])

        stable_softmax_pass2_fp["connections"] = new_conn

        # Deduplicate connections
        final_dedup = []
        seen = set()
        for c in stable_softmax_pass2_fp["connections"]:
            t = tuple(c)
            if t not in seen:
                seen.add(t)
                final_dedup.append(c)

        stable_softmax_pass2_fp["connections"] = final_dedup

        # Hack the kept vector max IO metadata
        if keep_inst in instance_dict:
            io_inst = instance_dict[keep_inst]
            if "metadata" in io_inst and "glb2out_0" in io_inst["metadata"]:
                metadata = io_inst["metadata"]["glb2out_0"]
                metadata["cycle_starting_addr"] = [0]
                metadata["cycle_stride"] = [1, 1]
                metadata["dimensionality"] = 2
                metadata["extent"] = [vec_len // glb_i, num_vecs]
                metadata["read_data_starting_addr"] = [0]
                metadata["read_data_stride"] = [0, 1]

        # Overwrite the JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

        # Hack design_meta_halide.json to change vec_max_host_stencil shape to [num_vecs]
        design_meta_halide_path = os.path.join(bin_path, "design_meta_halide.json")
        if os.path.exists(design_meta_halide_path):
            with open(design_meta_halide_path, "r") as f:
                design_meta_halide = json.load(f)

            # Update vec_max_host_stencil shape
            for inp in design_meta_halide.get("IOs", {}).get("inputs", []):
                if inp.get("name") == "vec_max_host_stencil":
                    inp["shape"] = [num_vecs]
                    break

            with open(design_meta_halide_path, "w") as f:
                json.dump(design_meta_halide, f, indent=2)

    def _hack_reduction_followed_by_elementwise_rv(self, json_path, bin_path, top_module, scalar_op_type, elementwise_op_type):
        """
        Common function for restructuring designs with reduction followed by elementwise operations.

        Args:
            json_path: Path to design JSON
            bin_path: Path to bin directory
            top_module: Name of the top module
            scalar_op_type: "division" or "fp_mul" - the scalar operation on reduced result
            elementwise_op_type: "fp_mul" or "fp_add" - the elementwise operation with input
        """
        with open(json_path, "r") as f:
            design = json.load(f)

        global_modules = design["namespaces"]["global"]["modules"]
        if top_module not in global_modules:
            raise RuntimeError(f"[ERROR] Module '{top_module}' not found in design.")

        top_module_json = global_modules[top_module]
        instance_dict = top_module_json.get("instances", {})
        original_connections = top_module_json.get("connections", [])

        # Determine number of tree stages
        tree_stages = int(self.halide_gen_args_dict["tree_stages"])

        # Identify instances to remove
        # 1. Remove tree intermediate mem tiles for all stages
        tree_mem_tiles = []
        tree_clk_consts = []
        for inst_name in list(instance_dict.keys()):
            # Check if this is a tree mem tile for any stage
            for stage in range(1, tree_stages + 1):
                if f"tree_{stage}_stencil$ub_tree_{stage}_stencil_BANK" in inst_name:
                    if "garnet" in inst_name:
                        tree_mem_tiles.append(inst_name)
                    elif "clk_en_const" in inst_name:
                        tree_clk_consts.append(inst_name)
                    break

        # 2. Remove output_cgra_stencil MEM instances EXCEPT BANK_0
        output_cgra_stencil_mems_to_remove = []
        output_cgra_stencil_clks_to_remove = []
        output_cgra_stencil_mem_main = None

        for inst_name in list(instance_dict.keys()):
            if "output_cgra_stencil$ub_output_cgra_stencil_BANK" in inst_name:
                if "BANK_0" in inst_name and "garnet" in inst_name:
                    output_cgra_stencil_mem_main = inst_name
                elif "garnet" in inst_name:
                    output_cgra_stencil_mems_to_remove.append(inst_name)
                elif "clk_en_const" in inst_name:
                     # If it's not for BANK_0, remove it
                    if "BANK_0" not in inst_name:
                        output_cgra_stencil_clks_to_remove.append(inst_name)

        # 3. Remove old single accumulator PE and const feeder
        old_const_feeder_candidates = []
        old_single_accum_pe_candidates = []

        for inst_name in instance_dict:
            if "op_hcompute_sum_cgra_stencil" in inst_name:
                if "inner_compute$const" in inst_name:
                    old_const_feeder_candidates.append(inst_name)
                if "float_DW_fp_add" in inst_name:
                    old_single_accum_pe_candidates.append(inst_name)

        # Find sum_cgra_stencil MEM instances to remove (we'll connect final_reduce_pe directly to scalar operation)
        sum_mem_name = None
        sum_mem_clk_name = None
        sum_cgra_stencil_ponds_to_remove = []
        sum_cgra_stencil_clks_to_remove = []

        for inst_name in instance_dict:
            if "sum_cgra_stencil$ub_sum_cgra_stencil_BANK" in inst_name and "garnet" in inst_name:
                # Remove all sum_cgra_stencil MEM instances
                if "BANK_0" in inst_name:
                    sum_mem_name = inst_name
                    sum_mem_clk_name = inst_name.replace("_garnet", "_clk_en_const")
                sum_cgra_stencil_ponds_to_remove.append(inst_name)
                clk_name = inst_name.replace("_garnet", "_clk_en_const")
                if clk_name in instance_dict:
                    sum_cgra_stencil_clks_to_remove.append(clk_name)

        # Collect all instances to remove
        instances_to_remove = set(tree_mem_tiles + tree_clk_consts)
        instances_to_remove.update(sum_cgra_stencil_ponds_to_remove)
        instances_to_remove.update(sum_cgra_stencil_clks_to_remove)
        if sum_mem_name:
            instances_to_remove.add(sum_mem_name)
        if sum_mem_clk_name and sum_mem_clk_name in instance_dict:
            instances_to_remove.add(sum_mem_clk_name)
        instances_to_remove.update(output_cgra_stencil_mems_to_remove)
        instances_to_remove.update(output_cgra_stencil_clks_to_remove)
        instances_to_remove.update(old_const_feeder_candidates)
        instances_to_remove.update(old_single_accum_pe_candidates)

        # Remove instances
        for inst_name in instances_to_remove:
            if inst_name in instance_dict:
                del instance_dict[inst_name]

        # Find the final tree stage PE
        final_tree_stage = tree_stages
        final_tree_pe_name = None

        candidate_pes = []
        for inst_name in instance_dict:
            if f"op_hcompute_tree_{final_tree_stage}_stencil" in inst_name and "float_DW_fp_add" in inst_name:
                candidate_pes.append(inst_name)

        if candidate_pes:
            final_tree_pe_name = candidate_pes[0]

        if final_tree_pe_name is None:
            raise RuntimeError(f"[ERROR] Could not find tree_{final_tree_stage}_stencil PE")

        final_tree_pe_output = f"{final_tree_pe_name}.O0"

        # Create new instances for dual accumulator schedule
        shared_clk_const_name = f"{top_module}_clk_en_const"
        filter_mem_name = f"{top_module}_filter_mem"
        accum_pond_0_name = f"{top_module}_accum_pond_0"
        accum_pond_1_name = f"{top_module}_accum_pond_1"
        accum_pe_0_name = f"{top_module}_accum_pe_0"
        accum_pe_1_name = f"{top_module}_accum_pe_1"
        final_reduce_pe_name = f"{top_module}_final_reduce_pe"
        accum_pe_0_inst_const_name = f"{top_module}_accum_pe_0_inst_const"
        accum_pe_1_inst_const_name = f"{top_module}_accum_pe_1_inst_const"
        final_reduce_pe_inst_const_name = f"{top_module}_final_reduce_pe_inst_const"

        # Add new instances
        if shared_clk_const_name not in instance_dict:
            instance_dict[shared_clk_const_name] = copy.deepcopy(self.const_clk_tpl)

        if filter_mem_name not in instance_dict:
            instance_dict[filter_mem_name] = copy.deepcopy(self.mem_tpl)

        if accum_pond_0_name not in instance_dict:
            instance_dict[accum_pond_0_name] = copy.deepcopy(self.pond_tpl)

        if accum_pond_1_name not in instance_dict:
            instance_dict[accum_pond_1_name] = copy.deepcopy(self.pond_tpl)

        if accum_pe_0_name not in instance_dict:
            instance_dict[accum_pe_0_name] = copy.deepcopy(self.pe_tpl)

        if accum_pe_1_name not in instance_dict:
            instance_dict[accum_pe_1_name] = copy.deepcopy(self.pe_tpl)

        if final_reduce_pe_name not in instance_dict:
            instance_dict[final_reduce_pe_name] = copy.deepcopy(self.pe_tpl)

        # Find PE instruction from final tree stage PE
        final_tree_pe_inst_name = f"{final_tree_pe_name.replace('$inner_compute$float_DW_fp_add', '$inner_compute$c0')}"
        if final_tree_pe_inst_name not in instance_dict:
             for k in instance_dict:
                 if f"op_hcompute_tree_{final_tree_stage}_stencil" in k and "$inner_compute$c0" in k:
                     final_tree_pe_inst_name = k
                     break

        if final_tree_pe_inst_name in instance_dict:
            pe_inst_source = instance_dict[final_tree_pe_inst_name]

            if accum_pe_0_inst_const_name not in instance_dict:
                instance_dict[accum_pe_0_inst_const_name] = copy.deepcopy(pe_inst_source)
            if accum_pe_1_inst_const_name not in instance_dict:
                instance_dict[accum_pe_1_inst_const_name] = copy.deepcopy(pe_inst_source)
            if final_reduce_pe_inst_const_name not in instance_dict:
                instance_dict[final_reduce_pe_inst_const_name] = copy.deepcopy(pe_inst_source)
        else:
             raise RuntimeError(f"[ERROR] Could not find instruction node for tree PE")

        # Build new connections
        connections = []

        def add_conn_once(src, dst):
            pair = [src, dst]
            if pair not in connections:
                connections.append(pair)

        # Reconnect all consecutive tree stages
        for stage_i in range(1, tree_stages):
            stage_i_plus_1 = stage_i + 1

            tree_i_plus_1_pe_names = [k for k in instance_dict if f"op_hcompute_tree_{stage_i_plus_1}_stencil" in k and "float_DW_fp_add" in k]
            current_stage_mems = [m for m in tree_mem_tiles if f"tree_{stage_i}_stencil" in m]

            for tree_mem in current_stage_mems:
                upstream_port = None
                for l, r in original_connections:
                    if tree_mem in l and ".data_in" in l: upstream_port = r
                    elif tree_mem in r and ".data_in" in r: upstream_port = l

                if not upstream_port: continue

                mem_outputs = []
                for l, r in original_connections:
                    if tree_mem in l and ".data_out" in l: mem_outputs.append(r)
                    elif tree_mem in r and ".data_out" in r: mem_outputs.append(l)

                for out_port in mem_outputs:
                    for pe in tree_i_plus_1_pe_names:
                        if pe in out_port:
                            add_conn_once(upstream_port, out_port)

        # Filter out connections involving removed instances
        for left, right in original_connections:
            if any(rem in left or rem in right for rem in instances_to_remove):
                continue

            if any(tree_mem in left or tree_mem in right for tree_mem in tree_mem_tiles):
                continue

            # Skip direct connections from IO to elementwise operation (to be replaced by IO -> MEM -> elementwise)
            elementwise_op_pattern = f"float_DW_{elementwise_op_type}"
            if "io16in_input_host_stencil" in left and "op_hcompute_output_glb_stencil" in right and elementwise_op_pattern in right:
                continue
            if "io16in_input_host_stencil" in right and "op_hcompute_output_glb_stencil" in left and elementwise_op_pattern in left:
                continue

            # Skip connections from output_cgra_stencil_mem (broadcast logic will handle this)
            if output_cgra_stencil_mem_main:
                if output_cgra_stencil_mem_main in left or output_cgra_stencil_mem_main in right:
                     if ".data_out" in left or ".data_out" in right:
                         continue

            # Skip connections involving sum_cgra_stencil MEM (we're removing it and connecting directly)
            if sum_mem_name and (sum_mem_name in left or sum_mem_name in right):
                continue
            if sum_mem_clk_name and (sum_mem_clk_name in left or sum_mem_clk_name in right):
                continue

            # Skip connections from elementwise PEs to output IOs
            if "op_hcompute_output_glb_stencil" in left and elementwise_op_pattern in left and "io16_hw_output_stencil" in right:
                continue
            if "op_hcompute_output_glb_stencil" in right and elementwise_op_pattern in right and "io16_hw_output_stencil" in left:
                continue

            # Skip all connections from elementwise PEs to intermediate PE inputs
            # We will add the correct connections later based on input IO -> output IO index matching
            # Intermediate PEs are other PEs (like fp_add) that are NOT elementwise PEs
            elementwise_pe_in_left = "op_hcompute_output_glb_stencil" in left and elementwise_op_pattern in left and ".O0" in left
            intermediate_pe_in_right = ("op_hcompute_output_glb_stencil" in right and
                                       elementwise_op_pattern not in right and  # Not an elementwise PE
                                       any(f".data{i}" in right for i in range(3)))

            elementwise_pe_in_right = "op_hcompute_output_glb_stencil" in right and elementwise_op_pattern in right and ".O0" in right
            intermediate_pe_in_left = ("op_hcompute_output_glb_stencil" in left and
                                      elementwise_op_pattern not in left and  # Not an elementwise PE
                                      any(f".data{i}" in left for i in range(3)))

            # Skip all elementwise PE -> intermediate PE connections and we'll add correct ones later
            if (elementwise_pe_in_left and intermediate_pe_in_right) or (elementwise_pe_in_right and intermediate_pe_in_left):
                continue

            # Skip connections between elementwise PEs (they should connect directly to output IOs, not to each other)
            # This prevents cycles when elementwise PEs connect directly to outputs (e.g., layer_norm_pass1)
            both_are_elementwise = (elementwise_pe_in_left and elementwise_pe_in_right)
            if both_are_elementwise:
                continue

            add_conn_once(left, right)


        # New connections for dual accumulator schedule
        add_conn_once(final_tree_pe_output, f"{filter_mem_name}.data_in_0")

        add_conn_once(f"{shared_clk_const_name}.out", f"{filter_mem_name}.clk_en")
        add_conn_once(f"{shared_clk_const_name}.out", f"{accum_pond_0_name}.clk_en")
        add_conn_once(f"{shared_clk_const_name}.out", f"{accum_pond_1_name}.clk_en")

        add_conn_once(f"{filter_mem_name}.data_out_0", f"{accum_pe_0_name}.data1")
        add_conn_once(f"{filter_mem_name}.data_out_1", f"{accum_pe_1_name}.data1")

        add_conn_once(f"{accum_pe_0_inst_const_name}.out", f"{accum_pe_0_name}.inst")
        add_conn_once(f"{accum_pe_1_inst_const_name}.out", f"{accum_pe_1_name}.inst")
        add_conn_once(f"{final_reduce_pe_inst_const_name}.out", f"{final_reduce_pe_name}.inst")

        add_conn_once(f"{accum_pond_0_name}.data_out_pond_0", f"{accum_pe_0_name}.data0")
        add_conn_once(f"{accum_pond_0_name}.data_in_pond_0", f"{accum_pe_0_name}.O0")
        add_conn_once(f"{accum_pond_0_name}.data_out_pond_1", f"{final_reduce_pe_name}.data0")

        add_conn_once(f"{accum_pond_1_name}.data_out_pond_0", f"{accum_pe_1_name}.data0")
        add_conn_once(f"{accum_pond_1_name}.data_in_pond_0", f"{accum_pe_1_name}.O0")
        add_conn_once(f"{accum_pond_1_name}.data_out_pond_1", f"{final_reduce_pe_name}.data1")

        # Find PEs that were originally connected to sum buffer (to avoid redundant connections
        # when there are multiple PEs with the same instruction in a long pipeline)
        # Dictionary maps PE name -> set of ports that were connected to sum buffer
        pes_connected_to_sum_buffer = {}
        if sum_mem_name:
            for left, right in original_connections:
                if sum_mem_name in left and ".data_out" in left:
                    if "op_hcompute_output_cgra_stencil" in right:
                        port_match = re.search(r"\.(data\d+)", right)
                        if port_match:
                            pe_name = right.rsplit(".", 1)[0]
                            port = port_match.group(1)
                            if pe_name not in pes_connected_to_sum_buffer:
                                pes_connected_to_sum_buffer[pe_name] = set()
                            pes_connected_to_sum_buffer[pe_name].add(port)
                elif sum_mem_name in right and ".data_out" in right:
                    if "op_hcompute_output_cgra_stencil" in left:
                        port_match = re.search(r"\.(data\d+)", left)
                        if port_match:
                            pe_name = left.rsplit(".", 1)[0]
                            port = port_match.group(1)
                            if pe_name not in pes_connected_to_sum_buffer:
                                pes_connected_to_sum_buffer[pe_name] = set()
                            pes_connected_to_sum_buffer[pe_name].add(port)

        for sum_pond_name in sum_cgra_stencil_ponds_to_remove:
            for left, right in original_connections:
                if sum_pond_name in left and ".data_out_pond" in left:
                    if "op_hcompute_output_cgra_stencil" in right:
                        port_match = re.search(r"\.(data\d+)", right)
                        if port_match:
                            pe_name = right.rsplit(".", 1)[0]
                            port = port_match.group(1)
                            if pe_name not in pes_connected_to_sum_buffer:
                                pes_connected_to_sum_buffer[pe_name] = set()
                            pes_connected_to_sum_buffer[pe_name].add(port)
                elif sum_pond_name in right and ".data_out_pond" in right:
                    if "op_hcompute_output_cgra_stencil" in left:
                        port_match = re.search(r"\.(data\d+)", left)
                        if port_match:
                            pe_name = left.rsplit(".", 1)[0]
                            port = port_match.group(1)
                            if pe_name not in pes_connected_to_sum_buffer:
                                pes_connected_to_sum_buffer[pe_name] = set()
                            pes_connected_to_sum_buffer[pe_name].add(port)

        # Connect final_reduce_pe directly to scalar operation (bypassing sum buffer MEM)
        if scalar_op_type == "division":
            get_mant_pe_name = None
            sub_exp_pe_name = None
            for inst_name in instance_dict:
                if "op_hcompute_output_cgra_stencil" in inst_name and "fp_getmant" in inst_name:
                    if inst_name in pes_connected_to_sum_buffer:
                        get_mant_pe_name = inst_name
                elif "op_hcompute_output_cgra_stencil" in inst_name and "fp_subexp" in inst_name:
                    if inst_name in pes_connected_to_sum_buffer:
                        sub_exp_pe_name = inst_name

            if get_mant_pe_name:
                # Use the port that was originally connected (typically one port per PE)
                target_ports = pes_connected_to_sum_buffer[get_mant_pe_name]
                if len(target_ports) > 1:
                    raise RuntimeError(f"PE {get_mant_pe_name} has multiple ports connected to sum buffer: {target_ports}")
                target_port = next(iter(target_ports))
                add_conn_once(f"{final_reduce_pe_name}.O0", f"{get_mant_pe_name}.{target_port}")
            if sub_exp_pe_name:
                target_ports = pes_connected_to_sum_buffer[sub_exp_pe_name]
                if len(target_ports) > 1:
                    raise RuntimeError(f"PE {sub_exp_pe_name} has multiple ports connected to sum buffer: {target_ports}")
                target_port = next(iter(target_ports))
                add_conn_once(f"{final_reduce_pe_name}.O0", f"{sub_exp_pe_name}.{target_port}")

        elif scalar_op_type == "log":
            get_mant_pe_name = None
            exp2f_pe_name = None
            for inst_name in instance_dict:
                if "op_hcompute_output_cgra_stencil" in inst_name and "fp_getmant" in inst_name:
                    if inst_name in pes_connected_to_sum_buffer:
                        get_mant_pe_name = inst_name
                elif "op_hcompute_output_cgra_stencil" in inst_name and "fp_cnvexp2f" in inst_name:
                    if inst_name in pes_connected_to_sum_buffer:
                        exp2f_pe_name = inst_name

            if get_mant_pe_name:
                target_ports = pes_connected_to_sum_buffer[get_mant_pe_name]
                if len(target_ports) > 1:
                    raise RuntimeError(f"PE {get_mant_pe_name} has multiple ports connected to sum buffer: {target_ports}")
                target_port = next(iter(target_ports))
                add_conn_once(f"{final_reduce_pe_name}.O0", f"{get_mant_pe_name}.{target_port}")
            if exp2f_pe_name:
                target_ports = pes_connected_to_sum_buffer[exp2f_pe_name]
                if len(target_ports) > 1:
                    raise RuntimeError(f"PE {exp2f_pe_name} has multiple ports connected to sum buffer: {target_ports}")
                target_port = next(iter(target_ports))
                add_conn_once(f"{final_reduce_pe_name}.O0", f"{exp2f_pe_name}.{target_port}")

        elif scalar_op_type == "fp_mul":
            scalar_fp_mul_pe_name = None
            for inst_name in instance_dict:
                if "op_hcompute_output_cgra_stencil" in inst_name and "float_DW_fp_mul" in inst_name:
                    if inst_name in pes_connected_to_sum_buffer:
                        scalar_fp_mul_pe_name = inst_name
                        break

            if scalar_fp_mul_pe_name:
                target_ports = pes_connected_to_sum_buffer[scalar_fp_mul_pe_name]
                if len(target_ports) > 1:
                    raise RuntimeError(f"PE {scalar_fp_mul_pe_name} has multiple ports connected to sum buffer: {target_ports}")
                target_port = next(iter(target_ports))
                add_conn_once(f"{final_reduce_pe_name}.O0", f"{scalar_fp_mul_pe_name}.{target_port}")

        else:
            raise ValueError(f"Unsupported scalar operation type: {scalar_op_type}")

        # Find all elementwise operation PEs dynamically
        elementwise_op_pattern = f"float_DW_{elementwise_op_type}"
        elementwise_pe_instances = []
        for inst_name in instance_dict:
            if "op_hcompute_output_glb_stencil" in inst_name and elementwise_op_pattern in inst_name:
                elementwise_pe_instances.append(inst_name)

        # Extract lane indices from elementwise PE names
        # Pattern: op_hcompute_output_glb_stencil[_<idx>]$inner_compute$float_DW_<op>_...
        def extract_lane_idx(pe_name):
            # Remove the prefix and suffix
            if "op_hcompute_output_glb_stencil$" in pe_name:
                return 0  # No suffix means lane 0
            elif "op_hcompute_output_glb_stencil_" in pe_name:
                # Extract number after the underscore
                match = re.search(r"op_hcompute_output_glb_stencil_(\d+)\$", pe_name)
                if match:
                    return int(match.group(1))
            return None

        # Build mapping of lane_idx -> elementwise_pe_name
        lane_to_elementwise = {}
        for pe_name in elementwise_pe_instances:
            lane_idx = extract_lane_idx(pe_name)
            if lane_idx is not None:
                lane_to_elementwise[lane_idx] = pe_name

        # Find all tile_input_stencil MEMs
        tile_input_mems = {}
        for inst_name in instance_dict:
            if "tile_input_stencil$ub_tile_input_stencil_BANK" in inst_name and "garnet" in inst_name:
                # Extract BANK index
                match = re.search(r"BANK_(\d+)_garnet", inst_name)
                if match:
                    bank_idx = int(match.group(1))
                    tile_input_mems[bank_idx] = inst_name

        # Connect each elementwise PE to its corresponding tile_input_stencil MEM
        # Based on the pattern: BANK_<lane_idx> -> op_hcompute_output_glb_stencil[_<lane_idx>]
        for lane_idx, elementwise_pe_name in sorted(lane_to_elementwise.items()):
            # Connect BANK_<lane_idx> to elementwise PE for lane <lane_idx>
            if lane_idx in tile_input_mems:
                tile_input_mem_name = tile_input_mems[lane_idx]
                add_conn_once(f"{tile_input_mem_name}.data_out_1", f"{elementwise_pe_name}.data0")

        # Broadcast Scalar Operation Output (MEM) -> Pipeline FIFOs -> All elementwise PEs
        if output_cgra_stencil_mem_main:
            # Broadcast from MEM to all elementwise PEs
            for elementwise_pe_name in elementwise_pe_instances:
                add_conn_once(f"{output_cgra_stencil_mem_main}.data_out_0", f"{elementwise_pe_name}.data1")

        # Connect elementwise PEs to output IOs with matching indexes
        # Extract stencil indexes from input IO instances
        def extract_io_idx(io_name, is_input):
            """Extract stencil index from IO instance name.
            Input IOs use stencil_<idx>_read or stencil_<idx>
            Output IOs use stencil_<idx>_write or stencil_<idx>
            """
            if is_input:
                match = re.search(r"stencil_(\d+)_read", io_name)
            else:
                match = re.search(r"stencil_(\d+)_write", io_name)
            if not match:
                match = re.search(r"stencil_(\d+)", io_name)
            return int(match.group(1)) if match else 0

        # Find all input IO instances and map by stencil index
        input_io_by_idx = {}
        output_io_by_idx = {}

        for inst_name, inst_config in instance_dict.items():
            if inst_config.get("modref") != "global.IO":
                continue

            if "io16in_input_host_stencil" in inst_name:
                idx = extract_io_idx(inst_name, is_input=True)
                input_io_by_idx[idx] = inst_name
            elif "io16_hw_output_stencil" in inst_name:
                idx = extract_io_idx(inst_name, is_input=False)
                output_io_by_idx[idx] = inst_name

        # Build mapping from input IO index -> BANK index from original connections
        input_io_idx_to_bank_idx = {}

        # Helper function to trace backwards from a port to find the input IO
        def trace_to_input_io(port_name, visited=None):
            """
            Trace backwards from a port through PEs to find the input IO.
            Returns the input IO name if found, None otherwise.
            """
            if visited is None:
                visited = set()

            # Avoid infinite loops
            if port_name in visited:
                return None
            visited.add(port_name)

            # Check if this port is directly from an input IO
            if "io16in_input_host_stencil" in port_name and ".out" in port_name:
                return port_name.split(".")[0]

            # If port_name is a PE output (.O0), we need to trace backwards through the PE's inputs
            # instead of looking for connections involving the output port
            if ".O0" in port_name and "op_hcompute" in port_name:
                pe_instance = port_name.split(".")[0]
                # Trace back through the PE's inputs (data0, data1, data2)
                for pe_input_idx in range(3):
                    pe_input_port = f"{pe_instance}.data{pe_input_idx}"
                    result = trace_to_input_io(pe_input_port, visited)
                    if result:
                        return result
                return None

            # Find what connects to this port (tracing backwards)
            # Connections are [source, destination], so we look for where port_name appears
            for left, right in original_connections:
                upstream_port = None

                # Check if port_name is the destination (right side)
                if port_name == right:
                    upstream_port = left
                # Check if port_name is the source (left side) - for bidirectional connections
                elif port_name == left:
                    upstream_port = right
                else:
                    continue

                upstream_instance = upstream_port.split(".")[0]

                # If upstream is an input IO, we found it
                if "io16in_input_host_stencil" in upstream_instance and ".out" in upstream_port:
                    return upstream_instance

                # If upstream is a PE output (.O0), trace back through the PE's inputs
                if ".O0" in upstream_port and "op_hcompute" in upstream_instance:
                    # This is a PE output, trace back through its inputs (data0, data1, data2)
                    for pe_input_idx in range(3):
                        pe_input_port = f"{upstream_instance}.data{pe_input_idx}"
                        result = trace_to_input_io(pe_input_port, visited)
                        if result:
                            return result

            return None

        # First pass: Look for direct connections tile_input_stencil$ub_tile_input_stencil_BANK_X.data_in_0 <-> io16in_input_host_stencil_..._stencil_Y_read_0.out
        for left, right in original_connections:
            # Look for connections tile_input_stencil$ub_tile_input_stencil_BANK_X.data_in_0 <-> io16in_input_host_stencil_..._stencil_Y_read_0.out
            bank_match = None
            input_io_name = None

            if "tile_input_stencil$ub_tile_input_stencil_BANK" in left and ".data_in_0" in left:
                bank_match = re.search(r"BANK_(\d+)_garnet", left)
                if "io16in_input_host_stencil" in right and ".out" in right:
                    input_io_name = right.split(".")[0]
            elif "tile_input_stencil$ub_tile_input_stencil_BANK" in right and ".data_in_0" in right:
                bank_match = re.search(r"BANK_(\d+)_garnet", right)
                if "io16in_input_host_stencil" in left and ".out" in left:
                    input_io_name = left.split(".")[0]

            if bank_match and input_io_name:
                bank_idx = int(bank_match.group(1))
                input_io_idx = extract_io_idx(input_io_name, is_input=True)
                input_io_idx_to_bank_idx[input_io_idx] = bank_idx

        # Second pass: Handle cases where MEM bank inputs come from PEs (not directly from input IOs)
        # Find all MEM banks that weren't matched in the first pass
        all_bank_indices = set()
        for left, right in original_connections:
            for side in [left, right]:
                if "tile_input_stencil$ub_tile_input_stencil_BANK" in side and ".data_in_0" in side:
                    bank_match = re.search(r"BANK_(\d+)_garnet", side)
                    if bank_match:
                        all_bank_indices.add(int(bank_match.group(1)))

        # For banks not yet mapped, trace backwards through PEs
        for bank_idx in all_bank_indices:
            if bank_idx in input_io_idx_to_bank_idx.values():
                continue  # Already mapped

            # Find the MEM bank input port
            mem_bank_input_port = None
            for left, right in original_connections:
                mem_bank_pattern = f"tile_input_stencil$ub_tile_input_stencil_BANK_{bank_idx}_garnet.data_in_0"
                if mem_bank_pattern in left:
                    mem_bank_input_port = right
                    break
                elif mem_bank_pattern in right:
                    mem_bank_input_port = left
                    break

            if mem_bank_input_port:
                # Trace backwards to find the input IO
                input_io_name = trace_to_input_io(mem_bank_input_port)
                if input_io_name:
                    input_io_idx = extract_io_idx(input_io_name, is_input=True)
                    input_io_idx_to_bank_idx[input_io_idx] = bank_idx

        # Error handling: Check if mapping is empty or incomplete
        if not input_io_idx_to_bank_idx:
            raise ValueError(
                "[ERROR]: Failed to build input IO index to BANK index mapping. "
                "No connections found between input IOs and tile_input_stencil MEM banks. "
                "This may indicate a design issue or unsupported connection pattern."
            )

        # Check if all expected banks are mapped
        mapped_banks = set(input_io_idx_to_bank_idx.values())
        if mapped_banks != all_bank_indices:
            missing_banks = all_bank_indices - mapped_banks
            raise ValueError(
                f"[ERROR]: Failed to map all MEM banks to input IOs. "
                f"Missing banks: {sorted(missing_banks)}. "
                f"Mapped banks: {sorted(mapped_banks)}. "
                f"This may indicate intermediate PEs that couldn't be traced back to input IOs."
            )

        # Connect elementwise PEs to output IOs based on input IO index matching
        # Path: input IO (index i) -> BANK (index j) -> elementwise (lane j) -> [intermediate PE] -> output IO (index i)
        # Handle cases where there may be intermediate PEs (like fp_add) between elementwise PE and output IO

        # Helper function to extract index from intermediate PE names
        def extract_intermediate_pe_idx(pe_name):
            """
            Extract index from intermediate PE names like op_hcompute_output_glb_stencil$...
            or op_hcompute_output_glb_stencil_<idx>$...
            """
            if "op_hcompute_output_glb_stencil$" in pe_name:
                return 0  # No suffix means index 0
            elif "op_hcompute_output_glb_stencil_" in pe_name:
                # Extract number after the underscore
                match = re.search(r"op_hcompute_output_glb_stencil_(\d+)\$", pe_name)
                if match:
                    return int(match.group(1))
            return None

        # Helper function to trace forward from output IO to find intermediate PE
        def trace_to_intermediate_pe(output_io_name):
            """
            Trace forward from output IO to find intermediate PE (like fp_add).
            Returns the intermediate PE name if found, None otherwise.
            Intermediate PEs should have op_hcompute_output_glb_stencil in their name
            but NOT be elementwise PEs (elementwise PEs connect directly to output IOs).
            """
            output_io_in_port = f"{output_io_name}.in"

            # Find what connects to the output IO input port
            for left, right in original_connections:
                if output_io_in_port == right:
                    # Found connection where output IO is the destination
                    source_port = left
                    source_instance = source_port.split(".")[0]

                    # Check if source is an intermediate PE (not elementwise)
                    if ("op_hcompute_output_glb_stencil" in source_instance and
                        ".O0" in source_port and
                        elementwise_op_pattern not in source_instance):
                        return source_instance

                elif output_io_in_port == left:
                    # Found connection where output IO is the source
                    dest_port = right
                    dest_instance = dest_port.split(".")[0]

                    # Check if destination is an intermediate PE (not elementwise)
                    if ("op_hcompute_output_glb_stencil" in dest_instance and
                        ".O0" in dest_port and
                        elementwise_op_pattern not in dest_instance):
                        return dest_instance

            return None

        # Build mapping from output IO index -> intermediate PE name
        output_io_idx_to_intermediate_pe = {}
        for output_io_idx, output_io_name in output_io_by_idx.items():
            intermediate_pe_name = trace_to_intermediate_pe(output_io_name)
            if intermediate_pe_name:
                # Verify the intermediate PE index matches the output IO index
                intermediate_pe_idx = extract_intermediate_pe_idx(intermediate_pe_name)
                if intermediate_pe_idx == output_io_idx:
                    output_io_idx_to_intermediate_pe[output_io_idx] = intermediate_pe_name

        # Connect elementwise PEs to output IOs (or intermediate PEs if present)
        # Path: input IO (index i) -> BANK (index j) -> elementwise (lane j) -> [intermediate PE] -> output IO (index i)
        for input_io_idx, bank_idx in input_io_idx_to_bank_idx.items():
            # Find elementwise PE with lane matching the BANK index
            if bank_idx in lane_to_elementwise and input_io_idx in output_io_by_idx:
                elementwise_pe_name = lane_to_elementwise[bank_idx]
                output_io_name = output_io_by_idx[input_io_idx]

                # Check if there's an intermediate PE for this output IO
                if input_io_idx in output_io_idx_to_intermediate_pe:
                    # Connect elementwise PE to intermediate PE
                    intermediate_pe_name = output_io_idx_to_intermediate_pe[input_io_idx]
                    # Based on the pattern in original connections, elementwise PEs connect to data0 of intermediate PEs
                    add_conn_once(f"{elementwise_pe_name}.O0", f"{intermediate_pe_name}.data0")
                else:
                    # No intermediate PE, connect directly to output IO
                    add_conn_once(f"{elementwise_pe_name}.O0", f"{output_io_name}.in")

        # Error handling: Check if intermediate PE mapping is incomplete
        if output_io_idx_to_intermediate_pe:
            # Some output IOs have intermediate PEs, verify all expected ones are mapped
            mapped_output_indices = set(output_io_idx_to_intermediate_pe.keys())
            expected_output_indices = set(output_io_by_idx.keys())
            if mapped_output_indices != expected_output_indices:
                missing_indices = expected_output_indices - mapped_output_indices
                # This is a warning, not an error, as some might connect directly
                print(f"[WARNING]: Some output IOs don't have intermediate PEs mapped: {sorted(missing_indices)}")

        top_module_json["connections"] = connections

        # Write PE fifos bypass config for dual accumulator PEs
        bypass_cfg = {
            accum_pe_0_name: {"input_fifo_bypass": [0, 0, 0], "output_fifo_bypass": 1},
            accum_pe_1_name: {"input_fifo_bypass": [0, 0, 0], "output_fifo_bypass": 1},
        }
        bypass_path = os.path.join(bin_path, "PE_fifos_bypass_config.json")
        print(f"Writing PE_fifos_bypass_config to {bypass_path}")
        with open(bypass_path, "w") as f:
            json.dump(bypass_cfg, f, indent=2)

        # Configure IO metadata
        num_vec = int(self.halide_gen_args_dict["vec_height"])
        vec_len = int(self.halide_gen_args_dict["vec_width"])
        glb_i = int(self.halide_gen_args_dict["glb_i"])

        for inst_name, inst_config in instance_dict.items():
            if inst_config.get("modref") != "global.IO":
                continue

            metadata = inst_config.setdefault("metadata", {})

            if "io16in_input_host_stencil" in inst_name:
                md = metadata.setdefault("glb2out_0", {})
                md["cycle_starting_addr"] = [0]
                md["cycle_stride"] = [1]
                md["dimensionality"] = 1
                md["extent"] = [num_vec * vec_len // glb_i]
                md["read_data_starting_addr"] = [0]
                md["read_data_stride"] = [1]
            elif "io16_hw_output_stencil" in inst_name:
                md = metadata.setdefault("in2glb_0", {})
                md["cycle_starting_addr"] = [0]
                md["cycle_stride"] = [1]
                md["dimensionality"] = 1
                md["extent"] = [num_vec * vec_len // glb_i]
                md["write_data_starting_addr"] = [0]
                md["write_data_stride"] = [1]

        # Hack design_meta_halide.json, assert only one input and one output, input and output should both have shape [vec_len, num_vec]
        design_meta_path = os.path.join(bin_path, "design_meta_halide.json")
        if os.path.exists(design_meta_path):
            with open(design_meta_path, "r") as f:
                design_meta = json.load(f)

            inputs = design_meta.get("IOs", {}).get("inputs", [])
            outputs = design_meta.get("IOs", {}).get("outputs", [])

            assert len(inputs) == 1, "Expect exactly one input in design_meta_halide.json"
            assert len(outputs) == 1, "Expect exactly one output in design_meta_halide.json"

            inputs[0]["shape"] = [vec_len, num_vec]
            outputs[0]["shape"] = [vec_len, num_vec]

            with open(design_meta_path, "w") as f:
                json.dump(design_meta, f, indent=2)

        # Write updated JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

    def hack_for_stable_softmax_pass3_fp_rv(self, json_path, bin_path):
        """
        Restructure stable_softmax_pass3_fp for:
        - Unrolled input IOs -> input buffer MEM.data_out_1 -> reduction tree with dual-accumulator schedule -> final reduce PE -> sum buffer MEM
        - For each unrolled lane: input IO -> MEM.data_in_0
        - Remove unnecessary mem tiles between tree stages
        - Add dual accumulator schedules
        """
        self._hack_reduction_followed_by_elementwise_rv(
            json_path, bin_path,
            top_module="stable_softmax_pass3_fp",
            scalar_op_type="division",
            elementwise_op_type="fp_mul"
        )


    def hack_for_scalar_avg_fp_rv(self, json_path, bin_path):

        # TODO: Hardcode input pipelining regs for now
        INPUT_PIPELINING_REGS = 3

        with open(json_path, "r") as f:
            design = json.load(f)

        top_module = "scalar_avg_fp"
        tree_out_pond = "tree_3_stencil$ub_tree_3_stencil_BANK_0_garnet"
        tree_out_pond_clk = "tree_3_stencil$ub_tree_3_stencil_BANK_0_clk_en_const"

        # Const PE instance to remove
        pe_to_remove = "op_hcompute_output_cgra_stencil$inner_compute$const_"

        # Locate "scalar_avg_fp" module
        global_modules = design["namespaces"]["global"]["modules"]
        if top_module not in global_modules:
            print(f"WARNING: Module '{top_module}' not found in design. No hack applied.")
            return
        scalar_avg_fp = global_modules[top_module]

        # Remove the tile instance and its clk_en_const
        instance_dict = scalar_avg_fp.get("instances", {})
        if tree_out_pond in instance_dict:
            del instance_dict[tree_out_pond]
        if tree_out_pond_clk in instance_dict:
            del instance_dict[tree_out_pond_clk]

        # Remove references to the tile from connections,
        #    capturing upstream => old_input, downstream => old_output
        old_input = None
        old_output = None
        new_connections = []

        connection_list = scalar_avg_fp.get("connections", [])
        for conn in connection_list:
            left, right = conn[0], conn[1]

            # Check references to tile or its clk const
            refs_tile_left = (tree_out_pond in left) or (tree_out_pond_clk in left)
            refs_tile_right = (tree_out_pond in right) or (tree_out_pond_clk in right)

            if not (refs_tile_left or refs_tile_right):
                # If no reference to tile, keep it for now
                new_connections.append(conn)
                continue

            # If referencing the tile, skip it but see if it's data_in or data_out
            if tree_out_pond in left:
                if ".data_in_pond_" in left or ".data_in_" in left:
                    old_input = right
            elif tree_out_pond in right:
                if ".data_in_pond_" in right or ".data_in_" in right:
                    old_input = left

            if tree_out_pond in left:
                if ".data_out_pond_" in left or ".data_out_" in left:
                    old_output = right
            elif tree_out_pond in right:
                if ".data_out_pond_" in right or ".data_out_" in right:
                    old_output = left

        # Reconnect old_input -> old_output if both exist
        if old_input and old_output:
            new_connections.append([old_input, old_output])
        else:
            print(
                f"WARNING: Could not find both data_in and data_out references for "
                f"tile '{tree_out_pond}'. No direct reconnection added."
            )

        # Remove the PE instance "op_hcompute_output_cgra_stencil$inner_compute$const_"
        #    from the instance dictionary (if present)
        if pe_to_remove in instance_dict:
            del instance_dict[pe_to_remove]

        # Filter out any connections referencing const PE instance
        final_connections = []
        for conn in new_connections:
            left, right = conn[0], conn[1]
            if pe_to_remove in left or pe_to_remove in right:
                # skip any references to that PE
                continue
            final_connections.append(conn)

        # Update the final connection list
        scalar_avg_fp["connections"] = final_connections

        # Identify psum PE with Pond and generate fifo bypass config
        # Write bypass config to design_top at the top level
        psum_pe_name = "op_hcompute_output_cgra_stencil_1$inner_compute$float_DW_fp_add_"
        PE_fifos_bypass_config = {
            psum_pe_name: {
                # HACK: Assuming the PE input port connected to Pond is "data_in_0"
                # We bypass the input and output fifos on the feedback path
                "input_fifo_bypass": [1, 0, 0],
                "output_fifo_bypass": 1,
            }
        }
        PE_fifos_bypass_config_path = os.path.join(bin_path, "PE_fifos_bypass_config.json")
        print(f"Writing PE_fifos_bypass_config to {PE_fifos_bypass_config_path}")
        with open(PE_fifos_bypass_config_path, "w") as f:
            json.dump(PE_fifos_bypass_config, f, indent=2)

        # Update output IO tile extent
        # HACK: Set extent to 2 (should set this to 1 since we only have 1 output, but then can't get f2g interrupt)
        # Also have to update cycle_start_addr and stride, as it means handshake rather than cycles in dense rv scenario
        io_out_name = "io16_hw_output_stencil_op_hcompute_hw_output_stencil_write_0"
        instance_dict[io_out_name]["metadata"]["in2glb_0"]["extent"] = [2]
        # We want to accept every valid handshake starting from the first one
        instance_dict[io_out_name]["metadata"]["in2glb_0"]["cycle_starting_addr"] = [0]
        instance_dict[io_out_name]["metadata"]["in2glb_0"]["cycle_stride"] = [1]

        # Overwrite the JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

    def hack_for_layer_norm_pass1_fp_rv(self, json_path, bin_path):
        """
        Restructure layer_norm_pass1_fp for:
        - Unrolled input IOs -> input buffer MEM.data_out_1 -> reduction tree with dual-accumulator schedule -> final reduce PE -> fp_mul (scalar op)
        - For each unrolled lane: input IO -> MEM.data_in_0
        - Remove unnecessary mem tiles between tree stages
        - Add dual accumulator schedules
        - Elementwise operation: fp_add
        - Scalar operation: fp_mul
        """
        self._hack_reduction_followed_by_elementwise_rv(
            json_path, bin_path,
            top_module="layer_norm_pass1_fp",
            scalar_op_type="fp_mul",
            elementwise_op_type="fp_add"
        )

    def hack_for_layer_norm_pass2_fp_rv(self, json_path, bin_path):
        self._hack_reduction_followed_by_elementwise_rv(
            json_path, bin_path,
            top_module="layer_norm_pass2_fp",
            scalar_op_type="log",
            elementwise_op_type="fp_mul"
        )

        # Add dummy_max_nop PE between fp_addiexp and fp_subexp for path balancing
        with open(json_path, "r") as f:
            design = json.load(f)

        top_module = "layer_norm_pass2_fp"
        global_modules = design["namespaces"]["global"]["modules"]
        if top_module not in global_modules:
            raise RuntimeError(f"[ERROR]: Module '{top_module}' not found in design.")

        layer_norm_pass2_fp = global_modules[top_module]
        instances = layer_norm_pass2_fp["instances"]
        connections = layer_norm_pass2_fp["connections"]

        # Find connections from fp_addiexp to fp_subexp
        connections_to_modify = []
        for conn in connections:
            left, right = conn[0], conn[1]
            # Check both directions since connections are unordered
            if "fp_addiexp" in left and ".O0" in left and "fp_subexp" in right and ".data1" in right:
                connections_to_modify.append((left, right))
            elif "fp_addiexp" in right and ".O0" in right and "fp_subexp" in left and ".data1" in left:
                connections_to_modify.append((right, left))

        if not connections_to_modify:
            raise RuntimeError(f"[ERROR]: No fp_addiexp to fp_subexp connections found in '{top_module}'.")

        # Helper function to add connection only if it doesn't exist
        def add_conn_once(src, dst):
            pair = [src, dst]
            if pair not in connections:
                connections.append(pair)

        # Create dummy_max_nop PEs and modify connections
        pe_counter = 0
        for src, dst in connections_to_modify:
            # Extract base name from source for PE naming
            src_base = src.split(".O0")[0]
            dummy_pe_name = f"{src_base}_dummy_max_nop_pe{pe_counter}"
            dummy_const_name = f"{src_base}_dummy_max_nop_const{pe_counter}"

            # Create const instruction instance
            instances[dummy_const_name] = {
                "genref": "coreir.const",
                "genargs": {"width": ["Int", 84]},
                "modargs": {"value": [["BitVector", 84], self.DUMMY_MAX_NOP_INSTR]},
            }

            # Create PE instance
            instances[dummy_pe_name] = copy.deepcopy(self.pe_tpl)

            # Remove original connection
            removed = False
            pair1 = [src, dst]
            pair2 = [dst, src]
            if pair1 in connections:
                connections.remove(pair1)
                removed = True
            elif pair2 in connections:
                connections.remove(pair2)
                removed = True
            if not removed:
                raise RuntimeError(
                    f"[ERROR]: Expected connection between '{src}' and '{dst}' not found when inserting dummy PE."
                )

            # Add new connections through dummy PE
            add_conn_once(src, f"{dummy_pe_name}.data0")
            add_conn_once(f"{dummy_pe_name}.inst", f"{dummy_const_name}.out")
            add_conn_once(f"{dummy_pe_name}.O0", dst)

            print(f"INFO: Added dummy_max_nop PE '{dummy_pe_name}' between '{src}' and '{dst}'")
            pe_counter += 1

        # Move square fp_mul PEs to after MEM.data_out_0
        # Find all square fp_mul PEs (op_hcompute_tile_input_stencil* with fp_mul)
        square_fp_mul_pes = []
        for inst_name in instances:
            if "op_hcompute_tile_input_stencil" in inst_name and "float_DW_fp_mul" in inst_name:
                square_fp_mul_pes.append(inst_name)

        # For each square fp_mul PE, find:
        # 1. Connections from IO to fp_mul (both data0 and data1)
        # 2. Connection from fp_mul.O0 to MEM.data_in_0
        # 3. The corresponding MEM bank and IO
        for fp_mul_pe in square_fp_mul_pes:
            # Find IO connections (both data0 and data1 should connect to same IO)
            io_connections = []
            mem_connection = None
            mem_bank_name = None

            for conn in connections:
                left, right = conn[0], conn[1]

                # Check for IO -> fp_mul connections
                if f"{fp_mul_pe}.data0" in right and "io16in_input_host_stencil" in left and ".out" in left:
                    io_connections.append((left, right))
                elif f"{fp_mul_pe}.data0" in left and "io16in_input_host_stencil" in right and ".out" in right:
                    io_connections.append((right, left))

                if f"{fp_mul_pe}.data1" in right and "io16in_input_host_stencil" in left and ".out" in left:
                    io_connections.append((left, right))
                elif f"{fp_mul_pe}.data1" in left and "io16in_input_host_stencil" in right and ".out" in right:
                    io_connections.append((right, left))

                # Check for fp_mul.O0 -> MEM.data_in_0 connection
                if f"{fp_mul_pe}.O0" in left and "tile_input_stencil$ub_tile_input_stencil_BANK" in right and ".data_in_0" in right:
                    mem_connection = (left, right)
                    mem_bank_name = right.split(".")[0]
                elif f"{fp_mul_pe}.O0" in right and "tile_input_stencil$ub_tile_input_stencil_BANK" in left and ".data_in_0" in left:
                    mem_connection = (right, left)
                    mem_bank_name = left.split(".")[0]

            if not io_connections or not mem_connection:
                raise RuntimeError(f"[ERROR]: Could not find all connections for square fp_mul PE '{fp_mul_pe}'.")

            # Extract IO name
            io_names = set()
            for io_conn in io_connections:
                io_name = io_conn[0].split(".")[0] if "io16in_input_host_stencil" in io_conn[0] else io_conn[1].split(".")[0]
                io_names.add(io_name)

            if len(io_names) != 1:
                raise RuntimeError(f"[ERROR]: Expected single IO for fp_mul PE '{fp_mul_pe}', found {io_names}.")

            io_name = next(iter(io_names))
            io_out_port = f"{io_name}.out"
            mem_data_out_0 = f"{mem_bank_name}.data_out_0"
            mem_data_in_0 = f"{mem_bank_name}.data_in_0"

            # Find tree PE connections from MEM.data_out_0
            tree_pe_connections = []
            for conn in connections:
                left, right = conn[0], conn[1]
                # Check if this connection is from MEM.data_out_0 to a tree PE
                if mem_data_out_0 == left and "op_hcompute_tree" in right and ".data" in right:
                    tree_pe_connections.append(right)
                elif mem_data_out_0 == right and "op_hcompute_tree" in left and ".data" in left:
                    tree_pe_connections.append(left)

            # Remove old connections
            connections_to_remove_indices = []
            fp_mul_o0 = f"{fp_mul_pe}.O0"
            fp_mul_data0 = f"{fp_mul_pe}.data0"
            fp_mul_data1 = f"{fp_mul_pe}.data1"
            for idx, conn in enumerate(connections):
                left, right = conn[0], conn[1]
                # Remove IO -> fp_mul connections
                if (io_out_port == left and (right == fp_mul_data0 or right == fp_mul_data1)) or \
                   (io_out_port == right and (left == fp_mul_data0 or left == fp_mul_data1)):
                    connections_to_remove_indices.append(idx)
                # Remove fp_mul.O0 -> MEM.data_in_0 connection
                elif (fp_mul_o0 == left and mem_data_in_0 == right) or \
                     (fp_mul_o0 == right and mem_data_in_0 == left):
                    connections_to_remove_indices.append(idx)
                # Remove MEM.data_out_0 -> tree PE connections (will be replaced with fp_mul.O0 -> tree PE)
                elif (mem_data_out_0 == left and right in tree_pe_connections) or (mem_data_out_0 == right and left in tree_pe_connections):
                    connections_to_remove_indices.append(idx)

            # Remove connections in reverse order to maintain indices
            for idx in sorted(connections_to_remove_indices, reverse=True):
                removed_conn = connections.pop(idx)
                print(f"[DEBUG] Removed connection: {removed_conn}")

            # Add new connections:
            # 1. IO -> MEM.data_in_0 (direct, no fp_mul)
            add_conn_once(io_out_port, mem_data_in_0)

            # 2. MEM.data_out_0 -> fp_mul.data0 and fp_mul.data1 (squaring after MEM)
            add_conn_once(mem_data_out_0, f"{fp_mul_pe}.data0")
            add_conn_once(mem_data_out_0, f"{fp_mul_pe}.data1")

            # 3. fp_mul.O0 -> tree PEs (redirected from MEM.data_out_0 -> tree PEs)
            for tree_pe_port in tree_pe_connections:
                add_conn_once(fp_mul_o0, tree_pe_port)

            print(f"[INFO] Moved square fp_mul PE '{fp_mul_pe}' to after {mem_bank_name}.data_out_0")

        # Write modified design back
        with open(json_path, "w") as f:
            json.dump(design, f, indent=2)

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

    def hack_for_silu_pass2_fp_rv(self, json_path, bin_path):
        return self.hack_for_silu_pass2_fp_static(json_path, bin_path)

    def hack_for_swiglu_pass2_fp_rv(self, json_path, bin_path):
        return self.hack_for_swiglu_pass2_fp_static(json_path, bin_path)

    def hack_for_mem_transpose_test_rv(self, json_path, bin_path, module_name="mem_transpose_test"):

        with open(json_path, "r") as f:
            design = json.load(f)

        top = design["namespaces"]["global"]["modules"][module_name]
        insts = top["instances"]
        conns = top["connections"]

        # Identify and remove the two PE instances and their const drivers
        to_remove = []
        for name, info in insts.items():
            if info.get("modref") == "global.PE":
                to_remove.append(name)
            if name.startswith("op_hcompute_conv_stencil") and ("inner_compute$" in name):
                to_remove.append(name)

        for name in set(to_remove):
            insts.pop(name, None)

        # Filter out any connections that reference the removed instances
        kept_conns = []
        for src, dst in conns:
            src_inst = src.split(".", 1)[0]
            dst_inst = dst.split(".", 1)[0]
            if src_inst in to_remove or dst_inst in to_remove:
                continue
            kept_conns.append([src, dst])

        # Connect the MEM tile to the output IO
        kept_conns.append(
            [
                "hw_input_global_wrapper_stencil$ub_hw_input_global_wrapper_stencil_BANK_0_garnet.data_out_0",
                "io16_hw_output_stencil_op_hcompute_hw_output_stencil_write_0.in",
            ]
        )

        # Update IO tile config
        io_in_name = "io16in_hw_input_stencil_op_hcompute_hw_input_global_wrapper_stencil_read_0"
        io_out_name = "io16_hw_output_stencil_op_hcompute_hw_output_stencil_write_0"
        # Input config
        insts[io_in_name]["metadata"]["glb2out_0"]["cycle_starting_addr"] = [0]
        insts[io_in_name]["metadata"]["glb2out_0"]["cycle_stride"] = [1]
        insts[io_in_name]["metadata"]["glb2out_0"]["dimensionality"] = 1
        insts[io_in_name]["metadata"]["glb2out_0"]["extent"] = [
            int(self.halide_gen_args_dict["in_img_x"]) * int(self.halide_gen_args_dict["in_img_y"])
        ]
        # Output config
        insts[io_out_name]["metadata"]["in2glb_0"]["cycle_starting_addr"] = [0]
        insts[io_out_name]["metadata"]["in2glb_0"]["cycle_stride"] = [1]
        insts[io_out_name]["metadata"]["in2glb_0"]["dimensionality"] = 1
        insts[io_out_name]["metadata"]["in2glb_0"]["extent"] = [
            int(self.halide_gen_args_dict["in_img_x"]) * int(self.halide_gen_args_dict["in_img_y"])
        ]

        top["connections"] = kept_conns

        # Overwrite the JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

    def hack_for_mem_slice_test_rv(self, json_path, bin_path):
        # Apply the same hack as mem_transpose_test
        self.hack_for_mem_transpose_test_rv(json_path, bin_path, module_name="mem_slice_test")

        # Adjust IO tile config
        with open(json_path, "r") as f:
            design = json.load(f)

        insts = design["namespaces"]["global"]["modules"]["mem_slice_test"]["instances"]
        io_in_name = "io16in_hw_input_stencil_op_hcompute_hw_input_global_wrapper_stencil_read_0"
        io_out_name = "io16_hw_output_stencil_op_hcompute_hw_output_stencil_write_0"

        insts[io_in_name]["metadata"]["glb2out_0"]["extent"] = [
            int(self.halide_gen_args_dict["in_img_x"]) * int(self.halide_gen_args_dict["in_img_y"])
        ]
        insts[io_out_name]["metadata"]["in2glb_0"]["extent"] = [
            int(self.halide_gen_args_dict["in_img_x"])
            * int(self.halide_gen_args_dict["in_img_y"])
            // 2
        ]

        # Overwrite the JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

    def hack_for_mem_filter_test_rv(self, json_path, bin_path):
        # Apply the same hack as mem_transpose_test
        self.hack_for_mem_transpose_test_rv(json_path, bin_path, module_name="mem_filter_test")

        # Adjust IO tile config
        with open(json_path, "r") as f:
            design = json.load(f)

        insts = design["namespaces"]["global"]["modules"]["mem_filter_test"]["instances"]
        io_in_name = "io16in_hw_input_stencil_op_hcompute_hw_input_global_wrapper_stencil_read_0"
        io_out_name = "io16_hw_output_stencil_op_hcompute_hw_output_stencil_write_0"

        insts[io_in_name]["metadata"]["glb2out_0"]["extent"] = [
            int(self.halide_gen_args_dict["in_img_x"]) * int(self.halide_gen_args_dict["in_img_y"])
        ]
        insts[io_out_name]["metadata"]["in2glb_0"]["extent"] = [
            int(self.halide_gen_args_dict["in_img_x"])
            * int(self.halide_gen_args_dict["in_img_y"])
            // 4
        ]

        # Overwrite the JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

    def hack_for_avgpool_layer_fp_rv_with_output_mem(self, json_path, bin_path):
        with open(json_path, "r") as f:
            design = json.load(f)

        top_module = "avgpool_layer_fp"
        const_prefix = "op_hcompute_output_cgra_stencil$inner_compute$const"
        modules = design["namespaces"]["global"]["modules"]
        if top_module not in modules:
            print(f"WARNING: Module '{top_module}' not found. No hack applied.")
            return

        avg = modules[top_module]
        instances = avg["instances"]
        conns = avg["connections"]

        # 1. Remove constant PEs
        for inst in list(instances):
            if inst.startswith(const_prefix):
                del instances[inst]
        filtered = [c for c in conns if const_prefix not in c[0] and const_prefix not in c[1]]

        # 2. Insert Pond: intercept PE->Mem writes, fork back to PE and forward to Mem
        mems = {n for n, i in instances.items() if i.get("genref") == "cgralib.Mem"}

        new_conns = []
        counts = {}

        for a, b in filtered:
            # remove original Mem->PE reads
            m_out = re.match(r"^(.+)\.data_out_(\d+)$", a)
            if m_out and b.endswith(".data0") and m_out.group(1) in mems:
                continue
            m_out = re.match(r"^(.+)\.data_out_(\d+)$", b)
            if m_out and a.endswith(".data0") and m_out.group(1) in mems:
                continue

            # detect PE->Mem write: src PE.O0, dst mem.data_in_i
            m_in = re.match(r"^(.+)\.data_in_(\d+)$", b)
            if m_in and a.endswith(".O0") and m_in.group(1) in mems:
                pe_src = a
                mem, idx = m_in.group(1), m_in.group(2)
            else:
                m_in = re.match(r"^(.+)\.data_in_(\d+)$", a)
                if m_in and b.endswith(".O0") and m_in.group(1) in mems:
                    pe_src = b
                    mem, idx = m_in.group(1), m_in.group(2)
                else:
                    new_conns.append((a, b))
                    continue

            # create pond
            key = (mem, idx)
            cnt = counts.get(key, 0)
            p = f"{mem}$pond_{idx}_{cnt}"
            counts[key] = cnt + 1
            inst = copy.deepcopy(self.pond_tpl)
            inst["genargs"]["ID"][1] = p
            instances[p] = inst

            # fork: PE->pond input
            new_conns.append((pe_src, f"{p}.data_in_pond_0"))
            # fork output0: back to PE input port (replace .O0 with .data0)
            pe_input = pe_src.rsplit(".O0", 1)[0] + ".data0"
            new_conns.append((f"{p}.data_out_pond_0", pe_input))
            # fork output1: to mem write port
            new_conns.append((f"{p}.data_out_pond_1", f"{mem}.data_in_{idx}"))

        avg["connections"] = new_conns

        # Configure input IO with stride to read pixels from the same channel
        for inst_name, inst_config in instances.items():
            if "io16in_input_host_stencil" in inst_name and inst_config["modref"] == "global.IO":
                inst_config["metadata"]["glb2out_0"]["cycle_starting_addr"] = [0]
                inst_config["metadata"]["glb2out_0"]["cycle_stride"] = [
                    1,
                    int(self.halide_gen_args_dict["in_img"])
                    * int(self.halide_gen_args_dict["in_img"]),
                ]
                inst_config["metadata"]["glb2out_0"]["dimensionality"] = 2
                inst_config["metadata"]["glb2out_0"]["extent"] = [
                    int(self.halide_gen_args_dict["in_img"])
                    * int(self.halide_gen_args_dict["in_img"]),
                    int(self.halide_gen_args_dict["n_ic"])
                    // int(self.halide_gen_args_dict["glb_i"]),
                ]
                inst_config["metadata"]["glb2out_0"]["read_data_starting_addr"] = [0]
                inst_config["metadata"]["glb2out_0"]["read_data_stride"] = [
                    int(self.halide_gen_args_dict["n_ic"])
                    // int(self.halide_gen_args_dict["glb_i"]),
                    1,
                ]

        # Configure output IO to remove static cycle offset
        for inst_name, inst_config in instances.items():
            if "io16_hw_output_stencil" in inst_name and inst_config["modref"] == "global.IO":
                inst_config["metadata"]["in2glb_0"]["cycle_starting_addr"] = [0]

        # write back JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

    def hack_for_avgpool_layer_fp_rv(self, json_path, bin_path):
        with open(json_path, "r") as f:
            design = json.load(f)

        top_module = "avgpool_layer_fp"
        const_prefix = "op_hcompute_output_cgra_stencil$inner_compute$const"
        modules = design["namespaces"]["global"]["modules"]
        if top_module not in modules:
            print(f"WARNING: Module '{top_module}' not found. No hack applied.")
            return

        avg = modules[top_module]
        instances = avg["instances"]
        old_conns = avg["connections"]

        # Remove constant PEs
        for inst in list(instances):
            if inst.startswith(const_prefix):
                del instances[inst]
        filtered_conns = [
            c for c in old_conns if const_prefix not in c[0] and const_prefix not in c[1]
        ]

        # Remove old single pond + PE accumulator and rewire connections
        old_output_pond = "output_cgra_stencil$ub_output_cgra_stencil_BANK_"
        old_output_pond_clk = "output_cgra_stencil$ub_output_cgra_stencil_BANK_"
        old_output_pe_const = "op_hcompute_output_cgra_stencil_"
        ponds_by_idx = {}
        clks_by_idx = {}
        adds_by_idx = {}
        muls_by_idx = {}
        for inst_name in list(instances):
            if inst_name.startswith(old_output_pond) and inst_name.endswith("_garnet"):
                # ...BANK_<i>_garnet
                idx = int(inst_name.split("_BANK_")[1].split("_")[0])
                ponds_by_idx[idx] = inst_name
            elif inst_name.startswith(old_output_pond_clk) and inst_name.endswith("_clk_en_const"):
                idx = int(inst_name.split("_BANK_")[1].split("_")[0])
                clks_by_idx[idx] = inst_name
            elif inst_name.startswith(old_output_pe_const) and "$inner_compute$float_DW_fp_add_" in inst_name:
                # op_hcompute_output_cgra_stencil_<i>$inner_compute$float_DW_fp_add_...
                idx = int(inst_name.split("op_hcompute_output_cgra_stencil_")[1].split("$")[0])
                adds_by_idx[idx] = inst_name
            elif inst_name.startswith(old_output_pe_const) and "$inner_compute$float_DW_fp_mul" in inst_name:
                idx = int(inst_name.split("op_hcompute_output_cgra_stencil_")[1].split("$")[0])
                muls_by_idx[idx] = inst_name
        # Delete instances
        for i, name in ponds_by_idx.items():
            if name in instances:
                del instances[name]
        for i, name in clks_by_idx.items():
            if name in instances:
                del instances[name]
        for i, name in adds_by_idx.items():
            if name in instances:
                del instances[name]
        # Rewire connections: pond.data_out_pond_1 -> lane becomes mul.O0 -> lane
        rewired_conns = []
        for left, right in filtered_conns:
            rewired = False

            # Left side is pond out_1
            if ".data_out_pond_1" in left:
                pond_inst = left.split(".")[0]
                if pond_inst.startswith(old_output_pond):
                    idx = int(pond_inst.split("_BANK_")[1].split("_")[0])
                    # pond i corresponds to mul (i+1)
                    mul_inst = muls_by_idx.get(idx + 1)
                    if mul_inst is not None:
                        rewired_conns.append([f"{mul_inst}.O0", right])
                        rewired = True

            # Right side is pond out_1
            if (not rewired) and ".data_out_pond_1" in right:
                pond_inst = right.split(".")[0]
                if pond_inst.startswith(old_output_pond):
                    idx = int(pond_inst.split("_BANK_")[1].split("_")[0])
                    mul_inst = muls_by_idx.get(idx + 1)
                    if mul_inst is not None:
                        rewired_conns.append([left, f"{mul_inst}.O0"])
                        rewired = True

            if rewired:
                continue

            # Drop any other edge touching old pond / clk / add
            left_base = left.split(".")[0]
            right_base = right.split(".")[0]
            if (
                left_base in ponds_by_idx.values()
                or right_base in ponds_by_idx.values()
                or left_base in clks_by_idx.values()
                or right_base in clks_by_idx.values()
                or left_base in adds_by_idx.values()
                or right_base in adds_by_idx.values()
            ):
                continue

            rewired_conns.append([left, right])

        # Use rewired_conns for the rest
        filtered_conns = rewired_conns

        # Find all output IOs and all final "src -> IO.in" connections
        io_in_conns = []
        for left, right in filtered_conns:
            if right.endswith(".in"):
                io_inst = right.split(".")[0]
                if io_inst in instances and instances[io_inst].get("modref") == "global.IO" and instances[io_inst].get("modargs").get("mode") == ["String", "out"]:
                    io_in_conns.append((left, right))

        if not io_in_conns:
            raise ValueError("Cannot find any final source that drives output IO.")

        # Keep old connections that are not affected by the hack
        kept_conns = []
        for left, right in filtered_conns:
            if (left, right) in io_in_conns:
                continue
            kept_conns.append([left, right])

        # One shared clk const
        shared_clk_const_name = f"{top_module}_clk_en_const"
        if shared_clk_const_name not in instances:
            instances[shared_clk_const_name] = copy.deepcopy(self.const_clk_tpl)

        # Rebuild connections
        connections = []

        def add_conn_once(src, dst):
            pair = [src, dst]
            if pair not in connections:
                connections.append(pair)

        # Keep unaffected connections
        for left, right in kept_conns:
            add_conn_once(left, right)

        # Per-lane accumulators
        lane_bypass_cfg = {}
        for lane_idx, (old_final_src, final_sink) in enumerate(io_in_conns):
            lane_prefix = f"{top_module}_lane_{lane_idx}"

            filter_mem_name = f"{lane_prefix}_filter_mem"
            accum_pond_0_name = f"{lane_prefix}_accum_pond_0"
            accum_pond_1_name = f"{lane_prefix}_accum_pond_1"
            accum_pe_0_name = f"{lane_prefix}_accum_pe_0"
            accum_pe_1_name = f"{lane_prefix}_accum_pe_1"
            final_reduce_pe_name = f"{lane_prefix}_final_reduce_pe"
            accum_pe_0_inst_const_name = f"{lane_prefix}_accum_pe_0_inst_const"
            accum_pe_1_inst_const_name = f"{lane_prefix}_accum_pe_1_inst_const"
            final_reduce_pe_inst_const_name = f"{lane_prefix}_final_reduce_pe_inst_const"

            if filter_mem_name not in instances:
                instances[filter_mem_name] = copy.deepcopy(self.mem_tpl)
            if accum_pond_0_name not in instances:
                instances[accum_pond_0_name] = copy.deepcopy(self.pond_tpl)
            if accum_pond_1_name not in instances:
                instances[accum_pond_1_name] = copy.deepcopy(self.pond_tpl)
            if accum_pe_0_name not in instances:
                instances[accum_pe_0_name] = copy.deepcopy(self.pe_tpl)
            if accum_pe_1_name not in instances:
                instances[accum_pe_1_name] = copy.deepcopy(self.pe_tpl)
            if final_reduce_pe_name not in instances:
                instances[final_reduce_pe_name] = copy.deepcopy(self.pe_tpl)
            if accum_pe_0_inst_const_name not in instances:
                instances[accum_pe_0_inst_const_name] = copy.deepcopy(self.fp_add_const_tpl)
            if accum_pe_1_inst_const_name not in instances:
                instances[accum_pe_1_inst_const_name] = copy.deepcopy(self.fp_add_const_tpl)
            if final_reduce_pe_inst_const_name not in instances:
                instances[final_reduce_pe_inst_const_name] = copy.deepcopy(self.fp_add_const_tpl)

            add_conn_once(old_final_src, f"{filter_mem_name}.data_in_0")

            add_conn_once(f"{shared_clk_const_name}.out", f"{filter_mem_name}.clk_en")
            add_conn_once(f"{shared_clk_const_name}.out", f"{accum_pond_0_name}.clk_en")
            add_conn_once(f"{shared_clk_const_name}.out", f"{accum_pond_1_name}.clk_en")

            add_conn_once(f"{filter_mem_name}.data_out_0", f"{accum_pe_0_name}.data1")
            add_conn_once(f"{filter_mem_name}.data_out_1", f"{accum_pe_1_name}.data1")

            add_conn_once(f"{accum_pe_0_inst_const_name}.out", f"{accum_pe_0_name}.inst")
            add_conn_once(f"{accum_pe_1_inst_const_name}.out", f"{accum_pe_1_name}.inst")
            add_conn_once(f"{final_reduce_pe_inst_const_name}.out", f"{final_reduce_pe_name}.inst")

            add_conn_once(f"{accum_pond_0_name}.data_out_pond_0", f"{accum_pe_0_name}.data0")
            add_conn_once(f"{accum_pond_0_name}.data_in_pond_0", f"{accum_pe_0_name}.O0")
            add_conn_once(f"{accum_pond_0_name}.data_out_pond_1", f"{final_reduce_pe_name}.data0")

            add_conn_once(f"{accum_pond_1_name}.data_out_pond_0", f"{accum_pe_1_name}.data0")
            add_conn_once(f"{accum_pond_1_name}.data_in_pond_0", f"{accum_pe_1_name}.O0")
            add_conn_once(f"{accum_pond_1_name}.data_out_pond_1", f"{final_reduce_pe_name}.data1")

            add_conn_once(f"{final_reduce_pe_name}.O0", final_sink)

            lane_bypass_cfg[accum_pe_0_name] = {"input_fifo_bypass": [0, 0, 0], "output_fifo_bypass": 1}
            lane_bypass_cfg[accum_pe_1_name] = {"input_fifo_bypass": [0, 0, 0], "output_fifo_bypass": 1}

        # Write back all-lane connections
        avg["connections"] = connections

        # Write bypass config for all accumulator PEs
        bypass_path = os.path.join(bin_path, "PE_fifos_bypass_config.json")
        with open(bypass_path, "w") as f:
            json.dump(lane_bypass_cfg, f, indent=2)

        # Configure input IO with stride to read pixels from the same channel
        for inst_name, inst_conf in instances.items():
            if "io16in_input_host_stencil" in inst_name and inst_conf.get("modref") == "global.IO":
                md = inst_conf["metadata"]["glb2out_0"]
                md["cycle_starting_addr"] = [0]
                md["cycle_stride"] = [
                    1,
                    int(self.halide_gen_args_dict["in_img"])
                    * int(self.halide_gen_args_dict["in_img"]),
                ]
                md["dimensionality"] = 2
                md["extent"] = [
                    int(self.halide_gen_args_dict["in_img"])
                    * int(self.halide_gen_args_dict["in_img"]),
                    int(self.halide_gen_args_dict["n_ic"])
                    // int(self.halide_gen_args_dict["glb_i"]),
                ]
                md["read_data_starting_addr"] = [0]
                md["read_data_stride"] = [
                    int(self.halide_gen_args_dict["n_ic"])
                    // int(self.halide_gen_args_dict["glb_i"]),
                    1,
                ]

        # Configure output IO to remove static cycle offset
        for inst_name, inst_conf in instances.items():
            if "io16_hw_output_stencil" in inst_name and inst_conf.get("modref") == "global.IO":
                inst_conf["metadata"]["in2glb_0"]["cycle_starting_addr"] = [0]

        # Write back the modified design
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

        # Update design_meta_halide.json to update input/output shape
        design_meta_path = os.path.join(bin_path, "design_meta_halide.json")
        with open(design_meta_path, "r") as f:
            design_meta = json.load(f)
        assert len(design_meta["IOs"]["inputs"]) == 1, "Expect only one input in avgpool_layer_fp"
        design_meta["IOs"]["inputs"][0]["shape"] = [int(self.halide_gen_args_dict["n_ic"]), int(self.halide_gen_args_dict["in_img"]), int(self.halide_gen_args_dict["in_img"])]
        assert len(design_meta["IOs"]["outputs"]) == 1, "Expect only one output in avgpool_layer_fp"
        design_meta["IOs"]["outputs"][0]["shape"] = [int(self.halide_gen_args_dict["n_ic"]), 1, 1]
        with open(design_meta_path, "w") as f:
            json.dump(design_meta, f, indent=2)

    def hack_for_get_e8m0_scale_tree_mu_input_rv(self, json_path, bin_path):
        '''
        This hack inserts MEM tiles between MU IOs and tree_mem / tree_mu PEs.
        MEM tile output port 0 goes to tree_mem PEs, with a delay of image size.
        MEM tile output port 1 goes to tree_mu PEs, without delay.
        It also inserts unquantized output paths and scale broadcast.
        '''
        with open(json_path, "r") as f:
            design = json.load(f)

        top_module_name = "get_e8m0_scale_tree_mu_input"
        instances = design["namespaces"]["global"]["modules"][top_module_name]["instances"]
        top_module = design["namespaces"]["global"]["modules"][top_module_name]
        connections = top_module["connections"]

        def clkwrk_idx_from_port(p):
            # Helper function to extract MU IO clkwrk index from port name
            m = re.search(r"_clkwrk_(\d+)_op_hcompute_mu_input_io_stencil", p)
            return int(m.group(1)) if m else None

        # Helper function to add a connection only if it doesn't already exist
        def add_conn_once(src, dst):
            pair = [src, dst]
            if pair not in connections:
                connections.append(pair)

        # Collect original MU->tree connections and ports
        for a, b in list(connections):
            is_mu_a = a.startswith("io16in_mu_")
            is_mu_b = b.startswith("io16in_mu_")
            is_tree_mem_a = "op_hcompute_tree_mem" in a and ".data" in a
            is_tree_mem_b = "op_hcompute_tree_mem" in b and ".data" in b
            is_tree_mu_a  = "op_hcompute_tree_mu"  in a and ".data" in a
            is_tree_mu_b  = "op_hcompute_tree_mu"  in b and ".data" in b

            if is_mu_a and (is_tree_mem_b or is_tree_mu_b):
                mu_port, tree_port = a, b
            elif is_mu_b and (is_tree_mem_a or is_tree_mu_a):
                mu_port, tree_port = b, a
            else:
                continue

            # Instantiate MEMs buffering activations with matching index to MU IOs
            mu_idx = clkwrk_idx_from_port(mu_port)
            assert mu_idx is not None, f"Cannot parse clkwrk index from MU port: {mu_port}"
            mem_name = f"mem_mu2tree_{mu_idx}"
            if mem_name not in instances:
                tpl = copy.deepcopy(self.mem_tpl)
                instances[mem_name] = tpl

            # Connect MU->MEM.in0
            add_conn_once(mu_port, f"{mem_name}.data_in_0")

            # Connect MEM.out0->tree_mem and MEM.out1->tree_mu
            if "op_hcompute_tree_mem" in tree_port:
                add_conn_once(f"{mem_name}.data_out_0", tree_port)
            else:
                assert "op_hcompute_tree_mu" in tree_port, f"Tree PE port {tree_port} is not a tree_mem or tree_mu port"
                add_conn_once(f"{mem_name}.data_out_1", tree_port)

        def is_direct_mu2tree(edge):
            # Helper function to check if an edge is a direct MU->tree edge in original connections
            x, y = edge
            mu_x = x.startswith("io16in_mu_")
            mu_y = y.startswith("io16in_mu_")
            tree_mem_x = "op_hcompute_tree_mem" in x and ".data" in x
            tree_mem_y = "op_hcompute_tree_mem" in y and ".data" in y
            tree_mu_x  = "op_hcompute_tree_mu"  in x and ".data" in x
            tree_mu_y  = "op_hcompute_tree_mu"  in y and ".data" in y
            return ((mu_x and (tree_mem_y or tree_mu_y)) or
                    (mu_y and (tree_mem_x or tree_mu_x)))

        # Remove original direct MU->tree edges
        connections[:] = [c for c in connections if not is_direct_mu2tree(c)]

        # Find scale PE name and create single scale output IO
        scale_pe_name = None
        for instance_name, instance_conf in instances.items():
            if "get_shared_exp" in instance_name:
                scale_pe_name = instance_name
                break
        assert scale_pe_name is not None, "Cannot find scale PE"

        # Collect the pre-hacked single scale-output IO and rename to indexed name
        existing_ios = [n for n in instances.keys() if "io16_hw_scale_output_stencil_op_hcompute_hw_scale_output_stencil_write_0" in n]
        assert len(existing_ios) == 1, "Expect one scale output IO of pre-hacked graph"
        old_io = existing_ios[0]
        old_port = old_io.replace("io16_", "")

        # Remove any old top-level type/connection using the legacy non-indexed port
        type_fields = top_module["type"][1]
        type_fields[:] = [f for f in type_fields if f[0] != old_port]
        connections[:] = [c for c in connections if c[0] != f"self.{old_port}" and c[1] != f"self.{old_port}"]
        connections[:] = [c for c in connections if c[0] != f"{old_io}.out" and c[1] != f"{old_io}.out"]
        connections[:] = [c for c in connections if old_io not in c[0] and old_io not in c[1]]

        # Create single scale output IO
        scale_output_io_name = "io16_hw_scale_output_stencil_clkwrk_0_op_hcompute_hw_scale_output_stencil_0_write_0"
        inst_copy = copy.deepcopy(instances.pop(old_io))
        instances[scale_output_io_name] = inst_copy

        # Add top-level type field and connect self port to IO.out
        scale_port = scale_output_io_name.replace("io16_", "")
        if all(field[0] != scale_port for field in type_fields):
            type_fields.append([scale_port, ["Array", 16, "Bit"]])
        add_conn_once(f"self.{scale_port}", f"{scale_output_io_name}.out")

        # Insert a MEM tile to filter scale output
        mem_scale_filter_name = "mem_scale_filter"
        if mem_scale_filter_name not in instances:
            instances[mem_scale_filter_name] = copy.deepcopy(self.mem_tpl)

        # Connect scale PE -> MEM -> scale output IO
        add_conn_once(f"{scale_pe_name}.O0", f"{mem_scale_filter_name}.data_in_0")
        add_conn_once(f"{mem_scale_filter_name}.data_out_0", f"{scale_output_io_name}.in")

        # Configure GLB store DMA for single scale output
        total_channels = int(self.halide_gen_args_dict["vec_width"])
        img_size = int(self.halide_gen_args_dict["vec_height"])
        mu_OC = int(self.halide_gen_args_dict["mu_i"])
        number_of_blocks = total_channels // (mu_OC * 2)

        # Configure the single scale output IO
        inst_copy["metadata"]["in2glb_0"]["cycle_starting_addr"] = [0]
        inst_copy["metadata"]["in2glb_0"]["cycle_stride"] = [1]
        inst_copy["metadata"]["in2glb_0"]["dimensionality"] = 1
        inst_copy["metadata"]["in2glb_0"]["extent"] = [img_size * number_of_blocks]
        inst_copy["metadata"]["in2glb_0"]["write_data_starting_addr"] = [0]
        inst_copy["metadata"]["in2glb_0"]["write_data_stride"] = [1]

        # Output GLB IO template for unquantized output
        io_tpl = {
            "modref": "global.IO",
            "modargs": {"mode": ["String", "out"]},
            "metadata": {
                "in2glb_0": {
                    "cycle_starting_addr": [0],
                    "cycle_stride": [1],
                    "dimensionality": 1,
                    "extent": [total_channels * img_size // mu_OC],
                    "write_data_starting_addr": [0],
                    "write_data_stride": [1],
                }
            },
        }

        # Collect MU IO sources from connections
        # We'll directly connect MU IOs to output IOs without any apply scale or data packing
        mu_io_sources = {}
        for conn in connections:
            for port in conn:
                # Look for MU IO outputs
                if port.startswith("io16in_mu_") and "op_hcompute_mu_input_io_stencil" in port:
                    # Extract the clkwrk index
                    m = re.search(r"_clkwrk_(\d+)_op_hcompute_mu_input_io_stencil", port)
                    if m:
                        mu_idx = int(m.group(1))
                        mu_io_sources[mu_idx] = port

        # Sort by index
        sorted_mu_indices = sorted(mu_io_sources.keys())

        # Create unquantized output IOs and connect them directly to MU IOs
        unquantized_output_ios = []
        for io_idx, mu_idx in enumerate(sorted_mu_indices):
            io_name = f"io16_unquantized_output_stencil_clkwrk_{io_idx}_op_hcompute_unquantized_output_stencil_{io_idx}_write_0"
            if io_name not in instances:
                instances[io_name] = copy.deepcopy(io_tpl)

            # Connect MU IO directly to output IO
            add_conn_once(mu_io_sources[mu_idx], f"{io_name}.in")
            unquantized_output_ios.append(io_name)

        # Type instances of self.<port> to corresponding IO.out
        type_fields = top_module["type"][1]
        for io_inst in unquantized_output_ios:
            port_name = io_inst.replace("io16_", "")
            type_fields.append([port_name, ["Array", 16, "Bit"]])
            add_conn_once(f"self.{port_name}", f"{io_inst}.out")

        # Overwrite the JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

        # Update design_meta_halide.json to update mu_inputs shape and add unquantized_output_stencil
        design_meta_path = os.path.join(bin_path, "design_meta_halide.json")
        with open(design_meta_path, "r") as f:
            design_meta = json.load(f)
        assert len(design_meta["IOs"]["inputs"]) == 1, "Expected only one input which is mu input"
        # Real input shape should match vec_width instead of vec_width_fake
        design_meta["IOs"]["inputs"][0]["shape"][0] = int(self.halide_gen_args_dict["vec_width"])
        # Add unquantized_output_stencil
        design_meta["IOs"]["outputs"].append(
            {
                "bitwidth": 16,
                "datafile": "unquantized_output_stencil.raw",
                "name": "unquantized_output_stencil",
                "shape": [len(unquantized_output_ios), img_size * total_channels // len(unquantized_output_ios)],
            }
        )
        # Fix the name and shape of hw_scale_output_stencil
        for output in design_meta["IOs"]["outputs"]:
            if output["name"] == "hw_scale_output_stencil":
                output["datafile"] = "hw_scale_output.raw"
                output["shape"] = [img_size * number_of_blocks]
                break

        with open(design_meta_path, "w") as f:
            json.dump(design_meta, f, indent=2)

    def hack_for_get_e8m0_scale_tree_gb_input_rv(self, json_path, bin_path):
        '''
        This hack creates two paths for GLB IOs:
        1. GLB IOs -> tree_glb (direct connection, no delay)
        2. GLB IOs -> shift fifo -> tree_fifo (with delay)
        '''
        with open(json_path, "r") as f:
            design = json.load(f)

        top_module_name = "get_e8m0_scale_tree_gb_input"
        instances = design["namespaces"]["global"]["modules"][top_module_name]["instances"]
        top_module = design["namespaces"]["global"]["modules"][top_module_name]
        connections = top_module["connections"]

        def clkwrk_idx_from_port(p):
            # Helper function to extract GLB IO clkwrk index from port name
            m = re.search(r"_clkwrk_(\d+)_op_hcompute_input_gb_stencil", p)
            return int(m.group(1)) if m else None

        # Helper function to add a connection only if it doesn't already exist
        def add_conn_once(src, dst):
            pair = [src, dst]
            if pair not in connections:
                connections.append(pair)

        # Collect original GLB->tree connections and create new routing
        glb_to_tree_fifo = []  # Will get shift FIFOs
        glb_to_tree_glb = []   # Direct connections

        for a, b in list(connections):
            is_glb_a = a.startswith("io16in_")
            is_glb_b = b.startswith("io16in_")
            is_tree_fifo_a = "op_hcompute_tree_fifo" in a and ".data" in a
            is_tree_fifo_b = "op_hcompute_tree_fifo" in b and ".data" in b
            is_tree_glb_a  = "op_hcompute_tree_glb"  in a and ".data" in a
            is_tree_glb_b  = "op_hcompute_tree_glb"  in b and ".data" in b

            if is_glb_a and is_tree_fifo_b:
                glb_to_tree_fifo.append((a, b))
            elif is_glb_b and is_tree_fifo_a:
                glb_to_tree_fifo.append((b, a))
            elif is_glb_a and is_tree_glb_b:
                glb_to_tree_glb.append((a, b))
            elif is_glb_b and is_tree_glb_a:
                glb_to_tree_glb.append((b, a))

        # Remove original direct GLB->tree edges
        def is_direct_glb2tree(edge):
            x, y = edge
            glb_x = x.startswith("io16in_")
            glb_y = y.startswith("io16in_")
            tree_fifo_x = "op_hcompute_tree_fifo" in x and ".data" in x
            tree_fifo_y = "op_hcompute_tree_fifo" in y and ".data" in y
            tree_glb_x  = "op_hcompute_tree_glb"  in x and ".data" in x
            tree_glb_y  = "op_hcompute_tree_glb"  in y and ".data" in y
            return ((glb_x and (tree_fifo_y or tree_glb_y)) or
                    (glb_y and (tree_fifo_x or tree_glb_x)))

        connections[:] = [c for c in connections if not is_direct_glb2tree(c)]

        # Create direct connections: GLB IOs -> tree_glb
        for glb_port, tree_glb_port in glb_to_tree_glb:
            add_conn_once(glb_port, tree_glb_port)

        # Create shift FIFO connections: GLB IOs -> shift_fifo -> tree_fifo
        for glb_port, tree_fifo_port in glb_to_tree_fifo:
            glb_idx = clkwrk_idx_from_port(glb_port)
            assert glb_idx is not None, f"Cannot parse clkwrk index from GLB port: {glb_port}"

            # Create shift FIFO instance
            fifo_name = f"shift_fifo_glb2tree_{glb_idx}"
            if fifo_name not in instances:
                instances[fifo_name] = copy.deepcopy(self.shift_fifo_tpl)

            # Connect GLB -> shift_fifo -> tree_fifo
            add_conn_once(glb_port, f"{fifo_name}.in")
            add_conn_once(f"{fifo_name}.out", tree_fifo_port)

        # Find scale PE name and create single scale output IO
        scale_pe_name = None
        for instance_name, instance_conf in instances.items():
            if "get_shared_exp" in instance_name:
                scale_pe_name = instance_name
                break
        assert scale_pe_name is not None, "Cannot find scale PE"

        # Collect the pre-hacked single scale-output IO and rename to indexed name
        existing_ios = [n for n in instances.keys() if "io16_hw_scale_output_stencil_op_hcompute_hw_scale_output_stencil_write_0" in n]
        assert len(existing_ios) == 1, "Expect one scale output IO of pre-hacked graph"
        old_io = existing_ios[0]
        old_port = old_io.replace("io16_", "")

        # Remove any old top-level type/connection using the legacy non-indexed port
        type_fields = top_module["type"][1]
        type_fields[:] = [f for f in type_fields if f[0] != old_port]
        connections[:] = [c for c in connections if c[0] != f"self.{old_port}" and c[1] != f"self.{old_port}"]
        connections[:] = [c for c in connections if c[0] != f"{old_io}.out" and c[1] != f"{old_io}.out"]
        connections[:] = [c for c in connections if old_io not in c[0] and old_io not in c[1]]

        # Create single scale output IO
        scale_output_io_name = "io16_hw_scale_output_stencil_clkwrk_0_op_hcompute_hw_scale_output_stencil_0_write_0"
        inst_copy = copy.deepcopy(instances.pop(old_io))
        instances[scale_output_io_name] = inst_copy

        # Add top-level type field and connect self port to IO.out
        scale_port = scale_output_io_name.replace("io16_", "")
        if all(field[0] != scale_port for field in type_fields):
            type_fields.append([scale_port, ["Array", 16, "Bit"]])
        add_conn_once(f"self.{scale_port}", f"{scale_output_io_name}.out")

        # Connect scale PE directly to scale output IO
        add_conn_once(f"{scale_pe_name}.O0", f"{scale_output_io_name}.in")

        # Configure GLB store DMA for single scale output
        total_channels = int(self.halide_gen_args_dict["vec_width"])
        img_size = int(self.halide_gen_args_dict["vec_height"])
        mu_OC = int(self.halide_gen_args_dict["glb_i"])
        number_of_blocks = total_channels // (mu_OC * 2)

        # Configure the single scale output IO
        inst_copy["metadata"]["in2glb_0"]["cycle_starting_addr"] = [1]
        inst_copy["metadata"]["in2glb_0"]["cycle_stride"] = [2]
        inst_copy["metadata"]["in2glb_0"]["dimensionality"] = 1
        inst_copy["metadata"]["in2glb_0"]["extent"] = [img_size * number_of_blocks]
        inst_copy["metadata"]["in2glb_0"]["write_data_starting_addr"] = [0]
        inst_copy["metadata"]["in2glb_0"]["write_data_stride"] = [1]

        # Configure input IOs schedule - each IO linearly takes in data
        for inst_name, inst_conf in instances.items():
            if inst_name.startswith("io16in_input_host_stencil_"):
                if "metadata" in inst_conf and "glb2out_0" in inst_conf["metadata"]:
                    inst_conf["metadata"]["glb2out_0"]["cycle_starting_addr"] = [0]
                    inst_conf["metadata"]["glb2out_0"]["cycle_stride"] = [1]
                    inst_conf["metadata"]["glb2out_0"]["dimensionality"] = 1
                    inst_conf["metadata"]["glb2out_0"]["extent"] = [total_channels // mu_OC * img_size]
                    inst_conf["metadata"]["glb2out_0"]["read_data_starting_addr"] = [0]
                    inst_conf["metadata"]["glb2out_0"]["read_data_stride"] = [1]

        # Overwrite the JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

        # Update design_meta_halide.json to update input shape
        design_meta_path = os.path.join(bin_path, "design_meta_halide.json")
        with open(design_meta_path, "r") as f:
            design_meta = json.load(f)
        assert len(design_meta["IOs"]["inputs"]) == 1, "Expected only one input"
        # Real input shape should match vec_width instead of vec_width_fake
        design_meta["IOs"]["inputs"][0]["shape"][0] = int(self.halide_gen_args_dict["vec_width"])
        # Fix the name and shape of hw_scale_output_stencil
        for output in design_meta["IOs"]["outputs"]:
            if output["name"] == "hw_scale_output_stencil":
                output["datafile"] = "hw_scale_output.raw"
                output["shape"] = [img_size * number_of_blocks]
                break

        with open(design_meta_path, "w") as f:
            json.dump(design_meta, f, indent=2)

    def hack_for_get_e8m0_scale_accum_gb_input_rv(self, json_path, bin_path):
        with open(json_path, "r") as f:
            design = json.load(f)

        top_module_name = "get_e8m0_scale_accum_gb_input"
        top_module = design["namespaces"]["global"]["modules"][top_module_name]
        instances = top_module["instances"]
        original_connections = top_module["connections"]

        abs_max_pe_data1_re = re.compile(
            r"^(op_hcompute_abs_max_stencil_(?P<idx>\d+)\$inner_compute\$float_DW_fp_add_[^\.]+\.)(?P<pin>data1)$"
        )
        abs_max_pond_out_re = re.compile(
            r"^abs_max_stencil\$ub_abs_max_stencil_BANK_(?P<bank>\d+)_garnet\.data_out_pond_1$"
        )
        packer_data_port_re = re.compile(
            r"^op_hcompute_output_cgra_stencil(_\d+)?\$inner_compute\$bit8_pack_[^\.]+\.data[01]$"
        )
        packer_o0_re = re.compile(
            r"^(op_hcompute_output_cgra_stencil(_\d+)?\$inner_compute\$bit8_pack_[^\.]+)\.O0$"
        )
        output_pond_data_in_re = re.compile(
            r"^(output_cgra_stencil\$ub_output_cgra_stencil_BANK_\d+_garnet)\.data_in_pond_0$"
        )
        output_pond_data_out_re = re.compile(
            r"^(output_cgra_stencil\$ub_output_cgra_stencil_BANK_\d+_garnet)\.data_out_pond_0$"
        )

        lane_input_sources = {}
        lane_final_sinks = {}
        packer_sinks_to_remove = set()
        packer_to_output_pond = {}
        output_pond_to_io = {}

        for left, right in original_connections:
            for endpoint, other in ((left, right), (right, left)):
                pe_match = abs_max_pe_data1_re.match(endpoint)
                if pe_match:
                    lane_idx = int(pe_match.group("idx")) - 1
                    lane_input_sources.setdefault(lane_idx, other)
                packer_match = packer_o0_re.match(endpoint)
                if packer_match:
                    pack_inst = packer_match.group(1)
                    pond_match = output_pond_data_in_re.match(other)
                    if pond_match:
                        packer_to_output_pond[pack_inst] = pond_match.group(1)
            pond_match = abs_max_pond_out_re.match(left)
            if pond_match and packer_data_port_re.match(right):
                lane_idx = int(pond_match.group("bank"))
                lane_final_sinks[lane_idx] = right
                packer_sinks_to_remove.add(right)
            pond_match = abs_max_pond_out_re.match(right)
            if pond_match and packer_data_port_re.match(left):
                lane_idx = int(pond_match.group("bank"))
                lane_final_sinks[lane_idx] = left
                packer_sinks_to_remove.add(left)
            pond_out_match = output_pond_data_out_re.match(left)
            if pond_out_match and right.endswith(".in") and right.startswith("io"):
                output_pond_to_io[pond_out_match.group(1)] = right
            pond_out_match = output_pond_data_out_re.match(right)
            if pond_out_match and left.endswith(".in") and left.startswith("io"):
                output_pond_to_io[pond_out_match.group(1)] = left

        expected_lane_count = len(lane_final_sinks)
        if len(lane_input_sources) != expected_lane_count:
            raise ValueError(
                f"Expected {expected_lane_count} lane input sources, found {len(lane_input_sources)}."
            )
        if len(lane_final_sinks) != expected_lane_count:
            raise ValueError(
                f"Expected {expected_lane_count} lane final sinks, found {len(lane_final_sinks)}."
            )

        packer_to_io = {}
        for pack_inst, pond_inst in packer_to_output_pond.items():
            io_sink = output_pond_to_io.get(pond_inst)
            if io_sink:
                packer_to_io[pack_inst] = io_sink

        def endpoint_instance(endpoint: str) -> str:
            return endpoint.split(".")[0]

        # Collect instances to delete (old single-accumulator implementation)
        instances_to_delete = set()
        for inst_name in list(instances.keys()):
            if "op_hcompute_abs_max_stencil$inner_compute$const" in inst_name:
                instances_to_delete.add(inst_name)
            elif inst_name.startswith("op_hcompute_abs_max_stencil_") and "$inner_compute$float_DW_fp_add" in inst_name:
                instances_to_delete.add(inst_name)
            elif inst_name.startswith("op_hcompute_abs_max_stencil_") and "$inner_compute$c" in inst_name:
                instances_to_delete.add(inst_name)
            elif inst_name.startswith("abs_max_stencil$ub_abs_max_stencil_BANK_"):
                instances_to_delete.add(inst_name)
            elif inst_name.startswith("output_cgra_stencil$ub_output_cgra_stencil_"):
                instances_to_delete.add(inst_name)

        for idx, src in lane_input_sources.items():
            if endpoint_instance(src) in instances_to_delete:
                raise ValueError(f"Lane {idx} input source '{src}' would be deleted; cannot build dual accum.")

        for inst_name in instances_to_delete:
            instances.pop(inst_name, None)

        cleaned_connections = []
        for left, right in original_connections:
            if endpoint_instance(left) in instances_to_delete or endpoint_instance(right) in instances_to_delete:
                continue
            if left in packer_sinks_to_remove or right in packer_sinks_to_remove:
                continue
            cleaned_connections.append([left, right])

        connections = []

        def add_conn_once(src: str, dst: str):
            pair = [src, dst]
            if pair not in connections:
                connections.append(pair)

        for left, right in cleaned_connections:
            add_conn_once(left, right)

        shared_clk_const_name = f"{top_module_name}_clk_en_const"
        if shared_clk_const_name not in instances:
            instances[shared_clk_const_name] = copy.deepcopy(self.const_clk_tpl)

        lane_bypass_cfg = {}
        for lane_idx in sorted(lane_final_sinks.keys()):
            lane_prefix = f"{top_module_name}_lane_{lane_idx}"
            filter_mem_name = f"{lane_prefix}_filter_mem"
            accum_pond_0_name = f"{lane_prefix}_accum_pond_0"
            accum_pond_1_name = f"{lane_prefix}_accum_pond_1"
            accum_pe_0_name = f"{lane_prefix}_accum_pe_0"
            accum_pe_1_name = f"{lane_prefix}_accum_pe_1"
            final_reduce_pe_name = f"{lane_prefix}_final_reduce_pe"
            get_shared_exp_pe_name = f"{lane_prefix}_get_shared_exp_pe"
            accum_pe_0_const = f"{lane_prefix}_accum_pe_0_inst_const"
            accum_pe_1_const = f"{lane_prefix}_accum_pe_1_inst_const"
            final_reduce_pe_const = f"{lane_prefix}_final_reduce_pe_inst_const"
            get_shared_exp_pe_const = f"{lane_prefix}_get_shared_exp_pe_inst_const"

            if filter_mem_name not in instances:
                mem_inst = copy.deepcopy(self.mem_tpl)
                mem_inst["genargs"]["ID"][1] = filter_mem_name
                instances[filter_mem_name] = mem_inst
            if accum_pond_0_name not in instances:
                pond0_inst = copy.deepcopy(self.pond_tpl)
                pond0_inst["genargs"]["ID"][1] = accum_pond_0_name
                instances[accum_pond_0_name] = pond0_inst
            if accum_pond_1_name not in instances:
                pond1_inst = copy.deepcopy(self.pond_tpl)
                pond1_inst["genargs"]["ID"][1] = accum_pond_1_name
                instances[accum_pond_1_name] = pond1_inst
            if accum_pe_0_name not in instances:
                instances[accum_pe_0_name] = copy.deepcopy(self.pe_tpl)
            if accum_pe_1_name not in instances:
                instances[accum_pe_1_name] = copy.deepcopy(self.pe_tpl)
            if final_reduce_pe_name not in instances:
                instances[final_reduce_pe_name] = copy.deepcopy(self.pe_tpl)
            if get_shared_exp_pe_name not in instances:
                instances[get_shared_exp_pe_name] = copy.deepcopy(self.pe_tpl)
            if accum_pe_0_const not in instances:
                instances[accum_pe_0_const] = copy.deepcopy(self.abs_max_const_tpl)
            if accum_pe_1_const not in instances:
                instances[accum_pe_1_const] = copy.deepcopy(self.abs_max_const_tpl)
            if final_reduce_pe_const not in instances:
                instances[final_reduce_pe_const] = copy.deepcopy(self.abs_max_const_tpl)
            if get_shared_exp_pe_const not in instances:
                instances[get_shared_exp_pe_const] = copy.deepcopy(self.get_shared_exp_const_tpl)

            lane_input_src = lane_input_sources[lane_idx]
            final_sink = lane_final_sinks[lane_idx]
            final_sink_base = final_sink.rsplit(".", 1)[0]
            packer_to_io_sink = packer_to_io.get(final_sink_base)
            if not packer_to_io_sink:
                raise ValueError(f"No IO sink found for packer '{final_sink_base}'.")

            add_conn_once(lane_input_src, f"{filter_mem_name}.data_in_0")

            add_conn_once(f"{shared_clk_const_name}.out", f"{filter_mem_name}.clk_en")
            add_conn_once(f"{shared_clk_const_name}.out", f"{accum_pond_0_name}.clk_en")
            add_conn_once(f"{shared_clk_const_name}.out", f"{accum_pond_1_name}.clk_en")

            add_conn_once(f"{filter_mem_name}.data_out_0", f"{accum_pe_0_name}.data1")
            add_conn_once(f"{filter_mem_name}.data_out_1", f"{accum_pe_1_name}.data1")

            add_conn_once(f"{accum_pe_0_const}.out", f"{accum_pe_0_name}.inst")
            add_conn_once(f"{accum_pe_1_const}.out", f"{accum_pe_1_name}.inst")
            add_conn_once(f"{final_reduce_pe_const}.out", f"{final_reduce_pe_name}.inst")
            add_conn_once(f"{get_shared_exp_pe_const}.out", f"{get_shared_exp_pe_name}.inst")

            add_conn_once(f"{accum_pond_0_name}.data_out_pond_0", f"{accum_pe_0_name}.data0")
            add_conn_once(f"{accum_pe_0_name}.O0", f"{accum_pond_0_name}.data_in_pond_0")
            add_conn_once(f"{accum_pond_0_name}.data_out_pond_1", f"{final_reduce_pe_name}.data0")

            add_conn_once(f"{accum_pond_1_name}.data_out_pond_0", f"{accum_pe_1_name}.data0")
            add_conn_once(f"{accum_pe_1_name}.O0", f"{accum_pond_1_name}.data_in_pond_0")
            add_conn_once(f"{accum_pond_1_name}.data_out_pond_1", f"{final_reduce_pe_name}.data1")

            add_conn_once(f"{final_reduce_pe_name}.O0", f"{get_shared_exp_pe_name}.data0")
            add_conn_once(f"{get_shared_exp_pe_name}.O0", final_sink)
            add_conn_once(f"{final_sink_base}.O0", packer_to_io_sink)

            lane_bypass_cfg[accum_pe_0_name] = {"input_fifo_bypass": [0, 0, 0], "output_fifo_bypass": 1}
            lane_bypass_cfg[accum_pe_1_name] = {"input_fifo_bypass": [0, 0, 0], "output_fifo_bypass": 1}

        top_module["connections"] = connections

        # Update input IO metadata
        head_dim = int(self.halide_gen_args_dict["head_dim"])
        seq_heads_prod = int(self.halide_gen_args_dict["seq_heads_prod"])
        glb_i = int(self.halide_gen_args_dict["glb_i"])
        block_size = 64

        for inst_name, inst_config in instances.items():
            if "io16in_input_host_stencil" in inst_name and inst_config.get("modref") == "global.IO":
                md = inst_config["metadata"]["glb2out_0"]
                md["cycle_starting_addr"] = [0]
                md["cycle_stride"] = [1, 1, 1]
                md["dimensionality"] = 3
                md["extent"] = [block_size, head_dim // glb_i, seq_heads_prod // block_size]
                md["read_data_starting_addr"] = [0]
                read_stride_0 = head_dim // glb_i
                read_stride_1 = 1 - read_stride_0 * (block_size - 1)
                read_stride_2 = 1
                md["read_data_stride"] = [read_stride_0, read_stride_1, read_stride_2]

        # Update output IO metadata
        for inst_name, inst_config in instances.items():
            if "io16_hw_output_stencil" in inst_name and inst_config.get("modref") == "global.IO":
                md = inst_config["metadata"]["in2glb_0"]
                md["cycle_starting_addr"] = [0]
                md["cycle_stride"] = [1]
                md["dimensionality"] = 1
                md["extent"] = [head_dim * (seq_heads_prod // block_size) // glb_i]
                md["write_data_starting_addr"] = [0]
                md["write_data_stride"] = [1]

        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

        design_meta_path = os.path.join(bin_path, "design_meta_halide.json")
        if os.path.exists(design_meta_path):
            with open(design_meta_path, "r") as f:
                design_meta = json.load(f)

            inputs = design_meta.get("IOs", {}).get("inputs", [])
            outputs = design_meta.get("IOs", {}).get("outputs", [])
            assert len(inputs) == 1, "Expect exactly one input in design_meta_halide.json"
            assert len(outputs) == 1, "Expect exactly one output in design_meta_halide.json"

            inputs[0]["shape"] = [head_dim, seq_heads_prod]
            outputs[0]["shape"] = [head_dim // 2, seq_heads_prod // block_size]

            with open(design_meta_path, "w") as f:
                json.dump(design_meta, f, indent=2)

        bypass_path = os.path.join(bin_path, "PE_fifos_bypass_config.json")
        with open(bypass_path, "w") as f:
            json.dump(lane_bypass_cfg, f, indent=2)

    def hack_for_apply_e8m0_scale_single_IO_rv(self, json_path, bin_path):
        with open(json_path, "r") as f:
            design = json.load(f)

        top_module_name = "apply_e8m0_scale_single_IO"
        modules = design["namespaces"]["global"]["modules"]
        if top_module_name not in modules:
            raise KeyError(f"Top module '{top_module_name}' not found in design JSON.")

        top_module = modules[top_module_name]
        instances = top_module["instances"]
        connections = top_module["connections"]
        type_fields = top_module["type"][1]

        def add_conn_once(src, dst):
            pair = [src, dst]
            if pair not in connections:
                connections.append(pair)

        # Collapse input scale IOs down to a single broadcaster.
        scale_io_prefix = "io16in_input_scale_host_stencil"
        scale_io_instances = [
            name for name in instances.keys() if name.startswith(scale_io_prefix)
        ]
        if not scale_io_instances:
            raise RuntimeError("No input scale IO instances found to collapse.")

        def scale_io_index(name: str) -> int:
            match = re.search(r"clkwrk_(\d+)", name)
            return int(match.group(1)) if match else 0

        scale_io_keep = min(scale_io_instances, key=scale_io_index)

        # Remove redundant input scale IO instances and their related ports/connections.
        removal_tokens = []
        for inst_name in scale_io_instances:
            if inst_name == scale_io_keep:
                continue
            instances.pop(inst_name, None)

            data_port = inst_name.replace("io16in_", "")
            read_en_port = data_port.replace("_read_0", "_read_en")
            removal_tokens.extend(
                [
                    inst_name,
                    f"self.{data_port}",
                    f"self.{read_en_port}",
                ]
            )
            type_fields[:] = [
                field
                for field in type_fields
                if field[0] not in {data_port, read_en_port}
            ]

        if removal_tokens:
            filtered_connections = []
            for src, dst in connections:
                if any(token in src or token in dst for token in removal_tokens):
                    continue
                filtered_connections.append([src, dst])
            connections[:] = filtered_connections

        # Rebuild e8m0 quant PE scale broadcast so every data1 port sees the kept IO.
        e8m0_quant_instances = [
            name for name in instances if "$inner_compute$e8m0_quant" in name
        ]
        assert (
            e8m0_quant_instances
        ), "Expected to find e8m0 quant PEs, but none were detected."

        quant_tokens = {f"{name}.data1" for name in e8m0_quant_instances}
        filtered_connections = []
        for src, dst in connections:
            if any(token in src or token in dst for token in quant_tokens):
                continue
            filtered_connections.append([src, dst])
        connections[:] = filtered_connections
        for quant in e8m0_quant_instances:
            add_conn_once(f"{quant}.data1", f"{scale_io_keep}.out")

        # Create scale packing path: IO -> (direct + FIFO) -> data packing PE -> new output IOs.
        scale_fifo_name = "shift_fifo_input_scale_broadcast"
        if scale_fifo_name not in instances:
            instances[scale_fifo_name] = copy.deepcopy(self.shift_fifo_tpl)

        scale_packing_pe = "op_hcompute_hw_scale_packed_output$inner_compute$data_packing"
        scale_packing_const = "op_hcompute_hw_scale_packed_output$inner_compute$c0"
        if scale_packing_pe not in instances:
            instances[scale_packing_pe] = copy.deepcopy(self.pe_tpl)
        if scale_packing_const not in instances:
            instances[scale_packing_const] = copy.deepcopy(self.data_packing_const_tpl)
        add_conn_once(f"{scale_packing_pe}.inst", f"{scale_packing_const}.out")

        add_conn_once(f"{scale_io_keep}.out", f"{scale_packing_pe}.data0")
        add_conn_once(f"{scale_io_keep}.out", f"{scale_fifo_name}.in")
        add_conn_once(f"{scale_fifo_name}.out", f"{scale_packing_pe}.data1")

        # Instantiate two output IOs for packed scale data.
        scale_output_ios = [
            (
                "io16_hw_scale_packed_output_stencil_clkwrk_0_op_hcompute_hw_scale_packed_output_stencil_0_write_0",
                "hw_scale_packed_output_stencil_clkwrk_0_op_hcompute_hw_scale_packed_output_stencil_0_write_0",
            ),
            (
                "io16_hw_scale_packed_output_stencil_clkwrk_1_op_hcompute_hw_scale_packed_output_stencil_1_write_1",
                "hw_scale_packed_output_stencil_clkwrk_1_op_hcompute_hw_scale_packed_output_stencil_1_write_1",
            ),
        ]

        for idx, (inst_name, port_name) in enumerate(scale_output_ios):
            if inst_name not in instances:
                instances[inst_name] = copy.deepcopy(self.output_io_tpl)

            if all(field[0] != port_name for field in type_fields):
                type_fields.append([port_name, ["Array", 16, "Bit"]])

            add_conn_once(f"self.{port_name}", f"{inst_name}.out")
            num_pipeline_fifos = 10
            if idx == 0 and num_pipeline_fifos > 0:
                upstream = f"{scale_packing_pe}.O0"
                for fifo_idx in range(num_pipeline_fifos):
                    fifo_name = f"pipeline_fifo_scale_packed_output_{fifo_idx}"
                    if fifo_name not in instances:
                        instances[fifo_name] = copy.deepcopy(self.pipeline_fifo_tpl)
                    add_conn_once(upstream, f"{fifo_name}.in")
                    upstream = f"{fifo_name}.out"
                add_conn_once(upstream, f"{inst_name}.in")
            else:
                add_conn_once(f"{scale_packing_pe}.O0", f"{inst_name}.in")

        # Reorder IO instances to keep category-wise indexing monotonic.
        io_category_patterns = OrderedDict(
            [
                (
                    "input_bf_act_host",
                    re.compile(
                        r"io16in_input_bf_act_host_stencil_clkwrk_(\d+)_op_hcompute_input_bf_act_glb_stencil(?:_(\d+))?_read_0"
                    ),
                ),
                (
                    "input_scale_host",
                    re.compile(
                        r"io16in_input_scale_host_stencil_clkwrk_(\d+)_op_hcompute_input_scale_glb_stencil(?:_(\d+))?_read_0"
                    ),
                ),
                (
                    "hw_output_mxint8_act",
                    re.compile(
                        r"io16_hw_output_mxint8_act_stencil_clkwrk_(\d+)_op_hcompute_hw_output_mxint8_act_stencil(?:_(\d+))?_write_0"
                    ),
                ),
                (
                    "hw_scale_packed_output",
                    re.compile(
                        r"io16_hw_scale_packed_output_stencil_clkwrk_(\d+)_op_hcompute_hw_scale_packed_output_stencil_(\d+)_write_(\d+)"
                    ),
                ),
            ]
        )

        category_entries = {cat: [] for cat in io_category_patterns}
        other_entries = []
        for name, inst in instances.items():
            matched = False
            for cat, pattern in io_category_patterns.items():
                m = pattern.match(name)
                if m:
                    idx = tuple(int(g) for g in m.groups() if g is not None)
                    category_entries[cat].append((idx, name, inst))
                    matched = True
                    break
            if not matched:
                other_entries.append((name, inst))

        reordered_instances = OrderedDict()
        for name, inst in other_entries:
            reordered_instances[name] = inst
        for cat in io_category_patterns:
            entries = sorted(category_entries[cat], key=lambda x: x[0])
            for _, name, inst in entries:
                reordered_instances[name] = inst

        top_module["instances"] = reordered_instances
        instances = top_module["instances"]

        num_pixels = int(self.halide_gen_args_dict["vec_height"])
        num_channels = int(self.halide_gen_args_dict["vec_width"])
        glb_i = int(self.halide_gen_args_dict["glb_i"])
        num_blocks = num_channels // (glb_i * 2) if glb_i != 0 else 0
        glb_io_metadata = {
            "input_bf_act_host": (
                "glb2out_0",
                {
                    "cycle_starting_addr": [0],
                    "cycle_stride": [1],
                    "dimensionality": 1,
                    "extent": [num_pixels * num_channels // int(self.halide_gen_args_dict["glb_i"])],
                    "read_data_starting_addr": [0],
                    "read_data_stride": [1],
                },
            ),
            "input_scale_host": (
                "glb2out_0",
                {
                    "cycle_starting_addr": [0],
                    "cycle_stride": [1, 1],
                    "dimensionality": 2,
                    "extent": [2, num_pixels * num_blocks],
                    "read_data_starting_addr": [0],
                    "read_data_stride": [0, 1],
                },
            ),
            "hw_output_mxint8_act": (
                "in2glb_0",
                {
                    "cycle_starting_addr": [0],
                    "cycle_stride": [1],
                    "dimensionality": 1,
                    # Divided by glb_i because of data packing
                    "extent": [num_pixels * num_channels // glb_i],
                    "write_data_starting_addr": [0],
                    "write_data_stride": [1],
                },
            ),
            "hw_scale_packed_output": (
                "in2glb_0",
                {
                    "cycle_starting_addr": [0],
                    "cycle_stride": [2],
                    "dimensionality": 1,
                    "extent": [num_pixels * num_blocks // 2 if num_blocks else 0],
                    "write_data_starting_addr": [0],
                    "write_data_stride": [1],
                },
            ),
        }

        for name, inst_conf in instances.items():
            matched_category = None
            for cat in io_category_patterns:
                if category_entries[cat] and any(entry[1] == name for entry in category_entries[cat]):
                    matched_category = cat
                    break
            if matched_category is None:
                continue

            metadata = inst_conf.setdefault("metadata", {})
            md_key, md_template = glb_io_metadata.get(matched_category, (None, None))
            if md_key is None:
                continue
            metadata[md_key] = copy.deepcopy(md_template)

        hw_scale_entries = sorted(
            category_entries["hw_scale_packed_output"], key=lambda x: x[0]
        )
        if hw_scale_entries:
            num_scale_ios = len(hw_scale_entries)
            assert (
                num_blocks > 0 and num_scale_ios > 0
            ), "Scale output metadata requires positive blocks and IO count."

            # Divided by 2 because of data packing
            extent_per_scale_io = num_pixels * num_blocks // 2 // num_scale_ios

            for idx, (_, inst_name, _) in enumerate(hw_scale_entries):
                md = instances[inst_name]["metadata"]["in2glb_0"]
                # Direct Stream:  0,0,1,1,2,2,3,3,4,...
                # Shifted Stream: x,0,0,1,1,2,2,3,3,...
                md["cycle_starting_addr"] = [2 + idx * 4]
                md["cycle_stride"] = [num_scale_ios * 4]
                md["dimensionality"] = 1
                md["extent"] = [extent_per_scale_io]
                md["write_data_starting_addr"] = [0]
                md["write_data_stride"] = [1]

        # Persist modifications.
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

        # Update design_meta_halide.json with accurate IO metadata for RV flow.
        design_meta_path = os.path.join(bin_path, "design_meta_halide.json")
        if os.path.exists(design_meta_path):
            with open(design_meta_path, "r") as f:
                design_meta = json.load(f)

            ios = design_meta.setdefault("IOs", {})
            inputs = ios.setdefault("inputs", [])
            outputs = ios.setdefault("outputs", [])

            # Update input shapes
            for entry in inputs:
                if entry.get("name") == "input_bf_act_host_stencil":
                    entry["shape"] = [num_channels, num_pixels]
                elif entry.get("name") == "input_scale_host_stencil":
                    entry["shape"] = [num_blocks, num_pixels]

            # Update / append outputs
            hw_output_entry = None
            scale_output_entry = None
            for entry in outputs:
                if entry.get("name") == "hw_output_mxint8_act_stencil":
                    hw_output_entry = entry
                elif entry.get("name") == "hw_scale_packed_output_stencil":
                    scale_output_entry = entry

            if hw_output_entry is not None:
                hw_output_entry["datafile"] = "hw_output_mxint8_act.raw"
                hw_output_entry["shape"] = [num_channels // 2, num_pixels]

            if scale_output_entry is None:
                scale_output_shape = [num_blocks // 2, num_pixels] if num_blocks >= 2 else [num_pixels // 2]
                scale_output_entry = {
                    "bitwidth": 16,
                    "datafile": "hw_scale_packed_output.raw",
                    "name": "hw_scale_packed_output_stencil",
                    "shape": scale_output_shape,
                }
                outputs.append(scale_output_entry)
            else:
                scale_output_entry["datafile"] = "hw_scale_packed_output.raw"
                scale_output_entry["shape"] = scale_output_shape

            with open(design_meta_path, "w") as f:
                json.dump(design_meta, f, indent=2)

    def hack_for_apply_e8m0_scale_multi_IOs_rv(self, json_path, bin_path):
        with open(json_path, "r") as f:
            design = json.load(f)

        modules = design["namespaces"]["global"]["modules"]
        top_module_name = "apply_e8m0_scale_multi_IOs"
        if top_module_name not in modules:
            raise KeyError(f"Top module '{top_module_name}' not found in design JSON.")

        instances = modules[top_module_name]["instances"]

        head_dim = int(self.halide_gen_args_dict["head_dim"])
        seq_heads_prod = int(self.halide_gen_args_dict["seq_heads_prod"])
        glb_i = int(self.halide_gen_args_dict["glb_i"])
        block_size = int(self.halide_gen_args_dict.get("block_size", 64))

        for inst_name, inst_config in instances.items():
            if inst_config.get("modref") != "global.IO":
                continue

            metadata = inst_config.setdefault("metadata", {})
            if "io16in_input_bf_act_host_stencil" in inst_name:
                md = metadata.setdefault("glb2out_0", {})
                md["cycle_starting_addr"] = [0]
                md["cycle_stride"] = [1]
                md["dimensionality"] = 1
                md["extent"] = [(head_dim * seq_heads_prod) // glb_i]
                md["read_data_starting_addr"] = [0]
                md["read_data_stride"] = [1]
            elif "io16in_input_scale_host_stencil" in inst_name:
                md = metadata.setdefault("glb2out_0", {})
                md["cycle_starting_addr"] = [0]
                md["cycle_stride"] = [1, 1, 1]
                md["dimensionality"] = 3
                md["extent"] = [head_dim // glb_i, block_size, seq_heads_prod // block_size]
                md["read_data_starting_addr"] = [0]
                md["read_data_stride"] = [1, 1 - (head_dim // glb_i), 1]
            elif "io16_hw_output_mxint8_act_stencil" in inst_name:
                md = metadata.setdefault("in2glb_0", {})
                md["cycle_starting_addr"] = [0]
                md["cycle_stride"] = [1]
                md["dimensionality"] = 1
                md["extent"] = [(head_dim * seq_heads_prod) // glb_i]
                md["write_data_starting_addr"] = [0]
                md["write_data_stride"] = [1]

        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

        design_meta_path = os.path.join(bin_path, "design_meta_halide.json")
        if os.path.exists(design_meta_path):
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

    def hack_for_get_apply_e8m0_scale_fp_rv(self, json_path, bin_path):
        '''
        This hack inserts MEM tiles between MU IOs and tree_mem / tree_mu PEs.
        MEM tile output port 0 goes to tree_mem PEs, with a delay of image size.
        MEM tile output port 1 goes to tree_mu PEs, without delay.
        It also inserts apply-scale pipeline and scale broadcast for fused quantization.
        '''

        with open(json_path, "r") as f:
            design = json.load(f)

        top_module_name = "get_apply_e8m0_scale_fp"
        instances = design["namespaces"]["global"]["modules"][top_module_name]["instances"]
        top_module = design["namespaces"]["global"]["modules"][top_module_name]
        connections = top_module["connections"]

        def clkwrk_idx_from_port(p):
            # Helper function to extract MU IO clkwrk index from port name
            m = re.search(r"_clkwrk_(\d+)_op_hcompute_mu_input_io_stencil", p)
            return int(m.group(1)) if m else None

        # Helper function to add a connection only if it doesn't already exist
        def add_conn_once(src, dst):
            pair = [src, dst]
            if pair not in connections:
                connections.append(pair)

        # Collect original MU->tree connections and ports
        for a, b in list(connections):
            is_mu_a = a.startswith("io16in_mu_")
            is_mu_b = b.startswith("io16in_mu_")
            is_tree_mem_a = "op_hcompute_tree_mem" in a and ".data" in a
            is_tree_mem_b = "op_hcompute_tree_mem" in b and ".data" in b
            is_tree_mu_a  = "op_hcompute_tree_mu"  in a and ".data" in a
            is_tree_mu_b  = "op_hcompute_tree_mu"  in b and ".data" in b

            if is_mu_a and (is_tree_mem_b or is_tree_mu_b):
                mu_port, tree_port = a, b
            elif is_mu_b and (is_tree_mem_a or is_tree_mu_a):
                mu_port, tree_port = b, a
            else:
                continue

            # Instantiate MEMs buffering activations with matching index to MU IOs
            mu_idx = clkwrk_idx_from_port(mu_port)
            assert mu_idx is not None, f"Cannot parse clkwrk index from MU port: {mu_port}"
            mem_name = f"mem_mu2tree_{mu_idx}"
            if mem_name not in instances:
                tpl = copy.deepcopy(self.mem_tpl)
                instances[mem_name] = tpl

            # Connect MU->MEM.in0
            add_conn_once(mu_port, f"{mem_name}.data_in_0")

            # Connect MEM.out0->tree_mem and MEM.out1->tree_mu
            if "op_hcompute_tree_mem" in tree_port:
                add_conn_once(f"{mem_name}.data_out_0", tree_port)
            else:
                assert "op_hcompute_tree_mu" in tree_port, f"Tree PE port {tree_port} is not a tree_mem or tree_mu port"
                add_conn_once(f"{mem_name}.data_out_1", tree_port)

        def is_direct_mu2tree(edge):
            # Helper function to check if an edge is a direct MU->tree edge in original connections
            x, y = edge
            mu_x = x.startswith("io16in_mu_")
            mu_y = y.startswith("io16in_mu_")
            tree_mem_x = "op_hcompute_tree_mem" in x and ".data" in x
            tree_mem_y = "op_hcompute_tree_mem" in y and ".data" in y
            tree_mu_x  = "op_hcompute_tree_mu"  in x and ".data" in x
            tree_mu_y  = "op_hcompute_tree_mu"  in y and ".data" in y
            return ((mu_x and (tree_mem_y or tree_mu_y)) or
                    (mu_y and (tree_mem_x or tree_mu_x)))

        # Remove original direct MU->tree edges
        connections[:] = [c for c in connections if not is_direct_mu2tree(c)]

        # Add shift FIFO for scale output data packing
        scale_output_shift_fifo_name = "scale_output_shift_fifo"
        if scale_output_shift_fifo_name not in instances:
            instances[scale_output_shift_fifo_name] = copy.deepcopy(self.shift_fifo_tpl)

        # Collect the pre-hacked single scale-output IO, then rename + clone to indexed names
        scale_output_IO = []
        existing_ios = [n for n in instances.keys() if "io16_hw_scale_output_stencil_op_hcompute_hw_scale_output_stencil_write_0" in n]
        assert len(existing_ios) == 1, "Expect one scale output IO of pre-hacked graph"
        old_io = existing_ios[0]
        old_port = old_io.replace("io16_", "")

        # Remove any old top-level type/connection using the legacy non-indexed port
        type_fields = top_module["type"][1]
        type_fields[:] = [f for f in type_fields if f[0] != old_port]
        connections[:] = [c for c in connections if c[0] != f"self.{old_port}" and c[1] != f"self.{old_port}"]
        connections[:] = [c for c in connections if c[0] != f"{old_io}.out" and c[1] != f"{old_io}.out"]
        connections[:] = [c for c in connections if old_io not in c[0] and old_io not in c[1]]

        # Build new indexed names
        new_io0 = "io16_hw_scale_output_stencil_clkwrk_0_op_hcompute_hw_scale_output_stencil_0_write_0"
        new_io1 = "io16_hw_scale_output_stencil_clkwrk_1_op_hcompute_hw_scale_output_stencil_1_write_0"

        # Rename existing to idx 0, then clone idx 1; insertion order yields [0, 1]
        inst_copy = copy.deepcopy(instances.pop(old_io))
        instances[new_io0] = inst_copy
        if new_io1 not in instances:
            instances[new_io1] = copy.deepcopy(inst_copy)

        # Add new top-level type fields and connect self port to each IO.out
        for io_name, idx in [(new_io0, 0), (new_io1, 1)]:
            port = io_name.replace("io16_", "")
            if all(field[0] != port for field in type_fields):
                type_fields.append([port, ["Array", 16, "Bit"]])
            add_conn_once(f"self.{port}", f"{io_name}.out")
            scale_output_IO.append(io_name)

        # Remove any old scale PE -> scale IO .in connections before rewiring
        scale_io_inputs = {f"{n}.in" for n in scale_output_IO}
        connections[:] = [c for c in connections if c[0] not in scale_io_inputs and c[1] not in scale_io_inputs]

        # Create scale data packing PE and connections
        # Find scale PE name
        scale_pe_name = None
        for instance_name, instance_conf in instances.items():
            if "get_shared_exp" in instance_name:
                scale_pe_name = instance_name
                break
        assert scale_pe_name is not None, "Cannot find scale PE"
        scale_data_packing_pe_name = "scale_output_data_packing_stencil$inner_compute$data_packing"
        scale_data_packing_const_name = "scale_output_data_packing_stencil$inner_compute$c0_dp"
        if scale_data_packing_pe_name not in instances:
            instances[scale_data_packing_pe_name] = {"modref": "global.PE"}
        if scale_data_packing_const_name not in instances:
            instances[scale_data_packing_const_name] = {
                "genref": "coreir.const",
                "genargs": {"width": ["Int", 84]},
                "modargs": {"value": [["BitVector", 84], self.DATA_PACKING_INSTR]},
            }
        add_conn_once(f"{scale_data_packing_pe_name}.inst", f"{scale_data_packing_const_name}.out")

        # SCALE_PE_SRC -> data_packing.in0
        add_conn_once(f"{scale_pe_name}.O0", f"{scale_data_packing_pe_name}.data0")

        # SCALE_PE_SRC -> shift FIFO -> data_packing.in1
        add_conn_once(f"{scale_pe_name}.O0", f"{scale_output_shift_fifo_name}.in")
        add_conn_once(f"{scale_output_shift_fifo_name}.out", f"{scale_data_packing_pe_name}.data1")

        # Insert a MEM tile to broadcast scale data_packing output to both IOs
        scale_output_broadcast_mem_name = "mem_scale_output_broadcast"
        if scale_output_broadcast_mem_name not in instances:
            instances[scale_output_broadcast_mem_name] = copy.deepcopy(self.mem_tpl)
        # Connect data_packing.O0 -> MEM.data_in_0
        add_conn_once(f"{scale_data_packing_pe_name}.O0", f"{scale_output_broadcast_mem_name}.data_in_0")

        # data_packing (via MEM).O0 -> additional FIFOs for index 0 IO
        # We need at least 5 additional FIFOs to balance the paths. We might remove this if we can improve the data rate.
        additional_fifo_names = []
        num_additional_fifos = 5  # Add 5 additional FIFOs

        # Create FIFO instance name
        for i in range(num_additional_fifos):
            fifo_name = f"scale_output_additional_fifo_{i}"
            additional_fifo_names.append(fifo_name)
            if fifo_name not in instances:
                instances[fifo_name] = copy.deepcopy(self.pipeline_fifo_tpl)

        # Connect MEM.data_out_0 to first additional FIFO
        if num_additional_fifos > 0:
            add_conn_once(f"{scale_output_broadcast_mem_name}.data_out_0", f"{additional_fifo_names[0]}.in")

            # Chain the additional FIFOs
            for i in range(1, num_additional_fifos):
                add_conn_once(f"{additional_fifo_names[i-1]}.out", f"{additional_fifo_names[i]}.in")

            # Connect last additional FIFO to index 0 IO
            index_0_io = scale_output_IO[0]  # First IO is index 0
            add_conn_once(f"{additional_fifo_names[-1]}.out", f"{index_0_io}.in")

            # Connect MEM.data_out_0 directly to index 1 IO (bypass additional FIFOs)
            index_1_io = scale_output_IO[1]  # Second IO is index 1
            add_conn_once(f"{scale_output_broadcast_mem_name}.data_out_0", f"{index_1_io}.in")
        else:
            # Fallback: connect directly to both IOs if no additional FIFOs
            for io_name in scale_output_IO:
                add_conn_once(f"{scale_output_broadcast_mem_name}.data_out_0", f"{io_name}.in")

        # Configure GLB store DMA to extract valid data of scale output, and consider data packing
        # Collect all scale output IOs instances
        scale_output_IO_idx = 0
        total_channels = int(self.halide_gen_args_dict["vec_width"])
        img_size = int(self.halide_gen_args_dict["vec_height"])
        mu_OC = int(self.halide_gen_args_dict["mu_i"])
        number_of_blocks = total_channels // (mu_OC * 2)
        for instance_name, instance_conf in instances.items():
            if "io16_hw_scale_output_stencil" in instance_name:
                # Two cases:
                # 1. vec_width is 64, meaning we only skip at the very beginning
                # 2. vec_width >=128 and vec_width % 64 == 0, meaning we have to skip pixels between loops
                assert (total_channels >= 64 and total_channels % 64 == 0), f"vec_width must be >= 64 and divisible by 64 as mx block size is 64"
                instance_conf["metadata"]["in2glb_0"]["cycle_starting_addr"][0] = scale_output_IO_idx
                instance_conf["metadata"]["in2glb_0"]["cycle_stride"] = [len(scale_output_IO)]
                instance_conf["metadata"]["in2glb_0"]["dimensionality"] = 1
                # Number of scales is divided by 2 because of data packing
                instance_conf["metadata"]["in2glb_0"]["extent"] = [img_size * number_of_blocks // 2 // len(scale_output_IO)]
                instance_conf["metadata"]["in2glb_0"]["write_data_starting_addr"] = [0]
                instance_conf["metadata"]["in2glb_0"]["write_data_stride"] = [1]

                scale_output_IO_idx += 1

        # Apply-scale pipeline + scale broadcast + output IOs

        # Output GLB IO template for quantized_output_stencil
        # Two cases:
        # 1. vec_width is 64, meaning we only skip at the very beginning
        # 2. vec_width >=128 and vec_width % 64 == 0, meaning we have to skip pixels between loops
        # if total_channels == 64:
        #     io_tpl = {
        #         "modref": "global.IO",
        #         "modargs": {"mode": ["String", "out"]},
        #         "metadata": {
        #             "in2glb_0": {
        #                 "cycle_starting_addr": [img_size * 2],
        #                 "cycle_stride": [1],
        #                 "dimensionality": 1,
        #                 "extent": [img_size * 2],
        #                 "write_data_starting_addr": [0],
        #                 "write_data_stride": [1],
        #             }
        #         },
        #     }
        # else:
        #     io_tpl = {
        #         "modref": "global.IO",
        #         "modargs": {"mode": ["String", "out"]},
        #         "metadata": {
        #             "in2glb_0": {
        #                 "cycle_starting_addr": [img_size * 2],
        #                 "cycle_stride": [1, img_size * 2 + 1],
        #                 "dimensionality": 2,
        #                 "extent": [
        #                     img_size * 2,
        #                     # Because block size is double the size of mu_OC
        #                     total_channels // mu_OC // 2
        #                 ],
        #                 "write_data_starting_addr": [0],
        #                 "write_data_stride": [1, 1],
        #             }
        #         },
        #     }

        # Output GLB IO template
        io_tpl = {
            "modref": "global.IO",
            "modargs": {"mode": ["String", "out"]},
            "metadata": {
                "in2glb_0": {
                    "cycle_starting_addr": [int(self.halide_gen_args_dict["vec_height"])],
                    "cycle_stride": [
                        1,
                        int(self.halide_gen_args_dict["vec_height"]) + 1,
                    ],
                    "dimensionality": 2,
                    "extent": [
                        int(self.halide_gen_args_dict["vec_height"]),
                        int(self.halide_gen_args_dict["vec_width"])
                        // int(self.halide_gen_args_dict["mu_i"])
                        // 2 # Because of data packing,
                    ],
                    "write_data_starting_addr": [0],
                    "write_data_stride": [1, 1],
                }
            },
        }

        # Collect MEM.out1 sources, emitting activations without delay
        mem_no_delay_srcs = []
        for iname in instances.keys():
            m = re.fullmatch(r"mem_mu2tree_(\d+)", iname)
            if m:
                mem_no_delay_srcs.append((int(m.group(1)), f"{iname}.data_out_1"))
        mem_no_delay_srcs.sort(key=lambda x: x[0])

        # Collect MEM.out0 sources, emitting activations with delay of image size
        mem_with_delay_srcs = []
        for iname in instances.keys():
            m = re.fullmatch(r"mem_mu2tree_(\d+)", iname)
            if m:
                mem_with_delay_srcs.append((int(m.group(1)), f"{iname}.data_out_0"))
        mem_with_delay_srcs.sort(key=lambda x: x[0])

        # Determine how many pipeline registers we need (max tree depth + 1)
        # NOTE: This is not used. We don't actually insert shift registers here as it makes the graph unroutable.
        n_regs = int(self.halide_gen_args_dict["tree_stages"]) + 1
        assert n_regs % 2 == 0, "Number of pipeline registers must be even"

        def build_apply_pipeline(group, idx, src, n_regs):
            """
            Build a pipeline with shift registers
            group: "mu" or "mem" for naming the pipeline
            idx: lane index [0..31]
            src: source for this lane (mem_mu2tree.data_out_0 or mem_mu2tree.data_out_1)
            """

            naming_base = f"apply_scale_{group}_{idx}_stencil"
            pe_name = f"{naming_base}$inner_compute$apply_scale"
            c0_name = f"{naming_base}$inner_compute$c0"

            # PE + const connection for apply scale instruction
            instances[pe_name] = copy.deepcopy(self.pe_tpl)
            instances[c0_name] = copy.deepcopy(self.apply_scale_const_tpl)
            add_conn_once(f"{pe_name}.inst", f"{c0_name}.out")

            # Shift-FIFO chain
            head = None
            prev = None
            if n_regs > 0:
                for k in range(n_regs):
                    rname = f"{naming_base}$d_reg__U0$reg{k}"
                    instances[rname] = copy.deepcopy(self.shift_fifo_tpl)
                    if k == 0:
                        # First shift FIFO
                        add_conn_once(src, f"{rname}.in")
                        head = rname
                    else:
                        add_conn_once(f"{prev}.out", f"{rname}.in")
                    prev = rname
                data0_src = f"{prev}.out"  # Tail shift FIFO feeds PE
            else:
                data0_src = src  # Bypass shift FIFOs and feed src directly

            # Tail -> PE.data0, and scale broadcast to data1 port
            add_conn_once(data0_src, f"{pe_name}.data0")
            add_conn_once(f"{scale_pe_name}.O0", f"{pe_name}.data1")

            return f"{pe_name}.O0"

        # Build 32 MU pipelines and 32 MEM pipelines as we have 32 MU IOs in Zircon
        # Also create apply_outputs dict for sorting and data packing
        apply_outputs = {"mem_no_delay": {}, "mem_with_delay": {}}
        for idx, pin in mem_no_delay_srcs:
            apply_outputs["mem_no_delay"][idx] = build_apply_pipeline("mem_no_delay", idx, pin, n_regs=0)

        for idx, pin in mem_with_delay_srcs:
            apply_outputs["mem_with_delay"][idx] = build_apply_pipeline("mem_with_delay", idx, pin, n_regs=0)

        # Data_packing stage: pair adjacent apply_scale PEs
        # Create pending lists for sorting
        pending_ios_mem_no_delay = []
        pending_ios_mem_with_delay = []

        # Pack adjacent apply_scale outputs for mem_no_delay, which corresponds to second 32 channels
        mem_no_delay_idxs = sorted(apply_outputs["mem_no_delay"].keys())
        for i in range(0, len(mem_no_delay_idxs) - 1, 2):
            idx0, idx1 = mem_no_delay_idxs[i], mem_no_delay_idxs[i + 1]

            naming_base = f"data_packing_pair_mem_no_delay_{idx0}_{idx1}_stencil"
            data_packing_pe = f"{naming_base}$inner_compute$data_packing"
            data_packing_const = f"{naming_base}$inner_compute$c0_dp"

            instances[data_packing_pe] = copy.deepcopy(self.pe_tpl)
            instances[data_packing_const] = copy.deepcopy(self.data_packing_const_tpl)
            add_conn_once(f"{data_packing_pe}.inst", f"{data_packing_const}.out")

            add_conn_once(apply_outputs["mem_no_delay"][idx0], f"{data_packing_pe}.data1") #
            add_conn_once(apply_outputs["mem_no_delay"][idx1], f"{data_packing_pe}.data0")

            orig_idx = idx0
            pending_ios_mem_no_delay.append((orig_idx, data_packing_pe))

        # Pack adjacent apply_scale outputs for mem_with_delay, which corresponds to first 32 channels
        mem_idxs = sorted(apply_outputs["mem_with_delay"].keys())
        for i in range(0, len(mem_idxs) - 1, 2):
            idx0, idx1 = mem_idxs[i], mem_idxs[i + 1]

            naming_base = f"data_packing_pair_mem_with_delay_{idx0}_{idx1}_stencil"
            data_packing_pe = f"{naming_base}$inner_compute$data_packing"
            data_packing_const = f"{naming_base}$inner_compute$c0_dp"

            instances[data_packing_pe] = copy.deepcopy(self.pe_tpl)
            instances[data_packing_const] = copy.deepcopy(self.data_packing_const_tpl)
            add_conn_once(f"{data_packing_pe}.inst", f"{data_packing_const}.out")

            add_conn_once(apply_outputs["mem_with_delay"][idx0], f"{data_packing_pe}.data1")
            add_conn_once(apply_outputs["mem_with_delay"][idx1], f"{data_packing_pe}.data0")

            orig_idx = idx0
            pending_ios_mem_with_delay.append((orig_idx, data_packing_pe))

        # Instantiate output IOs, MEM with delay first, then MEM no delay
        pending_ios_mem_with_delay = sorted(pending_ios_mem_with_delay, key=lambda x: x[0])
        pending_ios_mem_no_delay  = sorted(pending_ios_mem_no_delay,  key=lambda x: x[0])
        quantized_output_ios = []

        # # Insert MEM tiles between data packing PEs and quantized output IOs
        # # Pair every two data packing PEs and connect them to MEM tiles
        # all_pending_ios = []

        # # Add MEM with delay data packing PEs
        # for ascending_idx, (_, data_packing_pe) in enumerate(pending_ios_mem_with_delay):
        #     all_pending_ios.append((ascending_idx, data_packing_pe, "mem_with_delay"))

        # # Add MEM no delay data packing PEs
        # idx_offset = len(pending_ios_mem_with_delay)
        # for ascending_idx, (_, data_packing_pe) in enumerate(pending_ios_mem_no_delay):
        #     all_pending_ios.append((idx_offset + ascending_idx, data_packing_pe, "mem_no_delay"))

        # # Sort all pending IOs by their ascending index
        # all_pending_ios.sort(key=lambda x: x[0])

        # # Assert that we have an even number of data packing PEs for pairing
        # assert len(all_pending_ios) % 2 == 0, f"Expected even number of data packing PEs for pairing, got {len(all_pending_ios)}"

        # # Create MEM tiles for pairing data packing PEs to interleave data streams
        # mem_tile_outputs = []
        # pair_offset = len(all_pending_ios) // 2
        # num_pipeline_fifos = 7  # Number of pipeline FIFOs to add between data_packing_pe1 and MEM tile
        # for pair_idx in range(pair_offset):
        #     idx0, data_packing_pe0, group0 = all_pending_ios[pair_idx]
        #     idx1, data_packing_pe1, group1 = all_pending_ios[pair_idx + pair_offset]

        #     # Create MEM tile for this pair to interleave data streams
        #     mem_tile_name = f"mem_quantized_output_pair_{idx0}_{idx1}"
        #     mem_tile_inst = copy.deepcopy(mem_tpl)
        #     instances[mem_tile_name] = mem_tile_inst

        #     # Connect data packing PEs to MEM tile inputs
        #     add_conn_once(f"{data_packing_pe0}.O0", f"{mem_tile_name}.data_in_0")

        #     # Create pipeline FIFOs for data_packing_pe1.O0 -> mem_tile_name.data_in_1 path
        #     # This is a bit tricky. We want to ensure the data going into both input ports of MEM tile won't arrive at the same time.
        #     # We need write-after-write constraint, but looks like it's not implemented.
        #     # Adding pipeline FIFOs at pre-PnR stage won't actually gurantee the data won't arrive at the same time. Changing placement may cause a bug.
        #     fifo_names = []
        #     for fifo_idx in range(num_pipeline_fifos):
        #         fifo_name = f"pipeline_fifo_{mem_tile_name}_data_in_1_{fifo_idx}"
        #         fifo_names.append(fifo_name)
        #         instances[fifo_name] = copy.deepcopy(pipeline_fifo_tpl)

        #     # Chain the FIFOs together
        #     add_conn_once(f"{data_packing_pe1}.O0", f"{fifo_names[0]}.in")
        #     for i in range(1, num_pipeline_fifos):
        #         add_conn_once(f"{fifo_names[i-1]}.out", f"{fifo_names[i]}.in")
        #     add_conn_once(f"{fifo_names[num_pipeline_fifos-1]}.out", f"{mem_tile_name}.data_in_1")

        #     # Store MEM tile outputs for later connection to IOs
        #     mem_tile_outputs.append((pair_idx, f"{mem_tile_name}.data_out_0", f"{mem_tile_name}.data_out_1"))

        # mem_with_delay group
        for ascending_idx, (_orig_idx, data_packing_pe) in enumerate(pending_ios_mem_with_delay):
            io_idx = ascending_idx  # 0..15
            io_name = f"io16_quantized_output_stencil_clkwrk_{io_idx}_op_hcompute_quantized_output_stencil_{io_idx}_write_0"
            if io_name not in instances:
                instances[io_name] = copy.deepcopy(io_tpl)
            add_conn_once(f"{data_packing_pe}.O0", f"{io_name}.in")
            quantized_output_ios.append(io_name)

        # mem_no_delay group
        base = len(pending_ios_mem_with_delay)  # 16
        for ascending_idx, (_orig_idx, data_packing_pe) in enumerate(pending_ios_mem_no_delay):
            io_idx = base + ascending_idx  # 16..31
            io_name = f"io16_quantized_output_stencil_clkwrk_{io_idx}_op_hcompute_quantized_output_stencil_{io_idx}_write_0"
            if io_name not in instances:
                instances[io_name] = copy.deepcopy(io_tpl)
            add_conn_once(f"{data_packing_pe}.O0", f"{io_name}.in")
            quantized_output_ios.append(io_name)

        # # Create quantized output IOs connected to MEM tile outputs to interleave data streams
        # for mem_tile_idx, output0, output1 in mem_tile_outputs:
        #     # Create one IO connected to MEM tile output 0 (renamed with halved index)
        #     io_name = f"io16_quantized_output_stencil_clkwrk_{mem_tile_idx}_op_hcompute_quantized_output_stencil_{mem_tile_idx}_write_0"
        #     instances[io_name] = copy.deepcopy(io_tpl)
        #     add_conn_once(output0, f"{io_name}.in")
        #     quantized_output_ios.append(io_name)

        # Type instances of self.<port> to corresponding IO.out
        type_fields = top_module["type"][1]
        for io_inst in quantized_output_ios:
            port_name = io_inst.replace("io16_", "")
            type_fields.append([port_name, ["Array", 16, "Bit"]])
            add_conn_once(f"self.{port_name}", f"{io_inst}.out")

        # Overwrite the JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

        # Update design_meta_halide.json to update mu_inputs shape and add quantized_output_stencil
        design_meta_path = os.path.join(bin_path, "design_meta_halide.json")
        with open(design_meta_path, "r") as f:
            design_meta = json.load(f)
        assert len(design_meta["IOs"]["inputs"]) == 1, "Expected only one input which is mu input"
        # Real input shape should match vec_width instead of vec_width_fake
        design_meta["IOs"]["inputs"][0]["shape"][0] = int(self.halide_gen_args_dict["vec_width"])
        # Add quantized_output_stencil
        design_meta["IOs"]["outputs"].append(
            {
                "bitwidth": 16,
                "datafile": "quantized_output_stencil.raw",
                "name": "quantized_output_stencil",
                "shape": [mu_OC, img_size * total_channels // mu_OC // 2],
            }
        )
        # Fix the name and shape of hw_scale_output_stencil
        for output in design_meta["IOs"]["outputs"]:
            if output["name"] == "hw_scale_output_stencil":
                output["datafile"] = "hw_scale_output.raw"
                # img_size * num_blocks // 2 because of data packing
                output["shape"] = [img_size * total_channels // mu_OC // 2 // 2]
                break

        with open(design_meta_path, "w") as f:
            json.dump(design_meta, f, indent=2)

    def hack_for_relu_layer_multiout_fp_rv(self, json_path, bin_path):
        """
        We hardcoded the unroll to 4 as this is only a unit test for multi-output CGRA call.
        """
        ADD_3_INSTRUCTION = "84'h000008080800002480082"
        with open(json_path, "r") as f:
            design = json.load(f)

        top_module_name = "relu_layer_multiout_fp"
        ns = design["namespaces"]["global"]["modules"]
        top_module = ns[top_module_name]
        instances = top_module["instances"]
        connections = top_module["connections"]

        # Find a template of output IO for copying
        out_meta_template = None
        for inst_name, inst in instances.items():
            if inst.get("modref") == "global.IO" and inst.get("modargs", {}).get("mode", [""])[-1] == "out":
                meta = inst.get("metadata", {}).get("in2glb_0")
                if meta:
                    out_meta_template = copy.deepcopy(meta)
                    break
        if out_meta_template is None:
            # Otherwise use common pattern
            out_meta_template = {
                "cycle_starting_addr": [0],
                "cycle_stride": [1],
                "dimensionality": 1,
                "extent": [1568],
                "write_data_starting_addr": [0],
                "write_data_stride": [1],
            }

        # Hardcoded the unroll to 4
        base_out_clkwrk = 12
        lane_suffix = ["", "_1", "_2", "_3"]

        # Top-level port list
        record_fields = top_module["type"][1]

        # Helper to add a top-level port pair (data, valid)
        def add_top_ports(port_base_name):
            record_fields.append([port_base_name, ["Array", 16, "Bit"]])
            valid_name = port_base_name.rsplit("_write_0", 1)[0] + "_write_valid"
            record_fields.append([valid_name, "Bit"])

        # Create per-lane instances and connections
        for i in range(4):
            clkwrk_id = base_out_clkwrk + i
            suf = lane_suffix[i]

            # New adder PEs
            adder_inst = (
                "op_hcompute_add_3_stencil"
                f"{('_' + str(i)) if i > 0 else ''}$inner_compute$float_DW_fp_add_add3_{i}"
            )
            const_inst = (
                "op_hcompute_add_3_stencil"
                f"{('_' + str(i)) if i > 0 else ''}$inner_compute$c_add3_{i}"
            )
            in_inst = f"io16in_hw_input_stencil_clkwrk_{4+i}_op_hcompute_hw_input_global_wrapper_stencil{suf}_read_0"
            out_io_inst = f"io16_add_3_output_stencil_clkwrk_{clkwrk_id}_op_hcompute_add_3_stencil{suf}_write_0"
            # New top-level port names (data + valid)
            tl_data_port = f"add_3_output_stencil_clkwrk_{clkwrk_id}_op_hcompute_add_3_stencil{suf}_write_0"

            if const_inst not in instances:
                instances[const_inst] = {
                    "genref": "coreir.const",
                    "genargs": {"width": ["Int", 84]},
                    "modargs": {"value": [["BitVector", 84], ADD_3_INSTRUCTION]},
                }

            if adder_inst not in instances:
                instances[adder_inst] = {"modref": "global.PE"}

            if out_io_inst not in instances:
                instances[out_io_inst] = {
                    "modref": "global.IO",
                    "modargs": {"mode": ["String", "out"]},
                    "metadata": {"in2glb_0": copy.deepcopy(out_meta_template)},
                }

            existing_port_names = {p[0] for p in record_fields}
            if tl_data_port not in existing_port_names:
                add_top_ports(tl_data_port)

            def add_conn_once(src, dst):
                pair = [src, dst]
                if pair not in connections:
                    connections.append(pair)

            add_conn_once(f"{adder_inst}.data0", f"{in_inst}.out")
            add_conn_once(f"{adder_inst}.inst", f"{const_inst}.out")
            add_conn_once(f"{adder_inst}.O0", f"{out_io_inst}.in")
            add_conn_once(f"self.{tl_data_port}", f"{out_io_inst}.out")

        inst_items = list(instances.items())
        io_out_group = []
        others = []
        for idx, (iname, inst) in enumerate(inst_items):
            if inst.get("modref") == "global.IO" and inst.get("modargs", {}).get("mode", [""])[-1] == "out":
                m = re.search(r"io16.*output.*clkwrk_(\d+)_", iname, flags=re.IGNORECASE)
                if m:
                    io_out_group.append((int(m.group(1)), idx, iname, inst))
                    continue
            others.append((idx, iname, inst))

        if io_out_group:
            io_out_group.sort(key=lambda t: (t[0], t[1]))
            new_instances = OrderedDict()
            for _, name, inst in others:
                new_instances[name] = inst
            for _, _, name, inst in io_out_group:
                new_instances[name] = inst
            top_module["instances"] = new_instances

        # Write back json
        with open(json_path, "w") as f:
            json.dump(design, f, indent=2)

        # Also update design_meta_halide.json to update mu_inputs shape
        # TODO: This applies hack to design_meta as well. Do we want to create a new class?
        design_meta_path = os.path.join(bin_path, "design_meta_halide.json")
        with open(design_meta_path, "r") as f:
            design_meta = json.load(f)
        if len(design_meta["IOs"]["outputs"]) == 1:
            # copy this output but rename with new name
            add_3_output = copy.deepcopy(design_meta["IOs"]["outputs"][0])
            add_3_output["name"] = "add_3_output_stencil"
            add_3_output["datafile"] = "hw_add_3.raw"
            design_meta["IOs"]["outputs"].append(add_3_output)
        with open(design_meta_path, "w") as f:
            json.dump(design_meta, f, indent=2)

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
            pes_balanced = 0

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

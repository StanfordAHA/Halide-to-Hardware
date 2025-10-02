#!/usr/bin/env python3

import os, re, argparse, json, copy
from pretty_format_json import pretty_format_json

APPS_NEEDING_HACKS = [
    "scalar_reduction",
    "scalar_reduction_fp",
    "scalar_max_fp",
    "stable_softmax_pass2_fp",
    "stable_softmax_pass3_fp",
    "scalar_avg_fp",
    "layer_norm_pass2_fp",
    "layer_norm_pass3_fp",
    "gelu_pass2_fp",
    "silu_pass2_fp",
    "swiglu_pass2_fp",
    "vector_reduction_fp",
    "mem_transpose_test",
    "mem_slice_test",
    "mem_filter_test",
    "avgpool_layer_fp",
    "mat_vec_mul_fp",
    "get_e8m0_scale_test_fp",
    "get_apply_e8m0_scale_fp",
    "relu_layer_multiout_fp",
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

    def hack_for_stable_softmax_pass2_fp_static(self, json_path, bin_path):

        # TODO: Hardcode input pipelining regs for now
        INPUT_PIPELINING_REGS = 3

        with open(json_path, "r") as f:
            design = json.load(f)

        top_module = "stable_softmax_pass2_fp"
        tree_out_pond = "tree_3_stencil$ub_tree_3_stencil_BANK_0_garnet"
        tree_out_pond_clk = "tree_3_stencil$ub_tree_3_stencil_BANK_0_clk_en_const"

        # Const PE instance to remove
        pe_to_remove = "op_hcompute_output_cgra_stencil$inner_compute$const_"

        # Locate "stable_softmax_pass2_fp" module
        global_modules = design["namespaces"]["global"]["modules"]
        if top_module not in global_modules:
            print(f"WARNING: Module '{top_module}' not found in design. No hack applied.")
            return
        stable_softmax_pass2_fp = global_modules[top_module]

        # Remove the tile instance and its clk_en_const
        instance_dict = stable_softmax_pass2_fp.get("instances", {})
        if tree_out_pond in instance_dict:
            del instance_dict[tree_out_pond]
        if tree_out_pond_clk in instance_dict:
            del instance_dict[tree_out_pond_clk]

        # Remove references to the tile from connections,
        #    capturing upstream => old_input, downstream => old_output
        old_input = None
        old_output = None
        new_connections = []

        connection_list = stable_softmax_pass2_fp.get("connections", [])
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
        stable_softmax_pass2_fp["connections"] = final_connections

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
            # Set reduction tree delay
            reduction_tree_delay = 1  # It's one because we have fp.exp
            config["in2regfile_0"]["cycle_starting_addr"] = [
                input_pipelining_regs + reduction_tree_delay
            ]

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

        # Update stencil MEM config to be the same as output IO tile
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

        # Remove replicated IOs to stream vec max and only keep one for broadcasting to subtraction PEs
        max_io_prefix = "io16in_vec_max_host_stencil_clkwrk_"
        all_inst = list(instance_dict.keys())
        vecmax_insts = [inst for inst in all_inst if inst.startswith(max_io_prefix)]
        if len(vecmax_insts) > 1:
            # Sort them and keep the first
            vecmax_insts.sort()
            keep_inst = vecmax_insts[0]
            remove_insts = vecmax_insts[1:]

            # Remove from instance_dict
            for r in remove_insts:
                del instance_dict[r]

            # The module "type" is stable_softmax_pass2_fp["type"] = ["Record",[...]]
            type_list = stable_softmax_pass2_fp["type"][1]

            # Convert instance name -> the corresponding type prefix
            # e.g. "io16in_vec_max_host_stencil_clkwrk_8_op_hcompute_vec_max_glb_stencil_2_read_0"
            # -> "vec_max_host_stencil_clkwrk_8_op_hcompute_vec_max_glb_stencil_2_read_0"
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
                if "vec_max_host_stencil_clkwrk_" not in field_name:
                    # Not a vecmax field => keep
                    new_type_list.append(field_pair)
                    continue
                # It's a vecmax field => keep only if it belongs to keep_prefix
                # (i.e. starts with keep_prefix)
                if field_name.startswith(keep_prefix):
                    new_type_list.append(field_pair)
                # else skip
            stable_softmax_pass2_fp["type"][1] = new_type_list

            # Fix up connections: if referencing removed inst or fields, unify to keep inst
            new_conn = []
            for left, right in stable_softmax_pass2_fp["connections"]:
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

            stable_softmax_pass2_fp["connections"] = new_conn

        # Deduplicate final connections: remove exact duplicates
        final_dedup = []
        seen = set()
        for c in stable_softmax_pass2_fp["connections"]:
            # Represent connection as tuple (left, right)
            t = tuple(c)
            if t not in seen:
                seen.add(t)
                final_dedup.append(c)

        stable_softmax_pass2_fp["connections"] = final_dedup

        # Configure the IO to read the same addr of GLB repeatedly
        # TODO: this should be replaced by reading from MEM tile instead for power efficiency
        for inst_name, inst_config in instance_dict.items():
            if "io16in_vec_max" in inst_name and inst_config["modref"] == "global.IO":
                inst_config["metadata"]["glb2out_0"]["read_data_stride"] = [0]

        # Overwrite the JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

    def hack_for_stable_softmax_pass3_fp_static(self, json_path, bin_path):

        with open(json_path, "r") as f:
            design = json.load(f)

        top_module = "stable_softmax_pass3_fp"

        # Locate "stable_softmax_pass2_fp" module
        global_modules = design["namespaces"]["global"]["modules"]
        if top_module not in global_modules:
            print(f"WARNING: Module '{top_module}' not found in design. No hack applied.")
            return
        stable_softmax_pass3_fp = global_modules[top_module]
        instance_dict = stable_softmax_pass3_fp.get("instances", {})

        # Remove replicated IOs to stream pass2_sum and only keep one for broadcasting to divison PEs
        max_io_prefix = "io16in_pass2_sum_host_stencil_"
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

            # The module "type" is stable_softmax_pass3_fp["type"] = ["Record",[...]]
            type_list = stable_softmax_pass3_fp["type"][1]

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
                if "io16in_pass2_sum_host_stencil_" not in field_name:
                    # Not a vecmax field => keep
                    new_type_list.append(field_pair)
                    continue
                # It's a vecmax field => keep only if it belongs to keep_prefix
                # (i.e. starts with keep_prefix)
                if field_name.startswith(keep_prefix):
                    new_type_list.append(field_pair)
                # else skip
            stable_softmax_pass3_fp["type"][1] = new_type_list

            # Fix up connections: if referencing removed inst or fields, unify to keep inst
            new_conn = []
            for left, right in stable_softmax_pass3_fp["connections"]:
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

            stable_softmax_pass3_fp["connections"] = new_conn

        # Deduplicate final connections: remove exact duplicates
        final_dedup = []
        seen = set()
        for c in stable_softmax_pass3_fp["connections"]:
            # Represent connection as tuple (left, right)
            t = tuple(c)
            if t not in seen:
                seen.add(t)
                final_dedup.append(c)

        stable_softmax_pass3_fp["connections"] = final_dedup

        # Configure the IO to read the same addr of GLB repeatedly
        # TODO: this should be replaced by reading from MEM tile instead for power efficiency
        for inst_name, inst_config in instance_dict.items():
            if (
                "io16in_pass2_sum_host_stencil" in inst_name
                and inst_config["modref"] == "global.IO"
            ):
                inst_config["metadata"]["glb2out_0"]["read_data_stride"] = [0]

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

    def hack_for_layer_norm_pass2_fp_static(self, json_path, bin_path):

        # TODO: Hardcode input pipelining regs for now
        INPUT_PIPELINING_REGS = 3

        with open(json_path, "r") as f:
            design = json.load(f)

        top_module = "layer_norm_pass2_fp"
        tree_out_pond = "tree_3_stencil$ub_tree_3_stencil_BANK_0_garnet"
        tree_out_pond_clk = "tree_3_stencil$ub_tree_3_stencil_BANK_0_clk_en_const"

        # Const PE instance to remove
        pe_to_remove = "op_hcompute_output_cgra_stencil$inner_compute$const_"

        # Locate "layer_norm_pass2_fp" module
        global_modules = design["namespaces"]["global"]["modules"]
        if top_module not in global_modules:
            print(f"WARNING: Module '{top_module}' not found in design. No hack applied.")
            return
        layer_norm_pass2_fp = global_modules[top_module]

        # Remove the tile instance and its clk_en_const
        instance_dict = layer_norm_pass2_fp.get("instances", {})
        if tree_out_pond in instance_dict:
            del instance_dict[tree_out_pond]
        if tree_out_pond_clk in instance_dict:
            del instance_dict[tree_out_pond_clk]

        # Remove references to the tile from connections,
        #    capturing upstream => old_input, downstream => old_output
        old_input = None
        old_output = None
        new_connections = []

        connection_list = layer_norm_pass2_fp.get("connections", [])
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
        layer_norm_pass2_fp["connections"] = final_connections

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
            # Set reduction tree delay
            reduction_tree_delay = 0
            config["in2regfile_0"]["cycle_starting_addr"] = [
                input_pipelining_regs + reduction_tree_delay
            ]

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
        ) + 2  # Add 2 for the sqrt (log + exp)
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

        # Update stencil MEM config to be the same as output IO tile
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

        # Remove replicated IOs to stream vec max and only keep one for broadcasting to subtraction PEs
        max_io_prefix = "io16in_vec_avg_host_stencil_clkwrk_"
        all_inst = list(instance_dict.keys())
        vecmax_insts = [inst for inst in all_inst if inst.startswith(max_io_prefix)]
        if len(vecmax_insts) > 1:
            # Sort them and keep the first
            vecmax_insts.sort()
            keep_inst = vecmax_insts[0]
            remove_insts = vecmax_insts[1:]

            # Remove from instance_dict
            for r in remove_insts:
                del instance_dict[r]

            # The module "type" is layer_norm_pass2_fp["type"] = ["Record",[...]]
            type_list = layer_norm_pass2_fp["type"][1]

            # Convert instance name -> the corresponding type prefix
            # e.g. "io16in_vec_avg_host_stencil_clkwrk_8_op_hcompute_vec_avg_glb_stencil_2_read_0"
            # -> "vec_avg_host_stencil_clkwrk_8_op_hcompute_vec_avg_glb_stencil_2_read_0"
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
                if "vec_avg_host_stencil_clkwrk_" not in field_name:
                    # Not a vecmax field => keep
                    new_type_list.append(field_pair)
                    continue
                # It's a vecmax field => keep only if it belongs to keep_prefix
                # (i.e. starts with keep_prefix)
                if field_name.startswith(keep_prefix):
                    new_type_list.append(field_pair)
                # else skip
            layer_norm_pass2_fp["type"][1] = new_type_list

            # Fix up connections: if referencing removed inst or fields, unify to keep inst
            new_conn = []
            for left, right in layer_norm_pass2_fp["connections"]:
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

            layer_norm_pass2_fp["connections"] = new_conn

        # Deduplicate final connections: remove exact duplicates
        final_dedup = []
        seen = set()
        for c in layer_norm_pass2_fp["connections"]:
            # Represent connection as tuple (left, right)
            t = tuple(c)
            if t not in seen:
                seen.add(t)
                final_dedup.append(c)

        layer_norm_pass2_fp["connections"] = final_dedup

        # Configure the IO to read the same addr of GLB repeatedly
        # TODO: this should be replaced by reading from MEM tile instead for power efficiency
        for inst_name, inst_config in instance_dict.items():
            if "io16in_vec_avg" in inst_name and inst_config["modref"] == "global.IO":
                inst_config["metadata"]["glb2out_0"]["read_data_stride"] = [0]

        # Overwrite the JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

    def hack_for_layer_norm_pass3_fp_static(self, json_path, bin_path):

        with open(json_path, "r") as f:
            design = json.load(f)

        top_module = "layer_norm_pass3_fp"

        # Locate "stable_softmax_pass2_fp" module
        global_modules = design["namespaces"]["global"]["modules"]
        if top_module not in global_modules:
            print(f"WARNING: Module '{top_module}' not found in design. No hack applied.")
            return
        layer_norm_pass3_fp = global_modules[top_module]
        instance_dict = layer_norm_pass3_fp.get("instances", {})

        # Remove replicated IOs to stream pass1_avg and only keep one for broadcasting to divison PEs
        def remove_duplicate_ios(io_prefix, instance_dict, layer_norm_pass3_fp):
            # Find all instances that start with the given prefix
            all_inst = list(instance_dict.keys())
            io_insts = [inst for inst in all_inst if inst.startswith(io_prefix)]

            if len(io_insts) > 1:
                # Sort instances and select the first one to keep
                io_insts.sort()
                keep_inst = io_insts[0]
                remove_insts = io_insts[1:]

                # Remove extra instances from instance_dict
                for r in remove_insts:
                    del instance_dict[r]

                # Update the type record: layer_norm_pass3_fp["type"] = ["Record", [ ... ]]
                type_list = layer_norm_pass3_fp["type"][1]

                # Convert instance name to field prefix by stripping the "io16in_" part
                def inst_to_field_prefix(name):
                    if name.startswith("io16in_"):
                        return name[len("io16in_") :]
                    return name

                keep_prefix = inst_to_field_prefix(keep_inst)
                remove_prefixes = [inst_to_field_prefix(r) for r in remove_insts]

                new_type_list = []
                for field_pair in type_list:
                    field_name, field_type = field_pair
                    # If the field does not belong to the current IO prefix, keep it
                    if io_prefix not in field_name:
                        new_type_list.append(field_pair)
                    # If it belongs to the IO prefix, keep it only if it matches the kept instance
                    elif field_name.startswith(keep_prefix):
                        new_type_list.append(field_pair)
                layer_norm_pass3_fp["type"][1] = new_type_list

                # Update connections so that references to removed instances or fields are unified to the kept ones
                new_conn = []
                for left, right in layer_norm_pass3_fp["connections"]:
                    new_left, new_right = left, right
                    # Replace any removed instance names with the kept instance name
                    for rinst in remove_insts:
                        if rinst in new_left:
                            new_left = new_left.replace(rinst, keep_inst)
                        if rinst in new_right:
                            new_right = new_right.replace(rinst, keep_inst)
                    # Replace any removed field prefixes with the kept prefix
                    for rpref in remove_prefixes:
                        if rpref in new_left:
                            new_left = new_left.replace(rpref, keep_prefix)
                        if rpref in new_right:
                            new_right = new_right.replace(rpref, keep_prefix)
                    new_conn.append([new_left, new_right])
                layer_norm_pass3_fp["connections"] = new_conn

        remove_duplicate_ios("io16in_pass1_avg_host_stencil_", instance_dict, layer_norm_pass3_fp)
        remove_duplicate_ios("io16in_pass2_std_host_stencil_", instance_dict, layer_norm_pass3_fp)

        # Deduplicate final connections: remove exact duplicates
        final_dedup = []
        seen = set()
        for c in layer_norm_pass3_fp["connections"]:
            # Represent connection as tuple (left, right)
            t = tuple(c)
            if t not in seen:
                seen.add(t)
                final_dedup.append(c)

        layer_norm_pass3_fp["connections"] = final_dedup

        # Configure the IO to read the same addr of GLB repeatedly
        # TODO: this should be replaced by reading from MEM tile instead for power efficiency
        for inst_name, inst_config in instance_dict.items():
            if (
                "io16in_pass1_avg_host_stencil" in inst_name
                or "io16in_pass2_std_host_stencil" in inst_name
            ) and inst_config["modref"] == "global.IO":
                inst_config["metadata"]["glb2out_0"]["read_data_stride"] = [0]

        # Overwrite the JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

    def hack_for_gelu_pass2_fp_static(self, json_path, bin_path):

        with open(json_path, "r") as f:
            design = json.load(f)

        top_module = "gelu_pass2_fp"

        # Locate "stable_softmax_pass2_fp" module
        global_modules = design["namespaces"]["global"]["modules"]
        if top_module not in global_modules:
            print(f"WARNING: Module '{top_module}' not found in design. No hack applied.")
            return
        gelu_pass2_fp = global_modules[top_module]
        instance_dict = gelu_pass2_fp.get("instances", {})

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

            # The module "type" is gelu_pass2_fp["type"] = ["Record",[...]]
            type_list = gelu_pass2_fp["type"][1]

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
            gelu_pass2_fp["type"][1] = new_type_list

            # Fix up connections: if referencing removed inst or fields, unify to keep inst
            new_conn = []
            for left, right in gelu_pass2_fp["connections"]:
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

            gelu_pass2_fp["connections"] = new_conn

        # Deduplicate final connections: remove exact duplicates
        final_dedup = []
        seen = set()
        for c in gelu_pass2_fp["connections"]:
            # Represent connection as tuple (left, right)
            t = tuple(c)
            if t not in seen:
                seen.add(t)
                final_dedup.append(c)

        gelu_pass2_fp["connections"] = final_dedup

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

    def hack_for_vector_reduction_fp_rv(self, json_path, bin_path):

        with open(json_path, "r") as f:
            design = json.load(f)

        top_module = "vector_reduction_fp"
        tree_out_pond = "tree_3_stencil$ub_tree_3_stencil_BANK_0_garnet"
        tree_out_pond_clk = "tree_3_stencil$ub_tree_3_stencil_BANK_0_clk_en_const"

        # Const PE instance to remove
        pe_to_remove = "op_hcompute_output_cgra_stencil$inner_compute$const_"

        # Locate "vector_reduction_fp" module
        global_modules = design["namespaces"]["global"]["modules"]
        if top_module not in global_modules:
            print(f"WARNING: Module '{top_module}' not found in design. No hack applied.")
            return
        vector_reduction_fp = global_modules[top_module]

        # Remove the tile instance and its clk_en_const
        instance_dict = vector_reduction_fp.get("instances", {})
        if tree_out_pond in instance_dict:
            del instance_dict[tree_out_pond]
        if tree_out_pond_clk in instance_dict:
            del instance_dict[tree_out_pond_clk]

        # Remove references to the tile from connections,
        #    capturing upstream => old_input, downstream => old_output
        old_input = None
        old_output = None
        new_connections = []

        connection_list = vector_reduction_fp.get("connections", [])
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
        vector_reduction_fp["connections"] = final_connections

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

        # Update output IO tile extent to match the number of output pixels
        # Also have to update cycle_start_addr and stride, as it means handshake rather than cycles in dense rv scenario
        io_out_name = "io16_hw_output_stencil_op_hcompute_hw_output_stencil_write_0"
        instance_dict[io_out_name]["metadata"]["in2glb_0"]["extent"] = [
            int(self.halide_gen_args_dict["vec_height"])
        ]
        # We want to accept every valid handshake starting from the first one
        instance_dict[io_out_name]["metadata"]["in2glb_0"]["cycle_starting_addr"] = [0]
        instance_dict[io_out_name]["metadata"]["in2glb_0"]["cycle_stride"] = [1]

        # Overwrite the JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

    def hack_for_mat_vec_mul_fp_rv(self, json_path, bin_path):
        # Load the design JSON
        with open(json_path, "r") as f:
            design = json.load(f)

        top_module = "mat_vec_mul_fp"
        tree_out_pond_pattern = r"^tree_\d+_stencil\$ub_tree_\d+_stencil_BANK_\d+_garnet$"
        tree_out_pond_clk_pattern = r"^tree_\d+_stencil\$ub_tree_\d+_stencil_BANK_\d+_clk_en_const$"
        pe_to_remove = "op_hcompute_output_cgra_stencil$inner_compute$const_"

        # Locate the mat_vec_mul_fp module
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
        old_input = old_output = None
        new_connections = []
        for left, right in connection_list:
            match_left = re.match(tree_out_pond_pattern, left.split(".")[0]) or re.match(
                tree_out_pond_clk_pattern, left.split(".")[0]
            )
            match_right = re.match(tree_out_pond_pattern, right.split(".")[0]) or re.match(
                tree_out_pond_clk_pattern, right.split(".")[0]
            )
            if not (match_left or match_right):
                new_connections.append([left, right])
                continue
            if match_left:
                if ".data_in" in left:
                    old_input = right
                if ".data_out" in left:
                    old_output = right
            if match_right:
                if ".data_in" in right:
                    old_input = left
                if ".data_out" in right:
                    old_output = left
        if old_input and old_output:
            new_connections.append([old_input, old_output])
        else:
            assert False, "[ERROR] Could not find data_in and data_out for pond tile."

        # Remove const PE instance and its connections
        if pe_to_remove in instance_dict:
            del instance_dict[pe_to_remove]
        filtered_conns = [
            c for c in new_connections if pe_to_remove not in c[0] and pe_to_remove not in c[1]
        ]
        mat["connections"] = filtered_conns

        # Generate FIFO bypass config for the psum PE
        psum_pe = "op_hcompute_output_cgra_stencil_1$inner_compute$float_DW_fp_add_"
        bypass_cfg = {psum_pe: {"input_fifo_bypass": [1, 0, 0], "output_fifo_bypass": 1}}
        bypass_path = os.path.join(bin_path, "PE_fifos_bypass_config.json")
        print(f"Writing PE_fifos_bypass_config to {bypass_path}")
        with open(bypass_path, "w") as f:
            json.dump(bypass_cfg, f, indent=2)

        # Update the output-IO tile’s metadata to match matrix_height size
        out_io = "io16_hw_output_stencil_op_hcompute_hw_output_stencil_write_0"
        md = instance_dict[out_io]["metadata"]["in2glb_0"]
        md["extent"] = [int(self.halide_gen_args_dict["matrix_height"])]
        md["cycle_starting_addr"] = [0]
        md["cycle_stride"] = [1]

        # Update the input-IO tile’s metadata to match matrix size
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
            io_name = (
                f"io16in_vector_host_stencil_clkwrk_{idx}"
                f"_op_hcompute_vector_glb_stencil_{idx}_read_0"
            )
            # Add the IO instance
            instance_dict[io_name] = {
                "modref": "global.IO",
                "modargs": {"mode": ["String", "in"]},
                "metadata": vector_meta,
            }
            # Connect self.vector_host_stencil* to io.in
            mat["connections"].append(
                [
                    f"self.vector_host_stencil_clkwrk_{idx}"
                    f"_op_hcompute_vector_glb_stencil_{idx}_read_0",
                    f"{io_name}.in",
                ]
            )
            # Connect pe.data1 to io.out
            mat["connections"].append([f"{pe_name}.data1", f"{io_name}.out"])
        # Update the module’s type record to include vector_host_stencil fields
        type_fields = mat["type"][1]
        for idx, _ in mul_list:
            type_fields.append(
                [
                    f"vector_host_stencil_clkwrk_{idx}"
                    f"_op_hcompute_vector_glb_stencil_{idx}_read_0",
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
                    inst["modargs"]["value"][1] = "84'h00000420009004040000e"
                else:
                    raise RuntimeError(f"Instance {inst_name} is not a coreir.const")

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
                "shape": [
                    int(self.halide_gen_args_dict["matrix_width"]),
                    int(self.halide_gen_args_dict["matrix_height"]),
                ],
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

    def hack_for_stable_softmax_pass2_fp_rv(self, json_path, bin_path):

        with open(json_path, "r") as f:
            design = json.load(f)

        top_module = "stable_softmax_pass2_fp"
        tree_out_pond = "tree_3_stencil$ub_tree_3_stencil_BANK_0_garnet"
        tree_out_pond_clk = "tree_3_stencil$ub_tree_3_stencil_BANK_0_clk_en_const"

        # Const PE instance to remove
        pe_to_remove = "op_hcompute_output_cgra_stencil$inner_compute$const_"

        # Locate "stable_softmax_pass2_fp" module
        global_modules = design["namespaces"]["global"]["modules"]
        if top_module not in global_modules:
            print(f"WARNING: Module '{top_module}' not found in design. No hack applied.")
            return
        stable_softmax_pass2_fp = global_modules[top_module]

        # Remove the tile instance and its clk_en_const
        instance_dict = stable_softmax_pass2_fp.get("instances", {})
        if tree_out_pond in instance_dict:
            del instance_dict[tree_out_pond]
        if tree_out_pond_clk in instance_dict:
            del instance_dict[tree_out_pond_clk]

        # Remove references to the tile from connections,
        #    capturing upstream => old_input, downstream => old_output
        old_input = None
        old_output = None
        new_connections = []

        connection_list = stable_softmax_pass2_fp.get("connections", [])
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
        stable_softmax_pass2_fp["connections"] = final_connections

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

        # Remove replicated IOs to stream vec max and only keep one for broadcasting to subtraction PEs
        max_io_prefix = "io16in_vec_max_host_stencil_clkwrk_"
        all_inst = list(instance_dict.keys())
        vecmax_insts = [inst for inst in all_inst if inst.startswith(max_io_prefix)]
        if len(vecmax_insts) > 1:
            # Sort them and keep the first
            vecmax_insts.sort()
            keep_inst = vecmax_insts[0]
            remove_insts = vecmax_insts[1:]

            # Remove from instance_dict
            for r in remove_insts:
                del instance_dict[r]

            # The module "type" is stable_softmax_pass2_fp["type"] = ["Record",[...]]
            type_list = stable_softmax_pass2_fp["type"][1]

            # Convert instance name -> the corresponding type prefix
            # e.g. "io16in_vec_max_host_stencil_clkwrk_8_op_hcompute_vec_max_glb_stencil_2_read_0"
            # -> "vec_max_host_stencil_clkwrk_8_op_hcompute_vec_max_glb_stencil_2_read_0"
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
                if "vec_max_host_stencil_clkwrk_" not in field_name:
                    # Not a vecmax field => keep
                    new_type_list.append(field_pair)
                    continue
                # It's a vecmax field => keep only if it belongs to keep_prefix
                # (i.e. starts with keep_prefix)
                if field_name.startswith(keep_prefix):
                    new_type_list.append(field_pair)
                # else skip
            stable_softmax_pass2_fp["type"][1] = new_type_list

            # Fix up connections: if referencing removed inst or fields, unify to keep inst
            new_conn = []
            for left, right in stable_softmax_pass2_fp["connections"]:
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

            stable_softmax_pass2_fp["connections"] = new_conn

        # Deduplicate final connections: remove exact duplicates
        final_dedup = []
        seen = set()
        for c in stable_softmax_pass2_fp["connections"]:
            # Represent connection as tuple (left, right)
            t = tuple(c)
            if t not in seen:
                seen.add(t)
                final_dedup.append(c)

        stable_softmax_pass2_fp["connections"] = final_dedup

        # Configure the IO to read the same addr of GLB repeatedly
        # TODO: this should be replaced by reading from MEM tile instead for power efficiency
        for inst_name, inst_config in instance_dict.items():
            if "io16in_vec_max" in inst_name and inst_config["modref"] == "global.IO":
                inst_config["metadata"]["glb2out_0"]["read_data_stride"] = [0]

        # Overwrite the JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

    def hack_for_stable_softmax_pass3_fp_rv(self, json_path, bin_path):
        return self.hack_for_stable_softmax_pass3_fp_static(json_path, bin_path)

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

    def hack_for_layer_norm_pass2_fp_rv(self, json_path, bin_path):

        with open(json_path, "r") as f:
            design = json.load(f)

        top_module = "layer_norm_pass2_fp"
        tree_out_pond = "tree_3_stencil$ub_tree_3_stencil_BANK_0_garnet"
        tree_out_pond_clk = "tree_3_stencil$ub_tree_3_stencil_BANK_0_clk_en_const"

        # Const PE instance to remove
        pe_to_remove = "op_hcompute_output_cgra_stencil$inner_compute$const_"

        # Locate "layer_norm_pass2_fp" module
        global_modules = design["namespaces"]["global"]["modules"]
        if top_module not in global_modules:
            print(f"WARNING: Module '{top_module}' not found in design. No hack applied.")
            return
        layer_norm_pass2_fp = global_modules[top_module]

        # Remove the tile instance and its clk_en_const
        instance_dict = layer_norm_pass2_fp.get("instances", {})
        if tree_out_pond in instance_dict:
            del instance_dict[tree_out_pond]
        if tree_out_pond_clk in instance_dict:
            del instance_dict[tree_out_pond_clk]

        # Remove references to the tile from connections,
        #    capturing upstream => old_input, downstream => old_output
        old_input = None
        old_output = None
        new_connections = []

        connection_list = layer_norm_pass2_fp.get("connections", [])
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
        layer_norm_pass2_fp["connections"] = final_connections

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

        # Remove replicated IOs to stream vec max and only keep one for broadcasting to subtraction PEs
        max_io_prefix = "io16in_vec_avg_host_stencil_clkwrk_"
        all_inst = list(instance_dict.keys())
        vecmax_insts = [inst for inst in all_inst if inst.startswith(max_io_prefix)]
        if len(vecmax_insts) > 1:
            # Sort them and keep the first
            vecmax_insts.sort()
            keep_inst = vecmax_insts[0]
            remove_insts = vecmax_insts[1:]

            # Remove from instance_dict
            for r in remove_insts:
                del instance_dict[r]

            # The module "type" is layer_norm_pass2_fp["type"] = ["Record",[...]]
            type_list = layer_norm_pass2_fp["type"][1]

            # Convert instance name -> the corresponding type prefix
            # e.g. "io16in_vec_avg_host_stencil_clkwrk_8_op_hcompute_vec_avg_glb_stencil_2_read_0"
            # -> "vec_avg_host_stencil_clkwrk_8_op_hcompute_vec_avg_glb_stencil_2_read_0"
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
                if "vec_avg_host_stencil_clkwrk_" not in field_name:
                    # Not a vecmax field => keep
                    new_type_list.append(field_pair)
                    continue
                # It's a vecmax field => keep only if it belongs to keep_prefix
                # (i.e. starts with keep_prefix)
                if field_name.startswith(keep_prefix):
                    new_type_list.append(field_pair)
                # else skip
            layer_norm_pass2_fp["type"][1] = new_type_list

            # Fix up connections: if referencing removed inst or fields, unify to keep inst
            new_conn = []
            for left, right in layer_norm_pass2_fp["connections"]:
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

            layer_norm_pass2_fp["connections"] = new_conn

        # Deduplicate final connections: remove exact duplicates
        final_dedup = []
        seen = set()
        for c in layer_norm_pass2_fp["connections"]:
            # Represent connection as tuple (left, right)
            t = tuple(c)
            if t not in seen:
                seen.add(t)
                final_dedup.append(c)

        layer_norm_pass2_fp["connections"] = final_dedup

        # Configure the IO to read the same addr of GLB repeatedly
        # TODO: this should be replaced by reading from MEM tile instead for power efficiency
        for inst_name, inst_config in instance_dict.items():
            if "io16in_vec_avg" in inst_name and inst_config["modref"] == "global.IO":
                inst_config["metadata"]["glb2out_0"]["read_data_stride"] = [0]

        # Overwrite the JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

    def hack_for_layer_norm_pass3_fp_rv(self, json_path, bin_path):
        return self.hack_for_layer_norm_pass3_fp_static(json_path, bin_path)

    def hack_for_gelu_pass2_fp_rv(self, json_path, bin_path):
        return self.hack_for_gelu_pass2_fp_static(json_path, bin_path)

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
        pond_tpl = {
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
            inst = copy.deepcopy(pond_tpl)
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
        conns = avg["connections"]

        # Remove constant PEs
        for inst in list(instances):
            if inst.startswith(const_prefix):
                del instances[inst]
        filtered = [c for c in conns if const_prefix not in c[0] and const_prefix not in c[1]]

        # Replace each Mem tile with a Pond instance named "<mem>$pond"
        pond_tpl = {
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

        mem_to_pond = {}
        for name, inst in list(instances.items()):
            if inst.get("genref") == "cgralib.Mem":
                pond_name = f"{name}$pond"
                mem_to_pond[name] = pond_name
                new_inst = copy.deepcopy(pond_tpl)
                new_inst["genargs"]["ID"][1] = pond_name
                instances[pond_name] = new_inst
                del instances[name]

        # Rename all connections: data_in_* -> data_in_pond_0, data_out_i -> data_out_pond_i
        def rename_port(tok):
            m = re.match(r"^(.+)\.(data_(?:in|out))_(\d+)$", tok)
            if m and m.group(1) in mem_to_pond:
                old, dtype, idx = m.group(1), m.group(2), m.group(3)
                new_name = mem_to_pond[old]
                if dtype == "data_in":
                    new_idx = "0"
                else:
                    new_idx = idx
                return f"{new_name}.{dtype}_pond_{new_idx}"
            for old, new in mem_to_pond.items():
                if tok.startswith(old + "."):
                    return tok.replace(old + ".", new + ".", 1)
            return tok

        avg["connections"] = [(rename_port(a), rename_port(b)) for (a, b) in filtered]

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

    def hack_for_get_e8m0_scale_test_fp_rv(self, json_path, bin_path):
        with open(json_path, "r") as f:
            design = json.load(f)

        top_module = "get_e8m0_scale_test_fp"
        instances = design["namespaces"]["global"]["modules"][top_module]["instances"]

        # Add extra_data metadata to all shift registers for bogus data init
        for inst_name, inst_conf in instances.items():
            if "$d_reg" in inst_name:
                assert inst_conf.get("genref") == "coreir.reg", f"Instance {inst_name} is not a shift register."
                if "metadata" not in inst_conf:
                    inst_conf["metadata"] = {}
                inst_conf["metadata"]["extra_data"] = 1

        # Overwrite the JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

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

        # Insert MEM tiles between MU IOs and tree_mem / tree_mu PEs
        mem_tpl = {
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
                tpl = copy.deepcopy(mem_tpl)
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

        # Configure GLB store DMA to extract valid data
        for instance_name, instance_conf in instances.items():
            if re.match(r"io16_.*_output", instance_name):
                # Two cases:
                # 1. vec_width is 64, meaning we only skip at the very beginning
                # 2. vec_width >=128 and vec_width % 64 == 0, meaning we have to skip pixels between loops
                assert (
                    int(self.halide_gen_args_dict["vec_width"]) >= 64
                    and int(self.halide_gen_args_dict["vec_width"]) % 64 == 0
                ), f"vec_width must be >= 64 and divisible by 64 as mx block size is 64"
                if int(self.halide_gen_args_dict["vec_width"]) == 64:
                    instance_conf["metadata"]["in2glb_0"]["cycle_starting_addr"][0] = int(
                        self.halide_gen_args_dict["vec_height"]
                    )
                    instance_conf["metadata"]["in2glb_0"]["cycle_stride"] = [1]
                    instance_conf["metadata"]["in2glb_0"]["dimensionality"] = 1
                    instance_conf["metadata"]["in2glb_0"]["extent"] = [
                        int(self.halide_gen_args_dict["vec_height"])
                    ]
                    instance_conf["metadata"]["in2glb_0"]["write_data_starting_addr"] = [0]
                    instance_conf["metadata"]["in2glb_0"]["write_data_stride"] = [1]
                else:
                    instance_conf["metadata"]["in2glb_0"]["cycle_starting_addr"][0] = int(
                        self.halide_gen_args_dict["vec_height"]
                    )
                    instance_conf["metadata"]["in2glb_0"]["cycle_stride"] = [
                        1,
                        int(self.halide_gen_args_dict["vec_height"]) + 1,
                    ]
                    instance_conf["metadata"]["in2glb_0"]["dimensionality"] = 2
                    instance_conf["metadata"]["in2glb_0"]["extent"] = [
                        int(self.halide_gen_args_dict["vec_height"]),
                        int(self.halide_gen_args_dict["vec_width"])
                        # The actual reduction tree width is double the size of mu_i
                        // (int(self.halide_gen_args_dict["mu_i"]) * 2),
                    ]
                    instance_conf["metadata"]["in2glb_0"]["write_data_starting_addr"] = [0]
                    instance_conf["metadata"]["in2glb_0"]["write_data_stride"] = [1, 1]

        # Apply-scale pipeline + scale broadcast + output IOs
        SCALE_PE_SRC = "op_hcompute_scale_output_glb_stencil$inner_compute$get_shared_exp_i3200i78_i1489.O0"
        APPLY_SCALE_INSTR = "84'h0220001000550015300a9"
        DATA_PACKING_INSTR = "84'h0200201104128c0d3001d"

        shift_reg_tpl = {
            "genref": "coreir.reg",
            "genargs": {"width": ["Int", 16]},
            "modargs": {
                "clk_posedge": ["Bool", True],
                "init": [["BitVector", 16], "16'h0000"],
            },
            "metadata": {"extra_data": 2},
        }
        pe_tpl = {
            "modref": "global.PE"
        }
        apply_scale_const_tpl = {
            "genref": "coreir.const",
            "genargs": {"width": ["Int", 84]},
            "modargs": {"value": [["BitVector", 84], APPLY_SCALE_INSTR]},
        }
        # Const template for data_packing PE instruction
        data_packing_const_tpl = {
            "genref": "coreir.const",
            "genargs": {"width": ["Int", 84]},
            "modargs": {"value": [["BitVector", 84], DATA_PACKING_INSTR]},
        }

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
            instances[pe_name] = copy.deepcopy(pe_tpl)
            instances[c0_name] = copy.deepcopy(apply_scale_const_tpl)
            add_conn_once(f"{pe_name}.inst", f"{c0_name}.out")

            # Shift-FIFO chain
            head = None
            prev = None
            if n_regs > 0:
                for k in range(n_regs):
                    rname = f"{naming_base}$d_reg__U0$reg{k}"
                    instances[rname] = copy.deepcopy(shift_reg_tpl)
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
            assert SCALE_PE_SRC.split(".")[0] in instances, f"SCALE_PE_SRC {SCALE_PE_SRC} not found in instances, check get scale PE name."
            add_conn_once(SCALE_PE_SRC, f"{pe_name}.data1")

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

            instances[data_packing_pe] = copy.deepcopy(pe_tpl)
            instances[data_packing_const] = copy.deepcopy(data_packing_const_tpl)
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

            instances[data_packing_pe] = copy.deepcopy(pe_tpl)
            instances[data_packing_const] = copy.deepcopy(data_packing_const_tpl)
            add_conn_once(f"{data_packing_pe}.inst", f"{data_packing_const}.out")

            add_conn_once(apply_outputs["mem_with_delay"][idx0], f"{data_packing_pe}.data1")
            add_conn_once(apply_outputs["mem_with_delay"][idx1], f"{data_packing_pe}.data0")

            orig_idx = idx0
            pending_ios_mem_with_delay.append((orig_idx, data_packing_pe))

        # Instantiate output IOs, MEM with delay first, then MEM no delay
        pending_ios_mem_with_delay = sorted(pending_ios_mem_with_delay, key=lambda x: x[0])
        pending_ios_mem_no_delay  = sorted(pending_ios_mem_no_delay,  key=lambda x: x[0])
        quantized_output_ios = []

        # Assign sorted index to all quantized output IOs, MEM with delay first, then MEM no delay
        # MEM with delay indexes
        for ascending_idx, (_, data_packing_pe) in enumerate(pending_ios_mem_with_delay):
            io_name = (f"io16_quantized_output_stencil_clkwrk_{ascending_idx}_op_hcompute_quantized_output_stencil_{ascending_idx}_write_0")
            instances[io_name] = copy.deepcopy(io_tpl)
            add_conn_once(f"{data_packing_pe}.O0", f"{io_name}.in")
            quantized_output_ios.append(io_name)

        # MEM no delay indexes
        idx_offset = len(pending_ios_mem_with_delay)
        for ascending_idx, (_, data_packing_pe) in enumerate(pending_ios_mem_no_delay):
            io_name = (f"io16_quantized_output_stencil_clkwrk_{idx_offset + ascending_idx}_op_hcompute_quantized_output_stencil_{idx_offset + ascending_idx}_write_0")
            instances[io_name] = copy.deepcopy(io_tpl)
            add_conn_once(f"{data_packing_pe}.O0", f"{io_name}.in")
            quantized_output_ios.append(io_name)


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
                "shape": [int(self.halide_gen_args_dict["mu_i"]), int(self.halide_gen_args_dict["vec_width"])*int(self.halide_gen_args_dict["vec_height"])//2//int(self.halide_gen_args_dict["mu_i"])],
            }
        )

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

        # Reorder IO out instances by clkwrk index
        from collections import OrderedDict

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


class GlobalDesignHacker:
    """
    A class to handle design JSON modifications (aka 'hacks') for all apps.
    """

    def __init__(self):
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

    # Perform global hack of design_top.json to add MU prefix for MU IOs
    global_design_top_hacker.add_mu_prefix_to_io(args.design_top_json)

    ## Hack design_meta_halide.json
    global_design_meta_hacker = GlobalDesignMetaHakcer()
    global_design_meta_hacker.hack_mu_inputs(args.design_meta_halide_json)


if __name__ == "__main__":
    main()

#!/usr/bin/env python3

import os, re, argparse, json
from pretty_format_json import pretty_format_json

APPS_NEEDING_HACKS = [
    "scalar_reduction",
    "scalar_reduction_fp",
    "scalar_max_fp",
    "stable_softmax_pass2_fp",
    "stable_softmax_pass3_fp",
    "scalar_avg_fp",
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

    def hack_design_if_needed(self, testname, json_path):
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
        hack_method_name = f"hack_for_{testname}"
        hack_method = getattr(self, hack_method_name, None)
        if hack_method is None:
            raise AttributeError(
                f"\033[91mError: Method '{hack_method_name}' does not exist for test '{testname}'.\033[0m"
            )
        hack_method(json_path)

    def hack_for_scalar_reduction(self, json_path):

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
            print(
                f"WARNING: Module '{top_module}' not found in design. No hack applied."
            )
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
            assert (
                "in2regfile_0" in config
            ), "Error: 'in2regfile_0' not found in config."
            input_pipelining_regs = INPUT_PIPELINING_REGS
            config["in2regfile_0"]["cycle_starting_addr"] = [input_pipelining_regs]

            # For regfile2out_0, set cycle_starting_addr to in2regfile_0 + 1
            assert (
                "regfile2out_0" in config
            ), "Error: 'regfile2out_0' not found in config."
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
        assert (
            io_out_name in instance_dict
        ), f"Error: '{io_out_name}' not found in instance_dict."
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

    def hack_for_scalar_reduction_fp(self, json_path):

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
            print(
                f"WARNING: Module '{top_module}' not found in design. No hack applied."
            )
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
            assert (
                "in2regfile_0" in config
            ), "Error: 'in2regfile_0' not found in config."
            input_pipelining_regs = INPUT_PIPELINING_REGS
            config["in2regfile_0"]["cycle_starting_addr"] = [input_pipelining_regs]

            # For regfile2out_0, set cycle_starting_addr to in2regfile_0 + 1
            assert (
                "regfile2out_0" in config
            ), "Error: 'regfile2out_0' not found in config."
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
        assert (
            io_out_name in instance_dict
        ), f"Error: '{io_out_name}' not found in instance_dict."
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

    def hack_for_scalar_max_fp(self, json_path):

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
            print(
                f"WARNING: Module '{top_module}' not found in design. No hack applied."
            )
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
        psum_pond_name = "max_output_cgra_inner_stencil$ub_max_output_cgra_inner_stencil_BANK_0_garnet"
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
            assert (
                "in2regfile_0" in config
            ), "Error: 'in2regfile_0' not found in config."
            input_pipelining_regs = INPUT_PIPELINING_REGS
            config["in2regfile_0"]["cycle_starting_addr"] = [input_pipelining_regs]

            # For regfile2out_0, set cycle_starting_addr to in2regfile_0 + 1
            assert (
                "regfile2out_0" in config
            ), "Error: 'regfile2out_0' not found in config."
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
        assert (
            io_out_name in instance_dict
        ), f"Error: '{io_out_name}' not found in instance_dict."
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

    def hack_for_stable_softmax_pass2_fp(self, json_path):

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
            print(
                f"WARNING: Module '{top_module}' not found in design. No hack applied."
            )
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
            assert (
                "in2regfile_0" in config
            ), "Error: 'in2regfile_0' not found in config."
            input_pipelining_regs = INPUT_PIPELINING_REGS
            # Set reduction tree delay
            reduction_tree_delay = 1 # It's one because we have fp.exp
            config["in2regfile_0"]["cycle_starting_addr"] = [input_pipelining_regs + reduction_tree_delay]

            # For regfile2out_0, set cycle_starting_addr to in2regfile_0 + 1
            assert (
                "regfile2out_0" in config
            ), "Error: 'regfile2out_0' not found in config."
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
        assert (
            io_out_name in instance_dict
        ), f"Error: '{io_out_name}' not found in instance_dict."
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
                    return name[len("io16in_"):]
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

    def hack_for_stable_softmax_pass3_fp(self, json_path):

        with open(json_path, "r") as f:
            design = json.load(f)

        top_module = "stable_softmax_pass3_fp"

        # Locate "stable_softmax_pass2_fp" module
        global_modules = design["namespaces"]["global"]["modules"]
        if top_module not in global_modules:
            print(
                f"WARNING: Module '{top_module}' not found in design. No hack applied."
            )
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
                    return name[len("io16in_"):]
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
            if "io16in_pass2_sum_host_stencil" in inst_name and inst_config["modref"] == "global.IO":
                inst_config["metadata"]["glb2out_0"]["read_data_stride"] = [0]

        # Overwrite the JSON
        with open(json_path, "w") as f:
            f.write(pretty_format_json(design))

    def hack_for_scalar_avg_fp(self, json_path):

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
            print(
                f"WARNING: Module '{top_module}' not found in design. No hack applied."
            )
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
            assert (
                "in2regfile_0" in config
            ), "Error: 'in2regfile_0' not found in config."
            input_pipelining_regs = INPUT_PIPELINING_REGS
            config["in2regfile_0"]["cycle_starting_addr"] = [input_pipelining_regs]

            # For regfile2out_0, set cycle_starting_addr to in2regfile_0 + 1
            assert (
                "regfile2out_0" in config
            ), "Error: 'regfile2out_0' not found in config."
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
        assert (
            io_out_name in instance_dict
        ), f"Error: '{io_out_name}' not found in instance_dict."
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


class GlobalDesignHacker:
    """
    A class to handle design JSON modifications (aka 'hacks') for all apps.
    """

    def __init__(self):
        # No filtering by apps
        pass

    def remove_stencil_mem_rv(self, json_path):
        """
        Remove "port_controller" instances and "hw_output_*_write_valid" instances
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
            #  - or matches "hw_output_.*_write_valid"
            instances_dict = mod_def["instances"]
            to_remove = []

            # Collect all instance names we want to remove
            for inst_name in instances_dict.keys():
                if "port_controller" in inst_name:
                    to_remove.append(inst_name)
                # Matches "hw_output_<anything>_write_valid"
                elif re.search(r"hw_output_.*_write_valid", inst_name):
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
    args = parser.parse_args()

    # We can expand this list if more apps need hacking
    apps_needing_hacks = APPS_NEEDING_HACKS

    # ----------------- Perform selected hacks -----------------
    selected_design_top_hacker = SelectedDesignHacker(hack_apps=apps_needing_hacks)

    # Perform hack if testname is in that list, otherwise skip
    selected_design_top_hacker.hack_design_if_needed(
        args.testname, args.design_top_json
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

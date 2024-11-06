import json
import shutil
import subprocess
import argparse
import os
import re
import sys
import glob
import math

def get_shape_args(app_name):
    app_params_path = '/aha/aha/util/application_parameters.json'
    assert os.path.exists(app_params_path), f"Application parameters file not found at {app_params_path}"
    with open(app_params_path, 'r') as file:
        app_params = json.load(file)
        halide_gen_args = app_params[f'apps/{app_name}']['default']['HALIDE_GEN_ARGS']
    args_dict = {arg.split('=')[0]: arg.split('=')[1] for arg in halide_gen_args.split(' ')}
    return args_dict

def run_app(app_name, run_test=True):
    app_path = f"/aha/Halide-to-Hardware/apps/hardware_benchmarks/apps/{app_name}"
    map_commands = [
        "make clean",
        f"aha map apps/{app_name} --chain",
        f"aha pnr apps/{app_name} --width 32 --height 16"
    ]
    if run_test:
        map_commands.append(f"aha test apps/{app_name} --dense-fp")
    try:
        for command in map_commands:
            # if "test" in command:
            #     breakpoint()
            result = subprocess.run(command, shell=True, check=True, cwd=app_path)
            if result.returncode != 0:
                print(f"Command failed: {command}")
                break
    except subprocess.CalledProcessError as e:
        print(f"An error occurred: {e}")
        sys.exit(1)

def run_pnr(app_name, generate_bitstream_only=False):
    app_path = f"/aha/Halide-to-Hardware/apps/hardware_benchmarks/apps/{app_name}"
    pnr_commands = f"aha pnr apps/{app_name} --width 32 --height 16"
    if generate_bitstream_only:
        pnr_commands += " --generate-bitstream-only"
    subprocess.run(pnr_commands, shell=True, check=True, cwd=app_path)

def replace_const_instr(preload_d, stream_d):
    def is_const_dict(d):
        """Check if a dictionary is a 'const' dict."""
        return (
            isinstance(d, dict) and
            d.get("genref") == "coreir.const" and
            "genargs" in d and
            "modargs" in d and
            d.get("genargs").get("width")[1] == 84
        )

    def find_const_dicts(d, path=()):
        """
        Recursively find all 'const' dictionaries in a nested dictionary.
        
        Returns a list of tuples: (path_to_const_dict, const_dict)
        """
        const_dicts = []
        if is_const_dict(d):
            const_dicts.append((path, d))
        elif isinstance(d, dict):
            for k, v in d.items():
                const_dicts.extend(find_const_dicts(v, path + (k,)))
        elif isinstance(d, list):
            for idx, item in enumerate(d):
                const_dicts.extend(find_const_dicts(item, path + (idx,)))
        return const_dicts

    def set_in_nested(d, path, value):
        """Set a value in a nested dictionary given a path."""
        for key in path[:-1]:
            d = d[key]
        d[path[-1]] = value

    # Find all 'const' dicts in preload_d and stream_d
    preload_consts = find_const_dicts(preload_d)
    stream_consts = find_const_dicts(stream_d)

    # Check and print the number of 'const' dicts
    num_preload_consts = len(preload_consts)
    num_stream_consts = len(stream_consts)

    print(f"Number of const dicts in preload_d: {num_preload_consts}")
    print(f"Number of const dicts in stream_d: {num_stream_consts}")

    if num_preload_consts != num_stream_consts:
        diff_consts = [const for const in preload_consts if const not in stream_consts]
        raise ValueError("Number of const dicts do not match. Aborting replacement.")

    # Replace 'const' dicts in preload_d with those from stream_d, in order
    for i, (path, _) in enumerate(preload_consts):
        const_dict_from_stream = stream_consts[i][1]
        set_in_nested(preload_d, path, const_dict_from_stream)

# helper func to replace 'input' with 'kernel' in given dict or lists
def replace_input_with_kernel(obj):
    if isinstance(obj, type({})):
        new_dict = {}
        for key, value in obj.items():
            new_key = key.replace('input', 'kernel')
            new_dict[new_key] = replace_input_with_kernel(value)
        return new_dict
    elif isinstance(obj, list):
        new_list = []
        for item in obj:
            if isinstance(item, (list, tuple)):
                new_list.append(replace_input_with_kernel(item))
            elif isinstance(item, str):
                new_list.append(item.replace('input', 'kernel'))
            else:
                new_list.append(item)
        return new_list
    elif isinstance(obj, str):
        return obj.replace('input', 'kernel')
    else:
        return obj

# add kernel read types in cu_op_hcompute_depthwise_conv_stencil_*['type']
def add_kernel_read_types_and_conns(d):
    assert isinstance(d['namespaces']['global']['modules'], type({}))
    for key, value in d['namespaces']['global']['modules'].items():
        if 'cu_op_hcompute_depthwise_conv_stencil' in key:
            # add kernel read types
            assert isinstance(value['type'][1], type([]))
            type_list = value['type'][1]
            kernel_type_list = []
            for item in type_list:
                kernel_type_list.append(item)
                # check if the item's name contains "input_host"
                if "input_host" in item[0]:
                    # create a new item with "input_host" replaced by "kernel_host"
                    new_item = [item[0].replace("input_host", "kernel_host"), item[1]]
                    # Append the new item to the updated list
                    kernel_type_list.append(new_item)
            value['type'][1] = kernel_type_list
            # add kernel read connections with PEs
            assert isinstance(value['connections'], type([]))
            stencil_conn = value['connections']
            kernel_stencil_conn = []
            for conn in stencil_conn:
                if isinstance(conn, list) and len(conn) == 2:
                    if 'input_host' in conn[0] and 'inner_compute' in conn[1]:
                        kernel_conn = conn.copy()
                        kernel_conn[0] = kernel_conn[0].replace('input', 'kernel')
                        kernel_conn[1] = kernel_conn[1].replace('data0', 'data1') if 'data0' in kernel_conn[1] else kernel_conn[1].replace('data1', 'data0')
                        kernel_stencil_conn.append(kernel_conn)
            stencil_conn.extend(kernel_stencil_conn)

def add_kernel_global_wrapper_read_write_conn(d):
    # copy cu_op_hcompute_kernel_host_global_wrapper_stencil
    assert isinstance(d['namespaces']['global']['modules'], type({}))
    module_keys = list(d['namespaces']['global']['modules'].keys())
    for key in module_keys:
        value = d['namespaces']['global']['modules'][key]
        if 'cu_op_hcompute_input_host_global_wrapper_global_wrapper_stencil' in key or 'cu_op_hcompute_input_host_global_wrapper_stencil' in key:
            kernel_stencil_dict = replace_input_with_kernel(value)
            d['namespaces']['global']['modules'][key.replace('input', 'kernel')] = kernel_stencil_dict

def add_dw_conv_kernel_read_type(d, unroll):
    assert isinstance(d['namespaces']['global']['modules']['depthwise_conv_preload_fp'], type({}))
    depthwise_conv_dict = d['namespaces']['global']['modules']['depthwise_conv_preload_fp']
    assert isinstance(depthwise_conv_dict['type'][1], type([]))
    type_list = depthwise_conv_dict['type'][1]
    kernel_type_list = []
    for item in type_list:
        kernel_type_list.append(item)
        # check if the item's name contains "input_host"
        if "input_host" in item[0]:
            # create a new item with "input_host" replaced by "kernel_host"
            pattern = r"(_clkwrk_)(\d+)(_)"  # Matches "_clkwrk_" followed by a number and "_"
            match = re.search(pattern, item[0])
            if match:
                prefix = match.group(1)
                index = int(match.group(2))
                suffix = match.group(3)
                new_index = index + int(unroll)
                new_item = [item[0].replace("input_host", "kernel_host").replace(f"{prefix}{index}{suffix}", f"{prefix}{new_index}{suffix}"), item[1]]
                # Append the new item to the updated list
                kernel_type_list.append(new_item)
    depthwise_conv_dict['type'][1] = kernel_type_list

def generate_PE_name_dict(stream_connections, preload_connections):
    # Initialize an empty dictionary to store the PE name mapping
    PE_name_dict = {}

    # Convert preload_connections to a dictionary for quick lookup of node pairs
    preload_lookup = {}
    for conn in preload_connections:
        # Ensure connection is a list with exactly two elements
        if len(conn) == 2:
            preload_lookup[conn[0]] = conn[1]
            preload_lookup[conn[1]] = conn[0]

    # Loop through each connection in depthwise_conv_stream_dict['connections']
    for conn in stream_connections:
        # Ensure each connection is a list with exactly two elements
        if len(conn) == 2:
            node_0, node_1 = conn[0], conn[1]

            # Check if either node has both ".inst" and "float_DW_fp_mul"
            if "float_DW_fp_mul" in node_0 and node_0.endswith(".inst"):
                # The other node is node_1
                other_node = node_1
                stream_PE_name = node_0  # stream_PE_name should be the original node (node_0)
            elif "float_DW_fp_mul" in node_1 and node_1.endswith(".inst"):
                # The other node is node_0
                other_node = node_0
                stream_PE_name = node_1  # stream_PE_name should be the original node (node_1)
            else:
                # Skip this connection if neither node matches the pattern
                continue

            # Find the matching node in the preload dictionary
            if other_node in preload_lookup:
                preload_mapped_node = preload_lookup[other_node]

                # Remove the ".inst" suffix from both the key and value
                stream_PE_name = stream_PE_name.replace(".inst", "")
                preload_PE_name = preload_mapped_node.replace(".inst", "")

                # Add to PE_name_dict
                PE_name_dict[stream_PE_name] = preload_PE_name

    return PE_name_dict

# create pond schedules in depthwise_conv['instances']
# e.g. kernel_host_global_wrapper_stencil$ub_kernel_host_global_wrapper_stencil_BANK_0_clk_en_const
# e.g. kernel_host_global_wrapper_stencil$ub_kernel_host_global_wrapper_stencil_BANK_0_garnet
# e.g. io16in_kernel_host_stencil_op_hcompute_kernel_host_global_wrapper_stencil_read_0
def add_pond_schedules_and_conns(preload_d, stream_d, in_img, ksize, stride, n_ic, unroll):

    depthwise_conv_stream_dict = stream_d['namespaces']['global']['modules']['depthwise_conv']
    assert isinstance(depthwise_conv_stream_dict, type({}))
    depthwise_conv_preload_dict = preload_d['namespaces']['global']['modules']['depthwise_conv_preload_fp']
    assert isinstance(depthwise_conv_preload_dict, type({}))

    # copy all kernel instances from stream to preload
    kernel_pairs = {k: v for k, v in depthwise_conv_stream_dict['instances'].items() if 'kernel' in k}
    depthwise_conv_preload_dict['instances'].update(kernel_pairs)

    # copy all kernel connections from stream to preload
    assert isinstance(depthwise_conv_stream_dict['connections'], list)
    assert isinstance(depthwise_conv_preload_dict['connections'], list)

    # Generate a dictionary to map PE names from the stream to the preload
    PE_name_dict = generate_PE_name_dict(depthwise_conv_stream_dict['connections'], depthwise_conv_preload_dict['connections'])

    for conn in depthwise_conv_stream_dict['connections']:
        # Ensure each connection is a list of two strings
        assert isinstance(conn, list) and len(conn) == 2
        
        # Extract both nodes
        node_0 = conn[0]
        node_1 = conn[1]

        # Check if either node contains "kernel"
        if 'kernel' in node_0 or 'kernel' in node_1:
            
            # If node_0 contains the pattern (e.g., "float_DW_fp_mul_<id>.data0"), replace the name and suffix
            if "float_DW_fp_mul" in node_0 and ".data0" in node_0:
                # Extract the specific pattern (e.g., float_DW_fp_mul_i2764_i2586)
                for key in PE_name_dict:
                    if key in node_0:
                        # Replace the key pattern with its new name from PE_name_dict
                        node_0 = node_0.replace(key, PE_name_dict[key])
                        # Replace ".data0" with ".data1"
                        node_0 = node_0.replace(".data0", ".data1")
                        break  # Stop once we have replaced the key

            # Do the same for node_1 if it contains the pattern (although this wasn't specified, adding for robustness)
            if "float_DW_fp_mul" in node_1 and ".data0" in node_1:
                for key in PE_name_dict:
                    if key in node_1:
                        node_1 = node_1.replace(key, PE_name_dict[key])
                        node_1 = node_1.replace(".data0", ".data1")
                        break

            # Create the updated connection and append to depthwise_conv_preload_dict['connections']
            updated_conn = [node_0, node_1]
            depthwise_conv_preload_dict['connections'].append(updated_conn)

def edit_hcompute_depthwise_conv_stencil(d):
    assert isinstance(d['namespaces']['global']['modules'], type({}))
    for key, value in d['namespaces']['global']['modules'].items():
        if 'hcompute_depthwise_conv_stencil' in key and "cu_op" not in key:
            # add in2_kernel_host_global_wrapper_stencil
            assert isinstance(value['type'][1], type([]))
            type_list = value['type'][1]
            kernel_type_list = []
            for item in type_list:
                kernel_type_list.append(item)
                # check if the item's name contains "input_host"
                if "input_host" in item[0]:
                    assert "in1" in item[0], "should have pattern like in1_input_host_global_wrapper_stencil"
                    # create a new item with "input_host" replaced by "kernel_host"
                    new_item = [item[0].replace("in1_input_host", "in2_kernel_host"), item[1]]
                    # Append the new item to the updated list
                    kernel_type_list.append(new_item)
            value['type'][1] = kernel_type_list
            # add kernel connections
            assert isinstance(value['connections'], type([]))
            stencil_conn = value['connections']
            kernel_stencil_conn = []
            for conn in stencil_conn:
                if isinstance(conn, list) and len(conn) == 2:
                    if 'in1_input_host' in conn[0] and 'mul' in conn[1]:
                        kernel_conn = conn.copy()
                        kernel_conn[0] = kernel_conn[0].replace('in1_input', 'in2_kernel')
                        kernel_conn[1] = kernel_conn[1].replace('data0', 'data1') if 'data0' in kernel_conn[1] else kernel_conn[1].replace('data1', 'data0')
                        kernel_stencil_conn.append(kernel_conn)
            stencil_conn.extend(kernel_stencil_conn)

def copy_hcompute_kernel_host_global_wrapper_stencil(d):
    # copy hcompute_input_host_global_wrapper_stencil and hcompute_input_host_global_wrapper_global_wrapper_stencil
    assert isinstance(d['namespaces']['global']['modules'], type({}))
    module_keys = list(d['namespaces']['global']['modules'].keys())
    for key in module_keys:
        value = d['namespaces']['global']['modules'][key]
        if "hcompute_input_host_global_wrapper" in key and "op" not in key:
            d['namespaces']['global']['modules'][key.replace('input', 'kernel')] = replace_input_with_kernel(value)

def create_kernel_host_global_wrapper_global_wrapper_stencil_ub(preload_d, stream_d):
    # Find the target dict in stream_d
    kernel_host_global_wrapper = stream_d['namespaces']['global']['modules']['kernel_host_global_wrapper_stencil_ub']
    assert isinstance(kernel_host_global_wrapper, type({}))
    
    # Recursive function to replace all occurrences of "_global_wrapper" with "_global_wrapper_global_wrapper"
    def replace_global_wrapper(data):
        if isinstance(data, dict):
            # For dictionaries, recursively replace keys and values
            new_dict = {}
            for key, value in data.items():
                # Replace "_global_wrapper" in key
                new_key = key.replace("_global_wrapper", "_global_wrapper_global_wrapper")
                # Recursively apply replacement to the value
                new_dict[new_key] = replace_global_wrapper(value)
            return new_dict
        elif isinstance(data, list):
            # For lists, recursively apply replacement to each item
            return [replace_global_wrapper(item) for item in data]
        elif isinstance(data, str):
            # For strings, replace occurrences of "_global_wrapper"
            return data.replace("_global_wrapper", "_global_wrapper_global_wrapper")
        else:
            # For other types (e.g., numbers), return the data unchanged
            return data

    # Apply the recursive replacement function to the target dict
    modified_kernel_host_global_wrapper = replace_global_wrapper(kernel_host_global_wrapper)
    
    # Rename the top-level key to 'kernel_host_global_wrapper_global_wrapper_stencil_ub'
    preload_d['namespaces']['global']['modules']['kernel_host_global_wrapper_global_wrapper_stencil_ub'] = modified_kernel_host_global_wrapper

def create_kernel_host_global_wrapper_stencil_ub(d):
    modules_dict = d['namespaces']['global']['modules']
    assert isinstance(modules_dict['input_host_global_wrapper_stencil_ub'], type({}))
    modules_dict['kernel_host_global_wrapper_stencil_ub'] = replace_input_with_kernel(modules_dict['input_host_global_wrapper_stencil_ub'])

def create_op_hcompute_kernel_host_global_wrapper_stencil(d):
    modules_dict = d['namespaces']['global']['modules']
    module_keys = list(modules_dict.keys())
    for key in module_keys:
        value = d['namespaces']['global']['modules'][key]
        if "op_hcompute_input_host_global_wrapper_stencil" in key and "__U" in key:
            d['namespaces']['global']['modules'][key.replace('input', 'kernel')] = replace_input_with_kernel(value)

def add_kernel_to_design_meta_halide(json_path, n_ic, ksize):
    with open(json_path, 'r') as file:
        design_meta_halide = json.load(file)
    kernel_exists = any(input_item.get("name") == "kernel_host_stencil" for input_item in design_meta_halide.get('IOs', {}).get('inputs', []))
    if kernel_exists:
        print("Kernel input already exists")
    else:
        new_kernel_input = {
            "bitwidth": 16,
            "datafile": "kernel_host_stencil.raw",
            "name": "kernel_host_stencil",
            "shape": [n_ic, ksize, ksize]
        }
        design_meta_halide['IOs']['inputs'].append(new_kernel_input)
        with open(json_path, 'w') as file:
            json.dump(design_meta_halide, file, indent=4)
            print("Kernel input added successfully.")

def update_pond_flush_latency(instance_dict, flush_latency_dict):
    for mem_name, mem_data in instance_dict.items():
        # Check if the memory has a corresponding latency in flush_latency_dict
        if mem_name in flush_latency_dict:
            latency = flush_latency_dict[mem_name]

            # Ensure the memory data contains 'modargs' and 'metadata'
            if "modargs" in mem_data and "metadata" in mem_data:
                modargs_config_list = mem_data["modargs"].get("config", [])
                metadata_config = mem_data["metadata"].get("config", {})

                # Check that modargs config follows the expected format
                if isinstance(modargs_config_list, list) and len(modargs_config_list) == 2:
                    modargs_config = modargs_config_list[1]  # The second element is the actual config dict

                    # Iterate over all keys in the modargs config
                    for key, modargs_entry in modargs_config.items():
                        # Check if the corresponding key exists in metadata config
                        if key in metadata_config:
                            metadata_entry = metadata_config[key]
                            modargs_cycle_starting_addr = modargs_entry.get("cycle_starting_addr", [])

                            # Update metadata "cycle_starting_addr" by adding latency to each value
                            updated_cycle_starting_addr = [
                                addr + latency for addr in modargs_cycle_starting_addr
                            ]

                            # Apply the updated starting addresses to the metadata
                            metadata_entry["cycle_starting_addr"] = updated_cycle_starting_addr

                    # After processing, update the original metadata with modified values
                    mem_data["metadata"]["config"] = metadata_config
    print("Pond flush latency updated successfully.")
    
def delay_inputs_to_wait_for_kernel(d, delay):
    # Function to delay inputs to wait for kernel streaming, used when n_ic > unroll
    instances_dict = d['namespaces']['global']['modules']['depthwise_conv_preload_fp']['instances']

    for inst_name, inst_data in instances_dict.items():
        if "io16in_input_host_stencil_clkwrk_" in inst_name or "io16_hw_output_global_wrapper_glb_stencil_clkwrk_" in inst_name:
            assert "metadata" in inst_data, f"metadata not found in instance {inst_name}"
            metadata_entry = inst_data["metadata"]
            assert len(metadata_entry) == 1, f"Expected only one metadata entry for {inst_name}"
            if "glb2out_0" in metadata_entry:
                metadata_entry['glb2out_0']['cycle_starting_addr'][0] += delay
                print(f"[INFO] Delaying {inst_name} cycle_starting_addr by {delay}")
            elif "in2glb_0" in metadata_entry:
                metadata_entry['in2glb_0']['cycle_starting_addr'][0] += delay
                print(f"[INFO] Delaying {inst_name} cycle_starting_addr by {delay}")
            else:
                raise ValueError(f"Invalid metadata entry for {inst_name}")
        elif "_port_controller_garnet" in inst_name or \
             "input_host_global_wrapper_global_wrapper_stencil$ub_input_host_global_wrapper_global_wrapper_stencil" in inst_name and "clk_en_const" not in inst_name:
            assert "metadata" in inst_data, f"metadata not found in instance {inst_name}"
            metadata_config = inst_data["metadata"]["config"]
            for key, metadata_entry in metadata_config.items():
                assert "cycle_starting_addr" in metadata_entry, f"cycle_starting_addr not found in {inst_name}['metadata']['config'][{key}]"
                metadata_entry["cycle_starting_addr"][0] += delay
                print(f"[INFO] Delaying {inst_name}['metadata']['config'][{key}]['cycle_starting_addr'] by {delay}")

def fix_pond_output_extent(d, in_img, n_ic, unroll):
    # Fix the output extent of pond to match input shape instead of output shape
    instances_dict = d['namespaces']['global']['modules']['depthwise_conv_preload_fp']['instances']
    for inst_name, inst_data in instances_dict.items():
        if "kernel_host_global_wrapper_stencil$ub_kernel_host_global_wrapper_stencil" in inst_name and "clk_en_const" not in inst_name:
            assert "metadata" in inst_data, f"metadata not found in instance {inst_name}"
            metadata_entry = inst_data["metadata"]
            assert len(metadata_entry) == 2, f"Expected only two metadata entry for {inst_name}: config and mode"
            assert "config" in metadata_entry, f"config not found in {inst_name}['metadata']"
            metadata_config = metadata_entry["config"]
            assert "regfile2out_0" in metadata_config, f"regfile2out_0 not found in {inst_name}['metadata']['config']"
            metadata_config["regfile2out_0"]["cycle_stride"] = [1, int(n_ic)//int(unroll)]
            print(f"[INFO] Setting {inst_name}['metadata']['config']['regfile2out_0']['cycle_stride'] to [1, {int(n_ic)//int(unroll)}]")
            metadata_config["regfile2out_0"]["dimensionality"] = 2
            print(f"[INFO] Setting {inst_name}['metadata']['config']['regfile2out_0']['dimensionality'] to 2")
            metadata_config["regfile2out_0"]["extent"] = [int(n_ic)//int(unroll), int(in_img)*int(in_img)]
            print(f"[INFO] Setting {inst_name}['metadata']['config']['regfile2out_0']['extent'] to [{int(n_ic)//int(unroll)}, {int(in_img)*int(in_img)}]")
            metadata_config["regfile2out_0"]["read_data_stride"] = [1, 0]
            print(f"[INFO] Setting {inst_name}['metadata']['config']['regfile2out_0']['read_data_stride'] to [1, 0]")

def align_kernel_with_input(d, flush_latency_dict, n_ic, unroll, ksize):
    # Two cases:
    # 1. input comes from IO directly, start addr is int(n_ic)//int(unroll)*int(ksize)*int(ksize) + num_regs_to_IO + Pond flush_latency + 3 (3 is hardcoded input broadcast pipelining latency)
    # 2. input comes from MEM tile, two cases:
    #    a. input comes from MEM tile port 0, start addr is MEM's tb2out_0's cycle_starting_addr + num_regs_to_MEM + Pond flush latency - MEM flush latency
    #    b. input comes from MEM tile port 1, start addr is MEM's tb2out_1's cycle_starting_addr + num_regs_to_MEM + Pond flush latency - MEM flush latency
    instances_dict = d['namespaces']['global']['modules']['depthwise_conv_preload_fp']['instances']
    connections_list = d['namespaces']['global']['modules']['depthwise_conv_preload_fp']['connections']
    for inst_name, inst_data in instances_dict.items():
        if "kernel_host_global_wrapper_stencil$ub_kernel_host_global_wrapper_stencil" in inst_name and "clk_en_const" not in inst_name:
            # operate on each pond
            source_node_type = None
            source_node_name = None
            source_node_port = None

            # find packed PEs
            for conn in connections_list:
                if f"{inst_name}.data_out_pond_0" in conn[1]:
                    pe_name = conn[0].split('.')[0]
                    pond_to_pe_port = conn[0].split('.')[-1]
            assert pe_name, f"PE name not found for {inst_name}"
            
            # trace back to find the source node and count num regs
            assert pond_to_pe_port == "data1", f"Invalid pond_to_pe_port: {pond_to_pe_port}"
            input_to_pe_port = "data0"

            pe_node = f"{pe_name}.{input_to_pe_port}"
            reg_count = 0

            for conn in connections_list:
                if pe_node in conn[0]:
                    prev_node = conn[1]
                    prev_node_name = prev_node.split('.')[0]
                    prev_node_port = prev_node.split('.')[-1]
                    break
            
            assert prev_node_name, f"Source node not found for {pe_node}, it might be in conn[1]"
            if "reg" not in prev_node_name:
                # source node found
                if "input_host_global_wrapper_global_wrapper_stencil$" in prev_node_name:
                    source_node_type = "mem"
                    source_node_name = prev_node_name
                    source_node_port = prev_node_port
                    assert source_node_port == "data_out_1" or source_node_port == "data_out_0", f"Invalid mem node port: {source_node_port}"
                    if source_node_port == "data_out_1":
                        mem_cycle_starting_addr = instances_dict[source_node_name]["metadata"]["config"]["tb2out_1"]["cycle_starting_addr"][0]
                        pond_updated_cycle_starting_addr = mem_cycle_starting_addr + reg_count + flush_latency_dict[inst_name] - flush_latency_dict[source_node_name]
                    else:
                        mem_cycle_starting_addr = instances_dict[source_node_name]["metadata"]["config"]["tb2out_0"]["cycle_starting_addr"][0]
                        pond_updated_cycle_starting_addr = mem_cycle_starting_addr + reg_count + flush_latency_dict[inst_name] - flush_latency_dict[source_node_name]
                elif "io16in_input_host_stencil" in prev_node_name:
                    source_node_type = "io"
                    print(f"[WARNING] Input from IO, adding 3 to cycle starting address, and 3 is hardcoded for input pipelining needing double check")
                    pond_updated_cycle_starting_addr = int(n_ic)//int(unroll)*int(ksize)*int(ksize) + reg_count + flush_latency_dict[inst_name] + 3
                else:
                    raise ValueError(f"Invalid source node name: {prev_node_name}")
                    
            else:
                reg_count += 1
                assert prev_node_port == "out", f"Invalid reg port: {prev_node_port}"
                prev_node = f"{prev_node_name}.in"
                while True:
                    found_prev_node = False
                    for conn in connections_list:
                        if prev_node in conn[0]:
                            found_prev_node = True
                            prev_node = conn[1]
                            prev_node_name = prev_node.split('.')[0]
                            prev_node_port = prev_node.split('.')[-1]
                            if "reg" not in prev_node_name:
                                # source node found
                                if "input_host_global_wrapper_global_wrapper_stencil$" in prev_node_name:
                                    source_node_type = "mem"
                                    source_node_name = prev_node_name
                                    source_node_port = prev_node_port
                                    assert source_node_port == "data_out_1" or source_node_port == "data_out_0", f"Invalid mem node port: {source_node_port}"
                                    if source_node_port == "data_out_1":
                                        mem_cycle_starting_addr = instances_dict[source_node_name]["metadata"]["config"]["tb2out_1"]["cycle_starting_addr"][0]
                                        pond_updated_cycle_starting_addr = mem_cycle_starting_addr + reg_count + flush_latency_dict[inst_name] - flush_latency_dict[source_node_name]
                                    else:
                                        mem_cycle_starting_addr = instances_dict[source_node_name]["metadata"]["config"]["tb2out_0"]["cycle_starting_addr"][0]
                                        pond_updated_cycle_starting_addr = mem_cycle_starting_addr + reg_count + flush_latency_dict[inst_name] - flush_latency_dict[source_node_name]
                                elif "io16in_input_host_stencil" in prev_node_name:
                                    source_node_type = "io"
                                    print(f"[WARNING] Input from IO, adding 3 to cycle starting address, and 3 is hardcoded for input pipelining needing double check")
                                    pond_updated_cycle_starting_addr = int(n_ic)//int(unroll)*int(ksize)*int(ksize) + reg_count + flush_latency_dict[inst_name] + 3
                                else:
                                    raise ValueError(f"Invalid source node name: {prev_node_name}")
                                break
                            else:
                                reg_count += 1
                                prev_node = f"{prev_node_name}.in"
                                break
                        elif prev_node in conn[1]:
                            found_prev_node = True
                            prev_node = conn[0]
                            prev_node_name = prev_node.split('.')[0]
                            prev_node_port = prev_node.split('.')[-1]
                            if "reg" not in prev_node_name:
                                # source node found
                                if "input_host_global_wrapper_global_wrapper_stencil$" in prev_node_name:
                                    source_node_type = "mem"
                                    source_node_name = prev_node_name
                                    source_node_port = prev_node_port
                                    assert source_node_port == "data_out_1" or source_node_port == "data_out_0", f"Invalid mem node port: {source_node_port}"
                                    if source_node_port == "data_out_1":
                                        mem_cycle_starting_addr = instances_dict[source_node_name]["metadata"]["config"]["tb2out_1"]["cycle_starting_addr"][0]
                                        pond_updated_cycle_starting_addr = mem_cycle_starting_addr + reg_count + flush_latency_dict[inst_name] - flush_latency_dict[source_node_name]
                                    else:
                                        mem_cycle_starting_addr = instances_dict[source_node_name]["metadata"]["config"]["tb2out_0"]["cycle_starting_addr"][0]
                                        pond_updated_cycle_starting_addr = mem_cycle_starting_addr + reg_count + flush_latency_dict[inst_name] - flush_latency_dict[source_node_name]
                                elif "io16in_input_host_stencil" in prev_node_name:
                                    source_node_type = "io"
                                    print(f"[WARNING] Input from IO, adding 3 to cycle starting address, and 3 is hardcoded for input pipelining needing double check")
                                    pond_updated_cycle_starting_addr = int(n_ic)//int(unroll)*int(ksize)*int(ksize) + reg_count + flush_latency_dict[inst_name] + 3
                                else:
                                    raise ValueError(f"Invalid source node name: {prev_node_name}")
                                break
                            else:
                                reg_count += 1
                                prev_node = f"{prev_node_name}.in"
                                break
                            
                    assert found_prev_node, f"Source node not found for {prev_node}"
                    if source_node_type == "mem" or source_node_type == "io":
                        break

            assert pond_updated_cycle_starting_addr, f"Pond updated cycle starting address not found for {inst_name}"

            # Update the updated cycle starting address to metadata for this pond
            instances_dict[inst_name]["metadata"]["config"]["regfile2out_0"]["cycle_starting_addr"][0] = pond_updated_cycle_starting_addr
            print(f"[INFO] Setting {inst_name}['metadata']['config']['regfile2out_0']['cycle_starting_addr'] to {pond_updated_cycle_starting_addr}")

def main():

    parser = argparse.ArgumentParser(description='Generate json for depthwise conv with kernel streaming')
    parser.add_argument('--preload', type=str, help='Preload depthwise conv app name', required=True)
    parser.add_argument('--stream', type=str, help='Stream depthwise conv app name', required=True)
    parser.add_argument('--clean', action='store_true', help='Clean the build directories')
    args = parser.parse_args()

    # first define halide_gen_args in application_parameter.json and get shapes
    halide_path = '/aha/Halide-to-Hardware/apps/hardware_benchmarks/apps/'
    preload_app = args.preload
    stream_app = args.stream
    args_dict = get_shape_args(preload_app)
    in_img = args_dict.get('in_img')
    ksize = args_dict.get('ksize')
    stride = args_dict.get('stride')
    n_ic = args_dict.get('n_ic')
    unroll = args_dict.get('unroll')

    # run preload weight and stream weight dw conv
    if os.path.exists(f'{halide_path}/{preload_app}/bin') and not args.clean:
        print("preload bin already exists")
    else:
        run_app(preload_app)

    if os.path.exists(f'{halide_path}/{stream_app}/bin') and not args.clean:
        print("stream bin already exists")
    else:
        run_app(stream_app, run_test=False)

    # load preload json and archive original preload json
    if not os.path.exists(f'{halide_path}/{preload_app}/design_top_final.json'):
        with open(f'{halide_path}/{preload_app}/bin/design_top.json','r') as file:
            design_top_preload = json.load(file)
        design_top_preload_original = design_top_preload
        with open(f'{halide_path}/{preload_app}/bin/design_top_preload_original.json', 'w') as file:
            print(f"Archiving {halide_path}/{preload_app}/bin/design_top_preload_original.json")
            json.dump(design_top_preload_original, file, indent=2)
        
        # load stream json
        with open(f'{halide_path}/{stream_app}/bin/design_top.json','r') as file:
            design_top_stream = json.load(file)

        # add kernel streaming to compute graph and add pond schedules
        replace_const_instr(preload_d=design_top_preload, stream_d=design_top_stream)
        add_kernel_read_types_and_conns(design_top_preload)
        add_kernel_global_wrapper_read_write_conn(design_top_preload)
        add_dw_conv_kernel_read_type(design_top_preload, unroll)
        add_pond_schedules_and_conns(preload_d = design_top_preload, stream_d=design_top_stream, in_img = int(in_img), ksize = int(ksize), stride = int(stride), n_ic = int(n_ic), unroll = int(unroll))
        edit_hcompute_depthwise_conv_stencil(design_top_preload)
        copy_hcompute_kernel_host_global_wrapper_stencil(design_top_preload)
        create_kernel_host_global_wrapper_global_wrapper_stencil_ub(preload_d = design_top_preload, stream_d=design_top_stream)
        create_kernel_host_global_wrapper_stencil_ub(design_top_preload)
        create_op_hcompute_kernel_host_global_wrapper_stencil(design_top_preload)

        # write updated design_top.json to file
        print(f"Writing {halide_path}/{preload_app}/design_top_final.json")
        with open(f'{halide_path}/{preload_app}/design_top_final.json', 'w') as file:
            json.dump(design_top_preload, file, indent=2)
    else:
        print(f"{halide_path}/{preload_app}/design_top_final.json already exists")

    # add kernel input to design_meta_halide.json
    add_kernel_to_design_meta_halide(f'{halide_path}/{preload_app}/bin/design_meta_halide.json', int(n_ic), int(ksize))

    # copy design_top_final.json to bin and run pnr with new netlist
    shutil.copy(f'{halide_path}/{preload_app}/design_top_final.json', f'{halide_path}/{preload_app}/bin/design_top.json')
    if os.path.exists(f'{halide_path}/{stream_app}/bin/glb_bank_config.json') and os.path.exists(f'{halide_path}/{preload_app}/bin/glb_bank_config.json'):
        with open(f'{halide_path}/{stream_app}/bin/glb_bank_config.json', 'r') as file:
            stream_bank_config = json.load(file)
        with open(f'{halide_path}/{preload_app}/bin/glb_bank_config.json', 'r') as file:
            preload_bank_config = json.load(file)
        print("Adding kernel_host_stencil to preload glb_bank_config.json")
        # update kernel_host_stencil from stream to preload
        preload_bank_config["inputs"]["kernel_host_stencil"] = stream_bank_config["inputs"]["kernel_host_stencil"]
        with open(f'{halide_path}/{preload_app}/bin/glb_bank_config.json', 'w') as file:
                json.dump(preload_bank_config, file, indent=2)
    try:
        run_pnr(preload_app, generate_bitstream_only=False)

    except:
        print("First PNR failed excepted.")
    
    # update pond schedule according to pond flush latency
    # load flush latency json
    assert os.path.exists(f'{halide_path}/{preload_app}/bin/depthwise_conv_preload_fp_flush_latencies.json')
    with open(f'{halide_path}/{preload_app}/bin/depthwise_conv_preload_fp_flush_latencies.json','r') as file:
        flush_latency_dict = json.load(file)
        assert isinstance(flush_latency_dict, type({})) and flush_latency_dict, "Flush latency data is empty"
    
    # load design_top_final.json and depthwise_conv_preload_fp instances dict
    assert os.path.exists(f'{halide_path}/{preload_app}/design_top_final.json')
    with open(f'{halide_path}/{preload_app}/design_top_final.json','r') as file:
        design_top_preload = json.load(file)
    update_pond_flush_latency(design_top_preload['namespaces']['global']['modules']['depthwise_conv_preload_fp']['instances'], flush_latency_dict)

    # extra fix when channel is not fully unrolled
    if int(n_ic) > int(unroll):
        # delay inputs to wait for kernel streaming if n_ic > unroll
        delay_inputs_to_wait_for_kernel(design_top_preload, int(n_ic)//int(unroll)*int(ksize)*int(ksize))
        fix_pond_output_extent(design_top_preload, int(in_img), int(n_ic), int(unroll))
        align_kernel_with_input(design_top_preload, flush_latency_dict, int(n_ic), int(unroll), int(ksize))

    # Write final design_top.json
    print(f"Writing {halide_path}/{preload_app}/design_top_final.json")
    with open(f'{halide_path}/{preload_app}/design_top_final.json', 'w') as file:
        json.dump(design_top_preload, file, indent=2)

    # run bitstream generation
    shutil.copy(f'{halide_path}/{preload_app}/design_top_final.json', f'{halide_path}/{preload_app}/bin/design_top.json')
    run_pnr(preload_app, generate_bitstream_only=True)

    # copy all raw files from stream
    stream_raw_files = glob.glob(f'{halide_path}/{stream_app}/bin/*.raw')
    for file in stream_raw_files:
        print(f"Copying {file} to {halide_path}/{preload_app}/bin")
        shutil.copy(file, f'{halide_path}/{preload_app}/bin')
    
if __name__ == '__main__':
    main()
import argparse
import json
import os
import pprint
import sys
import re
import copy
import traceback
from voyager.scripts.aha_flow.glb_dma_config import get_glb_dma_config

def atoi(text):
    return int(text) if text.isdigit() else text

def natural_keys(text):
    return [ atoi(c) for c in re.split(r'(\d+)', text) ]

# python ../../hw_support/parse_design_meta.py bin/design_meta_halide.json --top bin/design_top.json --place bin/design.place
def parseArguments():
    # Create argument parser
    parser = argparse.ArgumentParser()

    # Positional mandatory arguments
    parser.add_argument("DesignMeta", help="design_meta.json: file so far", type=str)

    # Optional arguments
    parser.add_argument("--top", help="design_top.json: parse out address sequence", type=str, default=None)
    parser.add_argument("--place", help="design.place: parse IO placement", type=str, default=None)
    parser.add_argument("--shuffle", help="update design_meta.json file to use shuffled data", action='store_true')
    parser.add_argument("--mu_tiling", default="", type=str, help="specify the output tiling used by the matrix unit. Need thiss to modify design_meta with the address gen config, based on the tiling information.")
    # Parse arguments
    args = parser.parse_args()

    return args

def findBetween(s:str, start:str, end:str):
    end = end if "clkwrk" in s else "_op_hcompute"
    assert (start in s) and (end in s)
    starti = s.find(start) + len(start)
    endi = s.find(end, starti)
    return s[starti:endi]

# this does not require an exact match
def findIO(metaBase, name:str):
    for metaIO in metaBase:
        if name in metaIO["name"]:
            return metaIO
    print(f"Could not find {name} in list of IOs")
    assert False

def setOrCheck(json, key, value):
    if key in json:
        pass
        # assert value == json[key]
    else:
        json[key] = value

def parseDesignTop(meta, filename: str):
    meta["testing"]["coreir"] = os.path.basename(filename)

    metaIn = []
    metaMU_in = []
    metaOut = []
    with open(filename, "r") as readFile:
        designTop = json.load(readFile)
        topName = designTop["top"]
        prefix, success, designName = topName.partition("global.")
        assert prefix == "" and success
        print("parsing design top:", designName)

        coreirInstances = designTop["namespaces"]["global"]["modules"][designName]["instances"]

        dense_ready_valid = "DENSE_READY_VALID" in os.environ and os.environ.get("DENSE_READY_VALID") == "1"
        for inst in coreirInstances:
            if inst.startswith("MU"):
                # this is a MU input
                ioName = findBetween(inst, "MU_io16in_", "_clkwrk")
                metaMU_in = findIO(meta["IOs"]["mu_inputs"], ioName)


                if "mu_io_tiles" in metaMU_in:
                    metaMU_in["mu_io_tiles"].append({"name":inst})
                else:
                    metaMU_in["mu_io_tiles"] = [{"name":inst}]

                # # change read_data_stride based on the number of input tiles
                # metaIn["io_tiles"]

            elif inst.startswith("io16in_"):
                # this is a data input
                ioName = findBetween(inst, "io16in_", "_clkwrk")
                addr = coreirInstances[inst]["metadata"]["glb2out_0"]
                metaIn = findIO(meta["IOs"]["inputs"], ioName)

                mode_string = "RV" if dense_ready_valid else "STATIC"

                if "io_tiles" in metaIn:
                    metaIn["io_tiles"].append({"name":inst, "addr":addr, "mode":mode_string})
                else:
                    metaIn["io_tiles"] = [{"name":inst, "addr":addr, "mode":mode_string}]

                # change read_data_stride based on the number of input tiles
                metaIn["io_tiles"]


            elif inst.startswith("io16_"):
                # this is a data output
                ioName = findBetween(inst, "io16_", "_clkwrk")
                addr = coreirInstances[inst]["metadata"]["in2glb_0"]
                metaOut = findIO(meta["IOs"]["outputs"], ioName)

                mode_string = "RV" if dense_ready_valid else "VALID"

                if "io_tiles" in metaOut:
                    #metaOut["io_tiles"].append({"name":inst, "addr":addr, "mode":"VALID"})
                    metaOut["io_tiles"].append({"name":inst, "addr":addr, "mode":mode_string})
                else:
                    #metaOut["io_tiles"] = [{"name":inst, "addr":addr, "mode":"VALID"}]
                    metaOut["io_tiles"] = [{"name":inst, "addr":addr, "mode":mode_string}]

        # alter the shape and data stride
        for input_struct in meta["IOs"]["inputs"]:
            if "io_tiles" in input_struct:
                num_tiles = len(input_struct["io_tiles"])
                # change the shape
                if num_tiles > 1 and input_struct["shape"][0] is not num_tiles:
                    new_shape = input_struct["shape"]
                    partial_unroll = num_tiles
                    # remove dimensions that are fully unrolled
                    while partial_unroll >= new_shape[0]:
                        assert(partial_unroll % new_shape[0] == 0), f"input shape has inner dim {new_shape[0]} and trying to unroll {partial_unroll}"
                        partial_unroll //= new_shape[0]
                        new_shape.pop(0)
                    assert(new_shape[0] % partial_unroll == 0), f"input shape has inner dim {new_shape[0]} and trying to unroll {num_tiles}"
                    new_shape.insert(0,num_tiles)
                    new_shape[1] //= partial_unroll
                    input_struct["shape"] = new_shape

        for output_struct in meta["IOs"]["outputs"]:
            num_tiles = len(output_struct["io_tiles"])
            # change the shape
            if num_tiles > 1 and output_struct["shape"][0] is not num_tiles:
                new_shape = output_struct["shape"]
                partial_unroll = num_tiles
                # remove dimensions that are fully unrolled
                while partial_unroll >= new_shape[0]:
                    assert(partial_unroll % new_shape[0] == 0), f"output shape has inner dim {new_shape[0]} and trying to unroll {partial_unroll}"
                    partial_unroll //= new_shape[0]
                    new_shape.pop(0)
                assert(new_shape[0] % partial_unroll == 0), f"output shape has inner dim {new_shape[0]} and trying to unroll {partial_unroll}"
                new_shape.insert(0,num_tiles)
                new_shape[1] //= partial_unroll
                output_struct["shape"] = new_shape

        # sort the inputs and outputs based on human sorting
        if "mu_io_tiles" in metaMU_in:
            metaMU_in["mu_io_tiles"].sort(key=lambda x: natural_keys(x["name"]))

        if "io_tiles" in metaIn:
            metaIn["io_tiles"].sort(key=lambda x: natural_keys(x["name"]))
        metaOut["io_tiles"].sort(key=lambda x: natural_keys(x["name"]))


def parseDesignPlace(meta, filename: str):
    print("parsing design place", filename)
    meta["testing"]["placement"] = os.path.basename(filename)

    with open(filename, "r") as readFile:
        lines = readFile.readlines()
        for line in lines:
            words = line.split()
            name = words[0]
            if ("\t#I" in line) and name.startswith("io16_"):
                assert len(words) == 4
                ioName = findBetween(name, "io16_", "_clkwrk")
                metaOut = findIO(meta["IOs"]["outputs"], ioName)
                tileOut = findIO(metaOut["io_tiles"], name)
                setOrCheck(tileOut, "x_pos", int(words[1]))
                setOrCheck(tileOut, "y_pos", int(words[2]))

            elif ("\t#I" in line) and name.startswith("io16in_"):
                assert len(words) == 4
                ioName = findBetween(name, "io16in_", "_clkwrk")
                metaOut = findIO(meta["IOs"]["inputs"], ioName)
                tileOut = findIO(metaOut["io_tiles"], name)
                setOrCheck(tileOut, "x_pos", int(words[1]))
                setOrCheck(tileOut, "y_pos", int(words[2]))

            elif ("\t#i" in line) and name.startswith("io1_"):
                ioName = findBetween(name, "io1_", "_clkwrk")
                validName = findBetween(name, "io1_", "_write_valid")
                metaOut = findIO(meta["IOs"]["outputs"], ioName)
                tileOut = findIO(metaOut["io_tiles"], validName)
                setOrCheck(tileOut, "x_pos", int(words[1]))
                setOrCheck(tileOut, "y_pos", int(words[2]))
                tileOut["valid_name"] = validName


            elif ("\t#U" in line or "\t#V" in line) and name.startswith("MU_io16in_"):
                assert len(words) == 4
                ioName = findBetween(name, "MU_io16in_", "_clkwrk")
                metaOut = findIO(meta["IOs"]["mu_inputs"], ioName)
                tileOut = findIO(metaOut["mu_io_tiles"], name)
                setOrCheck(tileOut, "x_pos", int(words[1]))
                setOrCheck(tileOut, "y_pos", int(words[2]))

# Hacky functions to tile xy loop when flattened and add padding to extents
def parseLoopExtentforPadding(meta, halide_gen_args):
    # Get pad_o values
    args = halide_gen_args.split()
    args_dict = {key: int(value) for key, value in (item.split('=') for item in halide_gen_args.split())}
    pad_o_left = args_dict.get('pad_o_left', 0)
    pad_o_right = args_dict.get('pad_o_right', 0)

    # Get X_dim
    out_shape_list = meta['IOs']['outputs'][0]['shape']
    assert out_shape_list[-1] == out_shape_list[-2], "Only square output supported for output padding"
    X_dim = out_shape_list[-1]

    # Change extent for all io tiles
    assert len(meta['IOs']['outputs']) == 1, "Only one output supported for output padding"
    io_tiles_list = meta['IOs']['outputs'][0]['io_tiles']
    for io_tile in io_tiles_list:
        addr_dict = io_tile['addr']
        found_X_cnt = 0
        # add dimension if X_dim or Y_dim are flattened
        for i, ext in enumerate(addr_dict['extent']):
            if ext == X_dim:
                found_X_cnt += 1
                addr_dict['extent'][i] += (pad_o_left + pad_o_right)
            elif ext == X_dim * X_dim:
                found_X_cnt += 2
                addr_dict['dimensionality'] += 1
                addr_dict['extent'][i] = X_dim + (pad_o_left + pad_o_right)
                addr_dict['extent'].insert(i+1, X_dim + (pad_o_left + pad_o_right))
                addr_dict['cycle_stride'].insert(i+1, addr_dict['cycle_stride'][i] * X_dim)
                addr_dict['write_data_stride'].insert(i+1, addr_dict['write_data_stride'][i] * X_dim)
                break
            elif ext % X_dim == 0:
                found_X_cnt += 1
                addr_dict['dimensionality'] += 1
                addr_dict['extent'][i] = X_dim + (pad_o_left + pad_o_right)
                addr_dict['extent'].insert(i+1, ext // X_dim)
                addr_dict['cycle_stride'].insert(i+1, addr_dict['cycle_stride'][i] * X_dim)
                addr_dict['write_data_stride'].insert(i+1, addr_dict['write_data_stride'][i] * X_dim)
        assert found_X_cnt == 2, "X_dim and Y_dim not found in addr_dict['extent']"

    # Add HALIDE_GEN_ARGS to meta file
    if meta.get("HALIDE_GEN_ARGS") is None: meta["HALIDE_GEN_ARGS"] = args_dict

def addGLBBankConfig(meta):
    with open("bin/glb_bank_config.json", "r") as f:
        glb_json = json.load(f)
    meta["GLB_BANK_CONFIG"] = glb_json

    # Build {x_pos: E64_packed} map
    input_pack_map = {}
    for blk in glb_json.get("inputs", []):
        for _, cfg in blk.items():
            for flag, x in zip(cfg.get("E64_packed", []), cfg.get("x_coord", [])):
                input_pack_map[x] = flag
    output_pack_map = {}
    for blk in glb_json.get("outputs", []):
        for _, cfg in blk.items():
            for flag, x in zip(cfg.get("E64_packed", []), cfg.get("x_coord", [])):
                output_pack_map[x] = flag

    # Build {x_pos: use_multi_bank_mode} maps from "use_multi_bank_mode"
    input_use_multi_bank_map = {}
    for blk in glb_json.get("inputs", []):
        for _, cfg in blk.items():
            for flag, x in zip(cfg.get("use_multi_bank_mode", []), cfg.get("x_coord", [])):
                input_use_multi_bank_map[x] = flag

    output_use_multi_bank_map = {}
    for blk in glb_json.get("outputs", []):
        for _, cfg in blk.items():
            for flag, x in zip(cfg.get("use_multi_bank_mode", []), cfg.get("x_coord", [])):
                output_use_multi_bank_map[x] = flag

    # Build {x_pos: bank_toggle_mode} map
    bank_toggle_mode_map = {}
    for blk in glb_json.get("outputs", []):
        for _, cfg in blk.items():
            for flag, x in zip(cfg.get("bank_toggle_mode", []), cfg.get("x_coord", [])):
                bank_toggle_mode_map[x] = flag

    for input_index, input_dict in enumerate(meta["IOs"]["inputs"]):
        assert "io_tiles" in input_dict, "io_tiles must be key of input_dict"
        for tile_index, io_tile in enumerate(input_dict["io_tiles"]):
            # Check if 'x_pos' of this io_tile is in 'glb_inputs' of the GLB_BANK_CONFIG
            if io_tile["x_pos"] in meta["GLB_BANK_CONFIG"].get("glb_inputs", []):
                meta["IOs"]["inputs"][input_index]["io_tiles"][tile_index]["is_glb_input"] = 1
            else:
                meta["IOs"]["inputs"][input_index]["io_tiles"][tile_index]["is_glb_input"] = 0

            # Add E64_packed flag for input tiles
            meta["IOs"]["inputs"][input_index]["io_tiles"][tile_index]["E64_packed"] = input_pack_map.get(io_tile["x_pos"], 1)

            # Add use_multi_bank_mode for input tiles
            meta["IOs"]["inputs"][input_index]["io_tiles"][tile_index]["use_multi_bank_mode"] = input_use_multi_bank_map.get(io_tile["x_pos"], 1)

    # Add E64_packed, bank_toggle_mode, and is_fake_io flags for output tiles
    for output_index, output_dict in enumerate(meta["IOs"]["outputs"]):
        assert "io_tiles" in output_dict, "io_tiles must be key of output_dict"
        for tile_index, io_tile in enumerate(output_dict["io_tiles"]):
            meta["IOs"]["outputs"][output_index]["io_tiles"][tile_index]["E64_packed"] = output_pack_map.get(io_tile["x_pos"], 1)
            meta["IOs"]["outputs"][output_index]["io_tiles"][tile_index]["bank_toggle_mode"] = bank_toggle_mode_map.get(io_tile["x_pos"], 0)
            meta["IOs"]["outputs"][output_index]["io_tiles"][tile_index]["use_multi_bank_mode"] = output_use_multi_bank_map.get(io_tile["x_pos"], 1)

    # Hack bank toggle config only if bank_toggle_mode_map is not empty
    if bank_toggle_mode_map:
        meta = hack_config_for_bank_toggle_mode(meta)

    return meta

# Function to change extent for glb tiling or add dimensions
def parseLoopExtentforTiling(meta, halide_gen_args):
    # Get unroll values
    args = halide_gen_args.split()
    args_dict = {key: int(value) for key, value in (item.split('=') for item in halide_gen_args.split())}
    unroll = args_dict.get('unroll', 0)
    n_ic = args_dict.get('n_ic', 0)
    assert len(meta['IOs']['outputs']) == 1, "Only one output supported for output padding"
    io_tiles_list = meta['IOs']['outputs'][0]['io_tiles']
    for io_tile in io_tiles_list:
        addr_dict = io_tile['addr']
        assert addr_dict['dimensionality'] == 2, "Implement fully unrolled first"
    # Add HALIDE_GEN_ARGS to meta file
    if meta.get("HALIDE_GEN_ARGS") is None: meta["HALIDE_GEN_ARGS"] = args_dict
    if meta.get("NUM_GLB_TILING") is None: meta["NUM_GLB_TILING"] = os.getenv("NUM_GLB_TILING")

def E64_packing(json_data):
    print(f"\033[94m INFO: Modifying design meta for E64 packing...\033[0m")

    # Collect x-positions that need E64 packing from GLB_BANK_CONFIG
    input_pack_x, output_pack_x = set(), set()
    for section, pack_set in [("inputs", input_pack_x), ("outputs", output_pack_x)]:
        for blk in json_data.get("GLB_BANK_CONFIG", {}).get(section, []):
            for _, cfg in blk.items():
                # If GLB_BANK_CONFIG is set but E64_packed is missing, treat all x_coords as packed
                if "E64_packed" not in cfg:
                    pack_set.update(cfg.get("x_coord", []))
                else:
                    for flag, x in zip(cfg.get("E64_packed", []), cfg.get("x_coord", [])):
                        if flag == 1:
                            pack_set.add(x)

    # If GLB_BANK_CONFIG is empty, treat all io_tiles as packed
    if not json_data.get("GLB_BANK_CONFIG"):
        for entry in json_data["IOs"]["inputs"]:
            input_pack_x.update(t["x_pos"] for t in entry["io_tiles"])
        for entry in json_data["IOs"]["outputs"]:
            output_pack_x.update(t["x_pos"] for t in entry["io_tiles"])

    unique_input_positions = set()
    unique_output_positions = set()
    trimmed_inputs = []
    trimmed_outputs = []

    # Assert that the number of inputs and outputs are multiples of 4
    # FIXME: Potentially fix this for apps with two loop levels
    def process_io(io_entries, unique_positions, pack_x_set):
        trimmed_entries = []
        for entry in io_entries:
            # Check per-entry if packing is actually used (any tile at a packed x)
            entry_uses_packing = any(tile["x_pos"] in pack_x_set for tile in entry["io_tiles"])

            # Enforce assertion only if packing is used
            if entry_uses_packing and (len(entry["io_tiles"]) % 4 != 0):
                raise AssertionError("Number of io_tiles must be a multiple of 4 for E64 mode")

            # If this entry doesn't use packing at all, leave it untouched
            if not entry_uses_packing:
                trimmed_entries.append(entry)
                continue

            new_io_tiles = []
            shape_updated = False
            for tile in entry["io_tiles"]:
                position = (tile["x_pos"], tile["y_pos"])
                if position not in unique_positions:
                    unique_positions.add(position)

                    # Apply extent scaling only for packed x-positions
                    if tile["x_pos"] in pack_x_set:
                        if "extent_multiplier" in tile:
                            tile["extent_multiplier"] *= 4
                        else:
                            tile["extent_multiplier"] = 4
                        shape_updated = True

                    new_io_tiles.append(tile)

            if new_io_tiles:
                entry["io_tiles"] = new_io_tiles

                # Adjust shape only when packing happened
                if shape_updated:
                    if "shape" in entry and len(entry["shape"]) >= 2:
                        entry["shape"][0] = int(entry["shape"][0] / 4)
                        entry["shape"][1] = entry["shape"][1] * 4
                        if entry["shape"][0] == 1:
                            entry["shape"] = [entry["shape"][1]] + entry["shape"][2:]
                    else:
                        print("ERROR: shape not found or incorrectly formatted. Confirm that deisgn_top.json and design.place are correct for E64 mode. (Hint: Is unroll a multiple of 4?)")
                        sys.exit(1)

                trimmed_entries.append(entry)
        return trimmed_entries

    json_data["IOs"]["inputs"] = process_io(json_data["IOs"]["inputs"], unique_input_positions, input_pack_x)
    json_data["IOs"]["outputs"] = process_io(json_data["IOs"]["outputs"], unique_output_positions, output_pack_x)

    return json_data


def zircon_fx_fy_stride_workaround_hack_cycle_stride_k_y_x_tiling(extents):
    """
    This is a hack to modify the cycle stride and starting address for the
    kernel/stride hack used to work around the MU bug on downsampling layers.
    This logic assumes K1, Y0, X0 tiling on the MU, which is the case for the
    downsampling layers in the ResNet18 thus far.
    """
    print(f"\033[93m INFO: Applying kernel and stride workaround to avoid Zircon MU bug...this workaround should only be used on downsample layers and MU tiling must be in order K1, Y0, X0.\033[0m")

    assert len(extents) == 3, "Tiling must have EXACTLY 3 dimensions (K1, Y0, X0) for this workaround. Alternate tiling will need a different modification to cycle starting addr and stride."

    cycle_starting_addr = [extents[0] * 2 + 1] # X0 * 2 + 1. Skip the entire first row of padding, plus on to account for first column of padding on row 1

    cycle_strides = [1] * 3
    cycle_strides[0] = 2 # X0: Skip every other pixel in the X0 dimension
    cycle_strides[1] = extents[0] * 2 + 2  # Y0: Skip every other row in the Y0 dimension, plus the first column of padding. + 1 b/c cycle stride 1 = no skip
    cycle_strides[2] = cycle_starting_addr[0] + 1 # K1: Whenever k is incremented, we want to skip the entire row of padding, plus the first column of padding on row 1

    return cycle_starting_addr, cycle_strides


def zircon_input_act_padding_workaround_hack_cycle_stride_k_y_x_tiling(extents, zircon_input_act_padding_workaround_size):
    """ This is a hack to modify the cycle stride for the
    input padding workaround used to work around the MU bug on conv5 layers.
    This logic assumes K1, Y0, X0 tiling on the MU, which is the case for the
    conv5 layers in the ResNet18 thus far.
    """
    print(f"\033[93m INFO: Applying input padding workaround to avoid Zircon MU bug...this workaroud should only be used if MU tiling is in order (K2 and/or K1) THEN Y0 THEN X0.\033[0m")

    assert (len(extents) == 3 or len(extents) == 4), "Tiling must have EXACTLY 3 dimensions (K2/K1, Y0, X0) or 4 dimensions (K2, K1, Y0, X0) for this workaround. Alternate tiling will need a different modification to cycle starting addr and/or stride."

    cycle_strides = [1] * len(extents) # X0
    # Y0: Skip "padding size" number of columns. + 1 b/c cycle stride 1 = no skip
    cycle_strides[1] = zircon_input_act_padding_workaround_size + 1
    # K2/K1: Whenever k is incremented, we want to skip the remaining columns in the current row, then all padding rows + 1 b/c cycle stride 1 = no skip
    cycle_strides[2] = zircon_input_act_padding_workaround_size + (zircon_input_act_padding_workaround_size * (extents[0] + zircon_input_act_padding_workaround_size)) + 1

    if len(extents) == 4:
        # K1/K1: Whenever k is incremented, we want to skip the remaining columns in the current row, then all padding rows + 1 b/c cycle stride 1 = no skip
        cycle_strides[3] = zircon_input_act_padding_workaround_size + (zircon_input_act_padding_workaround_size * (extents[0] + zircon_input_act_padding_workaround_size)) + 1

    return cycle_strides


def hack_addr_gen_for_mu_tiling(meta, mu_tiling_file):
    """
    Hack the address generator config based on the matrix unit tiling.
    This function reads the tiling file and modifies the address generator config in the design meta.
    """
    MU_WORD_NUM_BYTES = 32  # 32 bytes per word in the MU
    BANK_DATA_WIDTH = 64  # 64 bits per bank in GLB
    BANK_NUM_BYTES = BANK_DATA_WIDTH // 8  # 8 bytes per bank in GLB
    CGRA_WORD_NUM_BYTES = 2 # 2 bytes per word in CGRA
    # Read the tiling file
    if not os.path.isfile(mu_tiling_file):
        print(f"\033[91m ERROR: Tiling file {mu_tiling_file} does not exist. Cannot modify address generator config.\033[0m")
        sys.exit(1)

    zircon_fx_fy_stride_workaround = "ZIRCON_FX_FY_STRIDE_WORKAROUND" in os.environ and os.environ["ZIRCON_FX_FY_STRIDE_WORKAROUND"] == "1"
    zircon_input_act_padding_workaround = "ZIRCON_INPUT_ACT_PADDING_WORKAROUND" in os.environ and os.environ["ZIRCON_INPUT_ACT_PADDING_WORKAROUND"] == "1"
    if zircon_input_act_padding_workaround:
        assert "ZIRCON_INPUT_ACT_PADDING_WORKAROUND_SIZE" in os.environ, "ZIRCON_INPUT_ACT_PADDING_WORKAROUND_SIZE environment variable must be set for ZIRCON_INPUT_ACT_PADDING_WORKAROUND"
        zircon_input_act_padding_workaround_size = int(os.environ.get("ZIRCON_INPUT_ACT_PADDING_WORKAROUND_SIZE", 0))


    mha_permute = "MHA_PERMUTE" in os.environ and os.environ["MHA_PERMUTE"] == "1"
    num_attn_heads = int(os.environ.get("NUM_ATTENTION_HEADS", 12))
    # K dimension host tiling: used in resnet18 conv5 in Zircon
    # k_dim_host_tiling = "K_DIM_HOST_TILING" in os.environ and os.environ["K_DIM_HOST_TILING"] == "1"
    # if k_dim_host_tiling:
    #     assert "NUM_K_HOST_TILING_KERNELS" in os.environ, "NUM_K_HOST_TILING_KERNELS environment variable must be set for K_DIM_HOST_TILING"
    #     assert "K_DIM_HOST_TILING_IDX" in os.environ, "K_DIM_HOST_TILING_IDX environment variable must be set for K_DIM_HOST_TILING"
    #     num_k_host_tiling_kernels = int(os.environ.get("NUM_K_HOST_TILING_KERNELS"))
    #     k_dim_host_tiling_idx = int(os.environ.get("K_DIM_HOST_TILING_IDX"))


    # Update the address generator config in the design meta
    # TODO: This really shouldn't be applied to ALL inputs and outputs. In future, need some sort of metadata to specify which inputs and outputs are influenced by the matrix unit tiling
    for io_type in ["inputs", "outputs"]:
        apply_zircon_fx_fy_stride_workaround = zircon_fx_fy_stride_workaround and io_type == "outputs"
        apply_zircon_input_act_padding_workaround = zircon_input_act_padding_workaround and io_type == "outputs"

        for io in meta["IOs"][io_type]:
            io_name = io["name"]
            io_name_caps = io_name.upper()

            io_broadcast_dims = []
            if "BROADCAST_TENSORS" in os.environ:
                broadcast_tensors = os.environ["BROADCAST_TENSORS"].split(",")
                if io_name_caps in broadcast_tensors:
                    io_broadcast_dims_env_str = f"{io_name_caps}_BROADCAST_DIMS"
                    assert io_broadcast_dims_env_str in os.environ, f"{io_broadcast_dims_env_str} environment variable must be set because {io_name_caps} is in BROADCAST_TENSORS"
                    io_broadcast_dims = os.environ[io_broadcast_dims_env_str].split(",")


                    print(f"\033[94m INFO: Tensor {io_name_caps} is being broadcast over dims: {io_broadcast_dims}\033[0m")

            # Get the GLB DMA config
            dimensionality, strides, extents = get_glb_dma_config(mu_tiling_file, zircon_fx_fy_stride_workaround=apply_zircon_fx_fy_stride_workaround,
                                                                  zircon_input_act_padding_workaround=apply_zircon_input_act_padding_workaround, mha_permute=mha_permute, num_attn_heads=num_attn_heads, broadcast_dims=io_broadcast_dims)
            if "io_tiles" in io:
                for tile in io["io_tiles"]:
                    addr = tile.get("addr", {})
                    if addr:
                        # Update the strides, extents, and dimensionality based on MU tiling
                        if apply_zircon_fx_fy_stride_workaround:
                            cycle_starting_addr, cycle_stride = zircon_fx_fy_stride_workaround_hack_cycle_stride_k_y_x_tiling(extents)
                            addr["cycle_starting_addr"] = cycle_starting_addr
                            addr["cycle_stride"] = cycle_stride
                        elif apply_zircon_input_act_padding_workaround:
                            cycle_stride = zircon_input_act_padding_workaround_hack_cycle_stride_k_y_x_tiling(extents, zircon_input_act_padding_workaround_size)
                            addr["cycle_stride"] = cycle_stride
                        else:
                            addr["cycle_stride"] = [1] * dimensionality # reading/writing on every RV handshake, so cycle stride is all 1
                        addr["dimensionality"] = dimensionality
                        addr["extent"] = extents
                        if io_type == "inputs":
                            addr["read_data_stride"] = strides
                        elif io_type == "outputs":
                            addr["write_data_stride"] = strides
                    # Add a new key to indicate that this is modified for MU tiling
                    tile["hacked_for_mu_tiling"] = True

                    # # zircon k dim tiling
                    # if k_dim_host_tiling:
                    #     assert "HALIDE_GEN_ARGS" in os.environ, "HALIDE_GEN_ARGS environment variable must be set for K_DIM_HOST_TILING"
                    #     HALIDE_GEN_ARGS = os.environ["HALIDE_GEN_ARGS"]
                    #     n_oc_match = re.search(r'n_oc=(\d+)', HALIDE_GEN_ARGS)
                    #     assert n_oc_match, "No n_oc in HALIDE_GEN_ARGS!"
                    #     n_oc = int(n_oc_match.group(1))

                    #     if io_type == "outputs":
                    #         orig_start_addr = addr["write_data_starting_addr"]
                    #         addr["gold_check_starting_addr"] = orig_start_addr
                    #         addr["write_data_starting_addr"] = [orig_start_addr[0] + k_dim_host_tiling_idx * ((n_oc // num_k_host_tiling_kernels) // MU_WORD_NUM_BYTES) * (BANK_NUM_BYTES//CGRA_WORD_NUM_BYTES)]
                    #         if "extent_multiplier" in tile:
                    #             tile["extent_multiplier"] *= num_k_host_tiling_kernels
                    #         else:
                    #             tile["extent_multiplier"] = num_k_host_tiling_kernels
                    #     elif io_type == "inputs":
                    #         orig_start_addr = addr["read_data_starting_addr"]
                    #         addr["read_data_starting_addr"] = [orig_start_addr[0] + k_dim_host_tiling_idx * ((n_oc // num_k_host_tiling_kernels) // MU_WORD_NUM_BYTES) * (BANK_NUM_BYTES//CGRA_WORD_NUM_BYTES)]
                    #         addr["tb_write_starting_addr"] = orig_start_addr
    return meta


def hack_addr_gen_for_k_dim_host_tiling(meta):
    MU_WORD_NUM_BYTES = 32  # 32 bytes per word in the MU
    BANK_DATA_WIDTH = 64  # 64 bits per bank in GLB
    BANK_NUM_BYTES = BANK_DATA_WIDTH // 8  # 8 bytes per bank in GLB
    CGRA_WORD_NUM_BYTES = 2 # 2 bytes per word in CGRA
    NUM_BYTES_PER_IO_TILE = 4 # 4 bytes per IO tile (4-wide word)

    assert "HALIDE_GEN_ARGS" in os.environ, "HALIDE_GEN_ARGS environment variable must be set for K_DIM_HOST_TILING"
    HALIDE_GEN_ARGS = os.environ["HALIDE_GEN_ARGS"]
    n_oc_match = re.search(r'n_oc=(\d+)', HALIDE_GEN_ARGS)
    assert n_oc_match, "No n_oc in HALIDE_GEN_ARGS!"
    n_oc = int(n_oc_match.group(1))

    assert "NUM_K_HOST_TILING_KERNELS" in os.environ, "NUM_K_HOST_TILING_KERNELS environment variable must be set for K_DIM_HOST_TILING"
    assert "K_DIM_HOST_TILING_IDX" in os.environ, "K_DIM_HOST_TILING_IDX environment variable must be set for K_DIM_HOST_TILING"
    num_k_host_tiling_kernels = int(os.environ.get("NUM_K_HOST_TILING_KERNELS"))
    k_dim_host_tiling_idx = int(os.environ.get("K_DIM_HOST_TILING_IDX"))

    ios_without_k_dim_tiling_addr_offset_env = os.environ.get("NO_K_TILING_ADDR_OFFSET", "")
    ios_without_k_dim_tiling_addr_offset = ios_without_k_dim_tiling_addr_offset_env.split(" ") if ios_without_k_dim_tiling_addr_offset_env else []

    packed_outputs = "PACKED_OUTPUTS" in os.environ and os.environ["PACKED_OUTPUTS"] == "1"

    for io_type in ["inputs", "outputs"]:
         for io in meta["IOs"][io_type]:
            if "io_tiles" in io:
                io_name = io.get("name", "")
                if io_name in ios_without_k_dim_tiling_addr_offset:
                    print(f"\033[93m INFO: Skipping k-dim host tiling address offset for IO {io_name} as specified by NO_K_TILING_ADDR_OFFSET environment variable.\033[0m")
                    continue
                num_io_tiles = len(io["io_tiles"])
                io_bw_per_cycle = num_io_tiles * NUM_BYTES_PER_IO_TILE
                if packed_outputs:
                    io_bw_per_cycle *= 2
                for tile in io["io_tiles"]:
                    addr = tile.get("addr", {})
                    if io_type == "outputs":
                        orig_start_addr = addr["write_data_starting_addr"]
                        addr["gold_check_starting_addr"] = orig_start_addr
                        addr["write_data_starting_addr"] = [orig_start_addr[0] + k_dim_host_tiling_idx * ((n_oc // num_k_host_tiling_kernels) // io_bw_per_cycle) * (BANK_NUM_BYTES//CGRA_WORD_NUM_BYTES)]
                        if "extent_multiplier" in tile:
                            tile["extent_multiplier"] *= num_k_host_tiling_kernels
                        else:
                            tile["extent_multiplier"] = num_k_host_tiling_kernels
                    elif io_type == "inputs":
                        orig_start_addr = addr["read_data_starting_addr"]
                        addr["read_data_starting_addr"] = [orig_start_addr[0] + k_dim_host_tiling_idx * ((n_oc // num_k_host_tiling_kernels) // io_bw_per_cycle) * (BANK_NUM_BYTES//CGRA_WORD_NUM_BYTES)]
                        addr["tb_write_starting_addr"] = orig_start_addr

    return meta



def hack_config_for_bank_toggle_mode(meta):
    BANK_DATA_WIDTH = 64
    CGRA_DATA_WIDTH = 16

    # Duplicate output tile info for bank toggle mode as fake IO tiles, and attach real output tile number to outputs
    for output_index, output_dict in enumerate(meta["IOs"]["outputs"]):
        real_tile_cnt = len(output_dict["io_tiles"])
        assert output_dict["shape"][0] == real_tile_cnt, (
            f"outputs[{output_index}] shape[0] ({output_dict['shape'][0]}) does not match number of real io_tiles ({real_tile_cnt})"
        )

        extra_tiles = []
        for tile in output_dict["io_tiles"]:
            if tile.get("bank_toggle_mode", 0) == 1:
                dup_tile = copy.deepcopy(tile)
                if "name" in dup_tile:
                    dup_tile["name"] = f"{dup_tile['name']}_dup"
                if "x_pos" in dup_tile:
                    assert isinstance(dup_tile["x_pos"], int), "x_pos must be an integer"
                    dup_tile["x_pos"] = (dup_tile["x_pos"] + 1 if dup_tile["x_pos"] % 2 == 0 else dup_tile["x_pos"] - 1)
                dup_tile["is_fake_io"] = 1
                extra_tiles.append(dup_tile)
            tile["is_fake_io"] = 0
        if extra_tiles:
            output_dict["io_tiles"].extend(extra_tiles)
            # Make sure shape [0] still matches the number of io_tiles
            output_dict["shape"][0] *= 2
            output_dict["shape"][1] //= 2

        output_dict["io_tiles"].sort(key=lambda t: t["x_pos"])
        output_dict["num_real_tiles"] = real_tile_cnt
        assert real_tile_cnt % 2 == 0, "Number of real output IO tiles must be even for bank toggle mode"

    # Modify address gen for IO tiles in bank toggle mode
    for io in meta["IOs"]["outputs"]:
        if "io_tiles" in io:
            real_tile_idx = 0
            tile_n_start_addr = 0
            for tile in io["io_tiles"]:
                if tile["bank_toggle_mode"] == 1:
                    if tile["is_fake_io"] == 0:
                        real_tile_idx += 1
                    print(f"\033[94m INFO: Modifying address generator config for bank toggle mode for output tile {tile['name']}...\033[0m")
                    addr = tile.get("addr", {})
                    if addr:
                        word_width_per_tile = 2 * BANK_DATA_WIDTH // CGRA_DATA_WIDTH # which is 8 in zircon
                        cycle_stride_per_bank = addr["cycle_stride"][0] // io['num_real_tiles']
                        if real_tile_idx % 2 == 0:
                            # This means we are skipping pixels for interleaving across another GLB tile
                            # So we need to adjust starting addr for data stored in another GLB tile for bank toggle mode
                            addr["cycle_starting_addr"][0] = tile_n_start_addr + word_width_per_tile * cycle_stride_per_bank
                        else:
                            # Store the cycle starting addr of n-th real tile for n+1-th starting addr calculation
                            tile_n_start_addr = addr["cycle_starting_addr"][0]
                        # Update the strides, extents, and dimensionality for bank toggle mode
                        # Need additional two dimensions for tweaks across banks and tiles
                        addr["dimensionality"] += 2
                        new_cycle_stride = [
                            cycle_stride_per_bank,
                            cycle_stride_per_bank,
                            (word_width_per_tile + 1) * cycle_stride_per_bank,
                        ]
                        # Although we support 2D DMA config, we currently use MEM tiles to filter dummy data.
                        # This helps avoid a very strict division constraint on extent.
                        if addr["dimensionality"] > 3:
                            assert addr["dimensionality"] == 4, "Need to think about a cleaner way to hack cycle stride for 3D and more loop levels."
                            # new_cycle_stride.append(addr["extent"][0] // (2 * BANK_DATA_WIDTH // CGRA_DATA_WIDTH) * new_cycle_stride[2] + addr["cycle_stride"][0])
                            # Skip the same amount of data as the starting addr of tile N+1, which are bogus data stream length + GLB tile width offset
                            new_cycle_stride.append(tile_n_start_addr + word_width_per_tile * cycle_stride_per_bank + 1)

                        addr["cycle_stride"] = new_cycle_stride
                        assert addr["extent"][0] % word_width_per_tile == 0, "Extent must be divisible by GLB tile word width for bank toggle mode"
                        # 2 is the number of banks per GLB tile, take the ceiling of the division
                        new_extent = [word_width_per_tile // 2, 2, addr["extent"][0] // word_width_per_tile]
                        if addr["dimensionality"] > 3:
                            new_extent.extend(addr["extent"][1:])
                        addr["extent"] = new_extent
                        # We have to backtrack the address when interleaving across banks
                        addr["write_data_stride"] = [1, 1 - word_width_per_tile // 2, 1]
                        if addr["dimensionality"] > 3:
                            for _ in range(addr["dimensionality"] - 3):
                                addr["write_data_stride"].append(1)
    return meta


def main():
    args = parseArguments()

    with open(args.DesignMeta, "r") as designMeta:
        meta = json.load(designMeta)

        # Search for *.bs file in the bin directory
        #bin_directory = args.DesignMeta.replace("/design_meta_halide.json", "");
        slash_index = args.DesignMeta.rfind("/")
        bin_directory = args.DesignMeta[0:slash_index]
        for file in os.listdir(bin_directory):
            if file.endswith(".bs"):
                meta["testing"]["bitstream"] = file
                break

        if args.top != None:
            parseDesignTop(meta, args.top)

        if args.place != None:
            parseDesignPlace(meta, args.place)

    if args.shuffle:
        inputs = meta['IOs']['inputs']
        for _input in inputs:
            if _input['datafile'][0:5] == 'input':
                _input['datafile'] = 'input_host_stencil_shuffle.raw'
            elif _input['datafile'][0:6] == 'kernel':
                _input['datafile'] = 'kernel_shuffle.raw'
        outputs = meta['IOs']['outputs']
        for _output in outputs:
            if _output['datafile'][0:9] == 'hw_output':
                _output['datafile'] = 'hw_output_shuffle.raw'


    outputName = 'bin/design_meta.json'
    try:
        halide_gen_args = os.getenv("HALIDE_GEN_ARGS")
        if halide_gen_args is not None:
            # If pad_o in args call padding functions to modify extents
            if "pad_o_left" in halide_gen_args or "pad_o_right" in halide_gen_args: parseLoopExtentforPadding(meta, halide_gen_args)
            # If NUM_GLB_TILING is set then edit extent if necessary
            if os.getenv("NUM_GLB_TILING") is not None and atoi(os.getenv("NUM_GLB_TILING")) > 0: parseLoopExtentforTiling(meta, halide_gen_args)

        if os.path.isfile("bin/glb_bank_config.json"):
            print(f"\033[94m INFO: Modifying design meta for custom GLB bank config...\033[0m")
            meta = addGLBBankConfig(meta)

        # pprint.pprint(meta, fileout, indent=2, compact=True)
        # MO: E64 MODE
        exchange_64_mode = "E64_MODE_ON" in os.environ and os.environ.get("E64_MODE_ON") == "1"
        if exchange_64_mode:
            meta = E64_packing(meta)

        # Modify controller config to account for matrix unit tiling, if needed
        # TODO: Currently applying the addr gen config to all inputs and outputs. Need some sort of metadata to specify which inputs and outputs are influenced by the matrix unit tiling
        # TODO: When quantization is mapped, the scale factors are stored using a different scheme (bank toggle mode). Need to apply a different hacking strategy in that case
        if args.mu_tiling != "":
            print(f"\033[94m INFO: Modifying address generator config based on MU tiling from {args.mu_tiling}. This change will be applied to all inputs and outputs...\033[0m")
            meta = hack_addr_gen_for_mu_tiling(meta, args.mu_tiling)

        k_dim_host_tiling = "K_DIM_HOST_TILING" in os.environ and os.environ["K_DIM_HOST_TILING"] == "1"
        if k_dim_host_tiling:
            print(f"\033[94m INFO: Modifying address generator config for K dimension host tiling...\033[0m")
            meta = hack_addr_gen_for_k_dim_host_tiling(meta)

        print("writing to", outputName)
        with open(outputName, 'w', encoding='utf-8') as fileout:
            json.dump(meta, fileout, indent=2)
    except Exception as e:
        # Dump the error location and traceback
        # Dump broken metadata even if it failed for easier debugging
        print("\033[91m ERROR: Exception occurred during design meta processing!\033[0m")
        print("Full traceback:")
        traceback.print_exc()
        # Save the broken meta to design_meta_broken.json
        try:
            print(f"\033[93m WARNING: Saving broken metadata to bin/design_meta_broken.json...\033[0m")
            with open("bin/design_meta_broken.json", 'w', encoding='utf-8') as fileout:
                json.dump(meta, fileout, indent=2)
        except Exception as write_error:
            print(f"\033[91m ERROR: Failed to write broken metadata: {write_error}\033[0m")
        # Re-raise the original exception
        raise


if __name__ == "__main__":
    main()
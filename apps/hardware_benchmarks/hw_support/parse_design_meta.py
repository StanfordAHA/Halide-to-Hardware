import argparse
import json
import os
import pprint
import sys
import re
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

    for input_index, input_dict in enumerate(meta["IOs"]["inputs"]):
        assert "io_tiles" in input_dict, "io_tiles must be key of input_dict"
        for tile_index, io_tile in enumerate(input_dict["io_tiles"]):
            # Check if 'x_pos' of this io_tile is in 'glb_inputs' of the GLB_BANK_CONFIG
            if io_tile["x_pos"] in meta["GLB_BANK_CONFIG"].get("glb_inputs", []):
                meta["IOs"]["inputs"][input_index]["io_tiles"][tile_index]["is_glb_input"] = 1
            else:
                meta["IOs"]["inputs"][input_index]["io_tiles"][tile_index]["is_glb_input"] = 0

            # Add E64_packed flag for input tiles
            meta["IOs"]["inputs"][input_index]["io_tiles"][tile_index]["E64_packed"] = input_pack_map.get(io_tile["x_pos"], 0)

    # Add E64_packed flag for output tiles
    for output_index, output_dict in enumerate(meta["IOs"]["outputs"]):
        assert "io_tiles" in output_dict, "io_tiles must be key of output_dict"
        for tile_index, io_tile in enumerate(output_dict["io_tiles"]):
            meta["IOs"]["outputs"][output_index]["io_tiles"][tile_index]["E64_packed"] = output_pack_map.get(io_tile["x_pos"], 0)


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
            # Enforce assertion only if packing is used
            if pack_x_set and (len(entry["io_tiles"]) % 4 != 0):
                raise AssertionError("Number of io_tiles must be a multiple of 4 for E64 mode")

            new_io_tiles = []
            shape_updated = False
            for tile in entry["io_tiles"]:
                position = (tile["x_pos"], tile["y_pos"])
                if position not in unique_positions:
                    unique_positions.add(position)

                    # Apply extent scaling only for packed x-positions
                    if tile["x_pos"] in pack_x_set:
                        if "addr" in tile and "extent" in tile["addr"]:
                            tile["addr"]["extent"][0] *= 4
                            shape_updated = True
                        else:
                            print("ERROR: addr or extent not found in tile. Confirm that deisgn_top.json and design.place are correct for E64 mode. (Hint: Is unroll a multiple of 4?)")
                            sys.exit(1)

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



def hack_addr_gen_for_mu_tiling(meta, mu_tiling_file):
    """
    Hack the address generator config based on the matrix unit tiling.
    This function reads the tiling file and modifies the address generator config in the design meta.
    """
    # Read the tiling file
    if not os.path.isfile(mu_tiling_file):
        print(f"\033[91m ERROR: Tiling file {mu_tiling_file} does not exist. Cannot modify address generator config.\033[0m")
        sys.exit(1)

    # Get the GLB DMA config
    dimensionality, strides, extents = get_glb_dma_config(mu_tiling_file)
    # Update the address generator config in the design meta
    for io_type in ["inputs", "outputs"]:
        # TODO: This really shouldn't be applied to ALL inputs and outputs. In future, need some sort of metadata to specify which inputs and outputs are influenced by the matrix unit tiling
        for io in meta["IOs"][io_type]:
            if "io_tiles" in io:
                for tile in io["io_tiles"]:
                    addr = tile.get("addr", {})
                    if addr:
                        # Update the strides, extents, and dimensionality based on MU tiling
                        addr["cycle_stride"] = [1] * dimensionality # reading/writing on every RV handshake, so cycle stride is all 1
                        addr["dimensionality"] = dimensionality
                        addr["extent"] = extents
                        if io_type == "inputs":
                            addr["read_data_stride"] = strides
                        elif io_type == "outputs":
                            addr["write_data_stride"] = strides
                    # Add a new key to indicate that this is modified for MU tiling
                    tile["hacked_for_mu_tiling"] = True
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
    with open(outputName, 'w', encoding='utf-8') as fileout:
        halide_gen_args = os.getenv("HALIDE_GEN_ARGS")
        if halide_gen_args is not None:
            # If pad_o in args call padding functions to modify extents
            if "pad_o_left" in halide_gen_args or "pad_o_right" in halide_gen_args: parseLoopExtentforPadding(meta, halide_gen_args)
            # If NUM_GLB_TILING is set then edit extent if necessary
            if os.getenv("NUM_GLB_TILING") is not None and atoi(os.getenv("NUM_GLB_TILING")) > 0: parseLoopExtentforTiling(meta, halide_gen_args)

        if os.path.isfile("bin/glb_bank_config.json"): addGLBBankConfig(meta)

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

        print("writing to", outputName)
        json.dump(meta, fileout, indent=2)


if __name__ == "__main__":
    main()

import argparse
import json
import os
import pprint
import sys
import re

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

    with open(filename, "r") as readFile:
        designTop = json.load(readFile)
        topName = designTop["top"]
        prefix, success, designName = topName.partition("global.")
        assert prefix == "" and success
        print("parsing design top:", designName)

        coreirInstances = designTop["namespaces"]["global"]["modules"][designName]["instances"]
        for inst in coreirInstances:
            if inst.startswith("io16in_"):
                # this is a data input
                ioName = findBetween(inst, "io16in_", "_clkwrk")
                addr = coreirInstances[inst]["metadata"]["glb2out_0"]
                metaIn = findIO(meta["IOs"]["inputs"], ioName)
                if "io_tiles" in metaIn:
                    metaIn["io_tiles"].append({"name":inst, "addr":addr, "mode":"STATIC"})
                else:
                    metaIn["io_tiles"] = [{"name":inst, "addr":addr, "mode":"STATIC"}]

                # change read_data_stride based on the number of input tiles
                metaIn["io_tiles"]


            elif inst.startswith("io16_"):
                # this is a data output
                ioName = findBetween(inst, "io16_", "_clkwrk")
                addr = coreirInstances[inst]["metadata"]["in2glb_0"]
                metaOut = findIO(meta["IOs"]["outputs"], ioName)
                if "io_tiles" in metaOut:
                    metaOut["io_tiles"].append({"name":inst, "addr":addr, "mode":"VALID"})
                else:
                    metaOut["io_tiles"] = [{"name":inst, "addr":addr, "mode":"VALID"}]

        # alter the shape and data stride
        for input_struct in meta["IOs"]["inputs"]:
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

# Hacky functions to tile xy loop when flattened and add padding to extents
def unflatten_extent(addr_dict, X_dim, pad_o_left=0, pad_o_right=0):
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
            if 'write_data_stride' in addr_dict:
                addr_dict['write_data_stride'].insert(i+1, addr_dict['write_data_stride'][i] * X_dim)
            elif 'read_data_stride' in addr_dict:
                addr_dict['read_data_stride'].insert(i+1, addr_dict['read_data_stride'][i] * X_dim)
            else:
                raise ValueError("write_data_stride or read_data_stride not found in addr_dict")
            break
        elif ext % X_dim == 0:
            found_X_cnt += 1
            addr_dict['dimensionality'] += 1
            addr_dict['extent'][i] = X_dim + (pad_o_left + pad_o_right)
            addr_dict['extent'].insert(i+1, ext // X_dim)
            addr_dict['cycle_stride'].insert(i+1, addr_dict['cycle_stride'][i] * X_dim)
            if 'write_data_stride' in addr_dict:
                addr_dict['write_data_stride'].insert(i+1, addr_dict['write_data_stride'][i] * X_dim)
            elif 'read_data_stride' in addr_dict:
                addr_dict['read_data_stride'].insert(i+1, addr_dict['read_data_stride'][i] * X_dim)
            else:
                raise ValueError("write_data_stride or read_data_stride not found in addr_dict")
    assert found_X_cnt == 2, "X_dim and Y_dim not found in addr_dict['extent']"

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
        unflatten_extent(addr_dict, X_dim, pad_o_left, pad_o_right)

    # Add HALIDE_GEN_ARGS to meta file
    if meta.get("HALIDE_GEN_ARGS") is None: meta["HALIDE_GEN_ARGS"] = args_dict

def addGLBBankConfig(meta):
    with open("bin/glb_bank_config.json", "r") as f:
        glb_json = json.load(f)
    meta["GLB_BANK_CONFIG"] = glb_json
    for input_index, input_dict in enumerate(meta["IOs"]["inputs"]):
        assert "io_tiles" in input_dict, "io_tiles must be key of input_dict"
        for tile_index, io_tile in enumerate(input_dict["io_tiles"]):
            # Check if 'x_pos' of this io_tile is in 'glb_inputs' of the GLB_BANK_CONFIG
            if io_tile["x_pos"] in meta["GLB_BANK_CONFIG"].get("glb_inputs", []):
                meta["IOs"]["inputs"][input_index]["io_tiles"][tile_index]["is_glb_input"] = 1
            else:
                meta["IOs"]["inputs"][input_index]["io_tiles"][tile_index]["is_glb_input"] = 0

# Function to change extent for glb tiling or add dimensions
def parseLoopExtentforTiling(meta, halide_gen_args):
    # Get unroll values
    args = halide_gen_args.split()
    args_dict = {key: int(value) for key, value in (item.split('=') for item in halide_gen_args.split())}
    unroll = args_dict.get('unroll', 0)
    n_ic = args_dict.get('n_ic', 0)

    # Unflatten extent for all io tiles and check dimensionality
    for input in meta['IOs']['inputs']:
        io_tiles_list = input['io_tiles']
        # Get X_dim
        shape_list = input['shape']
        assert shape_list[-1] == shape_list[-2], "Only square shape supported for glb tiling."
        X_dim = shape_list[-1]
        for io_tile in io_tiles_list:
            addr_dict = io_tile['addr']
            unflatten_extent(addr_dict, X_dim)
            assert addr_dict['dimensionality'] == 2, "Implement fully unrolling along channel first"
    for output in meta['IOs']['outputs']:
        io_tile_list = output['io_tiles']
        # Get X_dim
        shape_list = output['shape']
        assert shape_list[-1] == shape_list[-2], "Only square shape supported for glb tiling."
        X_dim = shape_list[-1]
        for io_tile in io_tile_list:
            addr_dict = io_tile['addr']
            unflatten_extent(addr_dict, X_dim)
            assert addr_dict['dimensionality'] == 2, "Implement fully unrolling along channel first"

    # Add HALIDE_GEN_ARGS to meta file
    if meta.get("HALIDE_GEN_ARGS") is None: meta["HALIDE_GEN_ARGS"] = args_dict
    if meta.get("NUM_GLB_TILING") is None: meta["NUM_GLB_TILING"] = os.getenv("NUM_GLB_TILING")

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
        if "trunc_size" in halide_gen_args and meta.get("HALIDE_GEN_ARGS") is None: 
            args = halide_gen_args.split()
            args_dict = {key: int(value) for key, value in (item.split('=') for item in halide_gen_args.split())}
            meta["HALIDE_GEN_ARGS"] = args_dict

        # pprint.pprint(meta, fileout, indent=2, compact=True)
        print("writing to", outputName)
        json.dump(meta, fileout, indent=2)

if __name__ == "__main__":
    main()

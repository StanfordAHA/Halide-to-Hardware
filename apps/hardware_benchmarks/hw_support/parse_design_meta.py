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
                    metaIn["io_tiles"].append({"name":inst, "addr":addr})
                else:
                    metaIn["io_tiles"] = [{"name":inst, "addr":addr}]

                # change read_data_stride based on the number of input tiles
                metaIn["io_tiles"]


            elif inst.startswith("io16_"):
                # this is a data output
                ioName = findBetween(inst, "io16_", "_clkwrk")
                addr = coreirInstances[inst]["metadata"]["in2glb_0"]
                metaOut = findIO(meta["IOs"]["outputs"], ioName)
                if "io_tiles" in metaOut:
                    metaOut["io_tiles"].append({"name":inst, "addr":addr})
                else:
                    metaOut["io_tiles"] = [{"name":inst, "addr":addr}]

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
            # change the read data stride
            #for io_tile in input_struct["io_tiles"]:
            #    data_stride = io_tile["addr"]["read_data_stride"]
            #    io_tile["addr"]["read_data_stride"] = [stride // num_tiles for stride in data_stride]
            #        #assert(stride % num_tiles == 0), f"input stride is {stride}"

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
            # change the write data stride
            #for io_tile in output_struct["io_tiles"]:
            #    data_stride = io_tile["addr"]["write_data_stride"]
            #    io_tile["addr"]["write_data_stride"] = [stride // num_tiles for stride in data_stride]
            #        #assert(stride % num_tiles == 0), f"output stride is {stride}"

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
                _input['datafile'] = 'input_padded_shuffle.raw'
            elif _input['datafile'][0:6] == 'kernel':
                _input['datafile'] = 'kernel_shuffle.raw'
        outputs = meta['IOs']['outputs']
        for _output in outputs:
            if _output['datafile'][0:9] == 'hw_output':
                _output['datafile'] = 'hw_output_shuffle.raw'

    outputName = 'bin/design_meta.json'
    with open(outputName, 'w', encoding='utf-8') as fileout:
        # pprint.pprint(meta, fileout, indent=2, compact=True)
        print("writing to", outputName)
        json.dump(meta, fileout, indent=2)

if __name__ == "__main__":
    main()

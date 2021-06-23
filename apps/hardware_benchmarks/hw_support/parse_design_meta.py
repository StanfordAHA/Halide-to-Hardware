import argparse
import json
import os
import pprint
import sys

def parseArguments():
    # Create argument parser
    parser = argparse.ArgumentParser()

    # Positional mandatory arguments
    parser.add_argument("DesignMeta", help="design_meta.json: file so far", type=str)

    # Optional arguments
    parser.add_argument("--top", help="design_top.json: parse out address sequence", type=str, default=None)
    parser.add_argument("--place", help="design.place: parse IO placement", type=str, default=None)

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

            elif inst.startswith("io16_"):
                # this is a data output
                ioName = findBetween(inst, "io16_", "_clkwrk")
                addr = coreirInstances[inst]["metadata"]["in2glb_0"]
                metaOut = findIO(meta["IOs"]["outputs"], ioName)
                if "io_tiles" in metaOut:
                    metaOut["io_tiles"].append({"name":inst, "addr":addr})
                else:
                    metaOut["io_tiles"] = [{"name":inst, "addr":addr}]


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
        bin_directory = args.DesignMeta.replace("/design_meta_halide.json", "");
        for file in os.listdir(bin_directory):
            if file.endswith(".bs"):
                meta["testing"]["bitstream"] = file
                break

        if args.top != None:
            parseDesignTop(meta, args.top)

        if args.place != None:
            parseDesignPlace(meta, args.place)

    outputName = 'bin/design_meta.json'
    with open(outputName, 'w', encoding='utf-8') as fileout:
        # pprint.pprint(meta, fileout, indent=2, compact=True)
        print("writing to", outputName)
        json.dump(meta, fileout, indent=2)

if __name__ == "__main__":
    main()

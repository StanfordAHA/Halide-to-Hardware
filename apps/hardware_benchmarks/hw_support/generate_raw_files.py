import argparse
import json
import os
import pprint
import sys
import re

# python ../../hw_support/generate_raw_files.py bin/design_meta_halide.json
def parseArguments():
    # Create argument parser
    parser = argparse.ArgumentParser()

    # Positional mandatory arguments
    parser.add_argument("DesignMeta", help="design_meta_halide.json: should list inputs and outputs", type=str)

    # Parse arguments
    args = parser.parse_args()

    return args


def main():
    args = parseArguments()

    with open(args.DesignMeta, "r") as designMeta:
        meta = json.load(designMeta)

        slash_index = args.DesignMeta.rfind("/")
        bin_directory = args.DesignMeta[0:slash_index]

        # generate input raw files
        inputs = meta["IOs"]["inputs"]
        for inputname in inputs:
            raw_name = bin_directory + "/" + inputname["name"] + ".raw"
            print(f"input: {raw_name}")
            os.system(f"make {raw_name}")

        # generate output raw files
        outputs = meta["IOs"]["outputs"]
        for outputname in inputs:
            raw_name = bin_directory + "/" + outputname["name"] + ".raw"
            print(f"output: {raw_name}")
            os.system(f"make {raw_name}")
        

if __name__ == "__main__":
    main()

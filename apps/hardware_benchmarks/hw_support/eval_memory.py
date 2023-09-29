import argparse
import json
import os
import pprint
import sys
import re
import io
from contextlib import redirect_stdout

# python ../../hw_support/eval_design_top.py bin/design_top.json
def parseArguments():
    # Create argument parser
    parser = argparse.ArgumentParser()

    # Positional mandatory arguments
    parser.add_argument("MemoryCpp", help="app_memory.cpp: app memory file to evaluate", type=str)
    
    # Parse arguments
    args = parser.parse_args()

    return args

        
        
def main():
    args = parseArguments()

    with open(args.MemoryCpp, "r") as MemoryCpp:
        text = MemoryCpp.read()

        num_compute_kernels = text.count("->add_function")
        
        num_buffers = text.count("prg.buffer_port_widths")

        print(f"Number of compute kernels: {num_compute_kernels}")
        print(f"Number of buffers: {num_buffers}")

        
if __name__ == "__main__":
    main()

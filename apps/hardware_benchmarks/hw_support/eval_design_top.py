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
    parser.add_argument("DesignTop", help="design_top.json: design file to evaluate", type=str)
    # Optional arguments
    parser.add_argument("--resources", help="count instances in design file", action='store_true')
    parser.add_argument("--latency", help="calculate latency using port controller stencil valids", action='store_true')
    parser.add_argument("--memory", help="calculate memory words stored using input/output latencies", action='store_true')
    
    # Parse arguments
    args = parser.parse_args()

    return args

class Module:
    numPEs = 0
    numAdds = 0
    numMuls = 0
    numShifts = 0
    numPELuts = 0
    PELuts = {}
    numPEOthers = 0
    PEOthers = {}
    numMEMs = 0
    numMemories = 0
    numControllers = 0 # memories used a counters or stencil valids
    numPonds = 0
    numREGs = 0
    numRegisters = 0
    numConsts = 0
    numREGbits = 0
    numIns = 0
    numInVals = 0
    numInBits = 0
    numInResets = 0
    numOuts = 0
    numOutVals = 0
    numOutValids = 0
    numOutBits = 0
    latency = None
    latency_calc = ""
    memory = None

    stencil_valid_names = set()
    memory_names = set()

    def __init__(self, json):
        self.json = json
    
    def countResources(self):
        instances = self.json["instances"]
        
        for instname in instances:
            inst = instances[instname]
            #print(inst)
            if "genref" in inst:
                if inst["genref"] == "coreir.reg":
                    self.numREGs += 1
                    self.numRegisters += 1
                elif inst["genref"] == "coreir.const":
                    self.numREGs += 1
                    self.numConsts += 1
                elif inst["genref"] == "cgralib.Mem":
                    self.numMEMs += 1
                    if "port_controller" in instname:
                        self.numControllers += 1
                        self.stencil_valid_names.add(instname)
                    else:
                        self.numMemories += 1
                        self.memory_names.add(instname)
                elif inst["genref"] == "cgralib.Pond":
                    self.numMEMs += 1
                    self.numPonds += 1
                elif inst["genref"] == "cgralib.PE":
                    self.numPEs += 1
                    pe_op = inst["modargs"]["alu_op"][1] if inst["modargs"].get("alu_op") else ""
                    #print(instname, inst)
                    if inst["modargs"].get("lut_value"):
                        self.numPELuts += 1
                        #print(inst["modargs"]["lut_value"], instname, inst)
                        if pe_op != "":
                            lut_value = pe_op
                        else:
                            lut_value = inst["modargs"]["lut_value"][1]
                            self.numPEs -= 1
                        if self.PELuts.get(lut_value):
                            self.PELuts[lut_value] += 1
                        else:
                            self.PELuts[lut_value] = 1

                    elif pe_op == "rshft" or pe_op == "lshft":
                        self.numShifts += 1
                    elif pe_op == "add" or pe_op == "sub":
                        self.numAdds += 1
                    elif pe_op == "mult_0" or pe_op == "mult_1" or pe_op == "mult_2":
                        self.numMuls += 1
                    else:
                        #print("PE op encountered:", pe_op)
                        self.numPEOthers += 1
                        if self.PEOthers.get(pe_op):
                            self.PEOthers[pe_op] += 1
                        else:
                            self.PEOthers[pe_op] = 1
                elif inst["genref"] == "cgralib.IO" and inst["modargs"]["mode"][1] == "out":
                    self.numOuts += 1
                    self.numOutVals += 1
                elif inst["genref"] == "cgralib.IO" and inst["modargs"]["mode"][1] == "in":
                    self.numIns += 1
                    self.numInVals += 1
                elif inst["genref"] == "coreir.add":
                    pass
                elif inst["genref"] == "coreir.mul":
                    pass
                else:
                    print("gen not found:", inst)
                    
            elif "modref" in inst:
                if inst["modref"] == "cgralib.BitIO" and inst["modargs"]["mode"][1] == "out":
                    if "valid" in instname:
                        self.numOutValids += 1
                    else:
                        self.numOutBits += 1
                        self.numOuts += 1
                elif inst["modref"] == "cgralib.BitIO" and inst["modargs"]["mode"][1] == "in":
                    if "reset" in instname:
                        self.numInResets += 1
                    else:
                        #print(instname, " is ", inst)
                        self.numInBits += 1
                elif inst["modref"] == "corebit.const":
                    self.numREGs += 1
                    self.numREGbits += 1
                elif inst["modref"] == "corebit.reg":
                    self.numREGs += 1
                    self.numREGbits += 1
                else:
                    print("mod not found:", inst)
            else:
                print("inst not found", inst)

    def printResources(self):
        if len(self.PEOthers) > 0:
            print("Other PE counts:")
            for pe_op in self.PEOthers:
                print(f"  {pe_op}: {self.PEOthers[pe_op]}")
        if len(self.PELuts) > 0:
            print("LUT value counts:")
            for lut_val in self.PELuts:
                common_name = "(const1)" if lut_val=="8'hff" else ("(const0)" if lut_val=="8'h00" else ("(and)" if lut_val == "8'h88" else ("(const1)" if lut_val == "8'h3f" else "")))
                print(f"  {lut_val}: {self.PELuts[lut_val]} {common_name}")
        print(f"numPEs = {self.numPEs} (Add = {self.numAdds}, Mul = {self.numMuls}, Shift = {self.numShifts}, lut = {self.numPELuts}, Other = {self.numPEOthers})")
        print(f"numMEMs = {self.numMemories} (Mem = {self.numMemories}, Controller = {self.numControllers}, Pond = {self.numPonds})")
        print(f"numREGs = {self.numREGs}, (Reg = {self.numRegisters}, Const = {self.numConsts}, Bit = {self.numREGbits})")
        print(f"numIns = {self.numIns} (Val = {self.numInVals}, Bit = {self.numInBits}, Reset = {self.numInResets})")
        print(f"numOuts = {self.numOuts} (Val = {self.numOutVals}, Bit = {self.numOutBits}, Valid = {self.numOutValids})")

    def collectStencilValids(self):
        instances = self.json["instances"]
        
        for instname in instances:
            inst = instances[instname]
            if "genref" in inst and inst["genref"] == "cgralib.Mem":
                    if "port_controller" in instname:
                        self.stencil_valid_names.add(instname)


    # Using the port controllers, calculate the full application latency based on stencil valids.
    def calculateLatency(self):
        if len(self.stencil_valid_names) == 0:
            self.collectStencilValids()

        for instname in self.stencil_valid_names:
            inst = self.json["instances"][instname]
            stencil_valid = inst["modargs"]["config"][1]["stencil_valid"]

            # Calculate latency using start, extent, and stride.
            latency = stencil_valid["cycle_starting_addr"][0]
            latency_calc = "" + str(latency)
            
            extent = stencil_valid["extent"]
            cycle_stride = stencil_valid["cycle_stride"]
            for i in range(0, stencil_valid["dimensionality"]):
                latency += (extent[i] - 1) * cycle_stride[i]
                latency_calc += f" + {extent[i] - 1}*{cycle_stride[i]}"

            # save to results
            latency += 1
            latency_calc += " + 1"
            if self.latency == None:
                self.latency = latency
                self.latency_calc = latency_calc
            else:
                #assert(self.latency == latency)
                if latency > self.latency:
                    self.latency = latency
                    self.latency_calc = latency_calc
                
        print(f"Total Latency is {self.latency} = {self.latency_calc}")

    def collectMemoryNames(self):
        instances = self.json["instances"]
        
        for instname in instances:
            inst = instances[instname]
            if "genref" in inst and inst["genref"] == "cgralib.Mem":
                    if "port_controller" not in instname:
                        self.memory_names.add(instname)

        
    # For each memory, look at how many words are stored between the first word entering and leaving.
    def calculateMemory(self):
        if len(self.memory_names) == 0:
            self.collectMemoryNames()

        # Go through each of the memories
        self.memory = 0
        self.memory_names = sorted(self.memory_names)
        for instname in self.memory_names:
            if "dsa" in instname:
                continue
            
            inst = self.json["instances"][instname]

            json = inst["modargs"]["config"][1]
            input_addr = json["in2agg_0"]
            input_start = input_addr["cycle_starting_addr"][0]
            output_addr = json["tb2out_0"]
            output_start = output_addr["cycle_starting_addr"][0]

            # Determine total cycle length based on either output port
            if "tb2out_1" in json:
                output1_addr = json["tb2out_1"]
                output1_start = output1_addr["cycle_starting_addr"][0]
                length = max(output_start, output1_start) - input_start
                print(f"Mem {instname} from {input_start} to {output_start} and {output1_start} for len={length}")
                if output1_start > output_start:
                    output_addr = output1_addr
                    output_start = output1_start
            else:
                length = output_start - input_start
                print(f"Mem {instname} from {input_start} to {output_start} for len={length}")

            # Add only the cycles where data is written
            write_strides = input_addr["cycle_stride"]
            write_extents = input_addr["extent"]
            writes = 1
            for stride, extent in zip(write_strides, write_extents):
                if stride * extent < length:
                    writes *= extent
                else:
                    partial = 1 + (length-1) // stride
                    writes *= partial

            print(f"  strides={write_strides} extents={write_extents} writes={writes}")
            self.memory += writes

        
        print(f"Total Memory is {self.memory} words")
        
        
def main():
    args = parseArguments()

    with open(args.DesignTop, "r") as designTop:
        top = json.load(designTop)
        fullAppName = top["top"]
        [namespace, appname] = fullAppName.split(".")
        print("App is", appname)

        appJson = top["namespaces"][namespace]["modules"][appname]
        outputVector = ""

        m = Module(appJson)

        # count resources used
        if args.resources:
            m.countResources()
            m.printResources()

        # calculate time for execution
        if args.latency:
            m.calculateLatency()

        # calculate time for execution
        if args.memory:
            m.calculateMemory()

            
    outputName = 'bin/mapped_eval'
    with open(outputName, 'a') as fileout:
        # pprint.pprint(meta, fileout, indent=2, compact=True)
        print("writing to", outputName)
        if args.resources:
            f = io.StringIO()
            with redirect_stdout(f):
                m.printResources()
            fileout.write(f.getvalue())
                  
        if args.latency:
            fileout.write(f"Total Latency is {m.latency} = {m.latency_calc}\n")

        if args.memory:
            fileout.write(f"Total Memory is {m.memory} words\n")
        

if __name__ == "__main__":
    main()

#!/usr/bin/env python3

import sys
import json
import os
from pretty_format_json import pretty_format_json

def main():
    if len(sys.argv) < 6:
        print(f"Usage: {sys.argv[0]} <in.json> <out.json> <flush.json> <pond.json> <stencil_valid.json>")
        sys.exit(1)

    in_json_path     = sys.argv[1]
    out_json_path    = sys.argv[2]
    flush_json_path  = sys.argv[3]
    pond_json_path   = sys.argv[4]
    stval_json_path  = sys.argv[5]

    with open(in_json_path, "r") as f:
        top_data = json.load(f)

    with open(flush_json_path, "r") as f:
        flush_latencies = json.load(f)

    with open(pond_json_path, "r") as f:
        pond_latencies = json.load(f)

    with open(stval_json_path, "r") as f:
        stencil_valid_latencies = json.load(f)

    # We only apply Pond pipeline logic if environment var is set
    pond_pipelined = (os.environ.get("POND_PIPELINED") == "1")

    #------------------------------------------------
    # Helper function to “add latency” if we find a
    # "cycle_starting_addr" array in a dictionary
    #------------------------------------------------
    def add_latency_to_cycle_starting_addr(addr_array, latency, inst_name=""):
        """
        addr_array should be a list of integers, e.g. [0, 4, 32, ...].
        We add 'latency' to the first element. We also check for negative result.
        """
        if not addr_array:
            return
        if len(addr_array) > 0:
            new_val = addr_array[0] + latency
            if new_val < 0:
                print(
                    "\033[91m"
                    + f"Warning: Negative schedule for '{inst_name}' => cycle_starting_addr[0] became {new_val}. "
                      "This application will probably fail."
                    + "\033[0m"
                )
            addr_array[0] = new_val

    #------------------------------------------------
    # Helper function to set cycle_starting_addr
    # to an absolute value (instead of adding).
    #------------------------------------------------
    def set_cycle_starting_addr(addr_array, new_val, inst_name=""):
        if not addr_array:
            return
        if new_val < 0:
            print(
                "\033[91m"
                + f"Warning: Negative schedule for '{inst_name}' => cycle_starting_addr[0] set to {new_val}. "
                  "This application will probably fail."
                + "\033[0m"
            )
        addr_array[0] = new_val

    #------------------------------------------------
    # We want to locate "metadata" for each instance
    # and search for "cycle_starting_addr" inside the
    # nested dictionaries. The typical pattern:
    # 
    # modules -> each "instances" -> instance’s metadata
    # Possibly inside metadata["config"] -> subkeys
    # 
    # We check genref or modref to see if it’s cgralib.Mem,
    # cgralib.Pond, global.Pond, etc.
    #------------------------------------------------
    def process_instance(inst_name, inst_info):
        """
        Depending on the genref/modref, adjust
         cycle_starting_addr in metadata.

        The instance name is used to see if it’s in flush_latencies,
        stencil_valid_latencies, or pond_latencies.
        """
        genref = inst_info.get("genref", "")
        modref = inst_info.get("modref", "")
        metadata = inst_info.get("metadata", {})

        if not isinstance(metadata, dict):
            return  # skip if no valid metadata

        #----------------------------------------------
        # 1) cgralib.Mem
        #----------------------------------------------
        if genref == "cgralib.Mem":
            # If this instance is in flush_latencies => add flush
            if inst_name in flush_latencies:
                # We look inside metadata for subblocks that might have "cycle_starting_addr"
                # A typical place is metadata["config"] plus each subdict in it.
                process_metadata_for_latency(metadata, flush_latencies[inst_name], inst_name)

            # If in stencil_valid_latencies => add that
            if inst_name in stencil_valid_latencies:
                process_metadata_for_latency(metadata, stencil_valid_latencies[inst_name], inst_name)

        #----------------------------------------------
        # 2) cgralib.Pond
        #----------------------------------------------
        if genref == "cgralib.Pond":
            if inst_name in flush_latencies:
                process_metadata_for_latency(metadata, flush_latencies[inst_name], inst_name)

        #----------------------------------------------
        # 3) global.Pond + pipeline
        #----------------------------------------------
        if modref == "global.Pond" and pond_pipelined:
            # If pond pipelined, we might set regfile2out_0 cycle_starting_addr to a single value
            if inst_name in pond_latencies:
                # Typically stored in metadata["regfile2out_0"]["cycle_starting_addr"]
                rfo_meta = metadata.get("regfile2out_0", {})
                if isinstance(rfo_meta, dict):
                    csa = rfo_meta.get("cycle_starting_addr")
                    if isinstance(csa, list) and len(csa) > 0:
                        # set (not add)
                        set_cycle_starting_addr(csa, pond_latencies[inst_name], inst_name)


    #------------------------------------------------
    # Recursively process any dict that might have
    # "cycle_starting_addr" in it, adding 'latency'
    # to the first element of that array.
    #------------------------------------------------
    def process_metadata_for_latency(meta_dict, latency, inst_name):
        """
        Recursively walk 'meta_dict' (which should be a dict), 
        searching for "cycle_starting_addr" as a key with a list value.
        If found, add `latency` to the first element.
        """
        if not isinstance(meta_dict, dict):
            return
        # Check top level
        if "cycle_starting_addr" in meta_dict:
            csa = meta_dict["cycle_starting_addr"]
            if isinstance(csa, list) and len(csa) > 0:
                add_latency_to_cycle_starting_addr(csa, latency, inst_name)

        # Recurse deeper
        for k, v in meta_dict.items():
            if isinstance(v, dict):
                process_metadata_for_latency(v, latency, inst_name)
            # If it’s a list, we might skip it or check each element
            elif isinstance(v, list):
                # In some JSON structures, a list might contain dictionaries
                # so we can loop them:
                for elem in v:
                    if isinstance(elem, dict):
                        process_metadata_for_latency(elem, latency, inst_name)
            # else: skip


    #----------------------------------------------
    # Traverse the top_data to find modules/instances
    #----------------------------------------------
    namespaces = top_data.get("namespaces", {})
    global_ns  = namespaces.get("global", {})
    modules    = global_ns.get("modules", {})

    for mod_name, mod_data in modules.items():
        if not isinstance(mod_data, dict):
            continue
        instances = mod_data.get("instances", {})
        if not isinstance(instances, dict):
            continue

        for inst_name, inst_info in instances.items():
            # process each instance
            process_instance(inst_name, inst_info)

    #----------------------------------------------
    # Write out the updated JSON
    #----------------------------------------------
    with open(out_json_path, "w") as fout:
        fout.write(pretty_format_json(top_data))

if __name__ == "__main__":
    main()
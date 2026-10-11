"""
raw2txt.py

Convert all .raw files under the given path to .txt files
Usage:
    python raw2txt.py /path/to/dir
    python raw2txt.py some_file.raw
"""
import os
import json
import sys
import argparse
import numpy as np

def cgra_data_width():
    """CGRA data word width in bits: the lake collateral's data_width
    (LAKE_COLLATERAL_JSON_MEM, which `aha map` sets), else 16."""
    path = os.environ.get("LAKE_COLLATERAL_JSON_MEM", "")
    if not path:
        return 16
    with open(path) as f:
        return int(json.load(f).get("data_width", 16))

def convert_file(raw_path: str):
    # Read the raw file and swap bytes back to native (little‑endian) order
    # One CGRA data word per element
    nbytes = cgra_data_width() // 8
    data = np.fromfile(raw_path, dtype=f">u{nbytes}")
    txt_path = os.path.splitext(raw_path)[0] + '.txt'
    with open(txt_path, 'w') as f:
        for i in range(0, data.size, 8):
            chunk = data[i:i+8]
            hex_vals = [f"{int(v):0{2 * nbytes}x}" for v in chunk]
            f.write(" ".join(hex_vals) + "\n")
    print(f"[raw2txt] Converted {raw_path!r} → {txt_path!r}")

def main():
    parser = argparse.ArgumentParser(
        description="Convert .raw files to .txt with 8 hex values per line."
    )
    parser.add_argument(
        "path", nargs="?",
        help="Directory to search (recursively) or a single .raw file"
    )
    parser.add_argument(
        "--data-width", action="store_true",
        help="Print the CGRA data word width (bits) from the lake collateral and exit"
    )
    args = parser.parse_args()
    if args.data_width:
        print(cgra_data_width())
        return
    if args.path is None:
        parser.error("path is required")

    if os.path.isdir(args.path):
        for root, _, files in os.walk(args.path):
            for fn in files:
                if fn.lower().endswith(".raw"):
                    convert_file(os.path.join(root, fn))
    elif os.path.isfile(args.path) and args.path.lower().endswith(".raw"):
        convert_file(args.path)
    else:
        sys.exit(f"Error: no .raw files found at {args.path!r}")

if __name__ == "__main__":
    main()

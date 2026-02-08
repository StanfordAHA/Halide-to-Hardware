import sys
import os
import numpy as np

def hw_output_txt_to_raw(input_txt_path, output_raw_path):
    """
    Reads hex strings from a text file, converts them to uint16,
    swaps byte order (Big Endian), and saves to a raw binary file.
    """
    try:
        # Read the file content
        with open(input_txt_path, 'r') as f:
            hex_values = f.read().split()

        if not hex_values:
            print("Warning: Input file is empty.")
            return

        # Convert hex strings to unsigned 16-bit integers
        uint16_array = np.array([int(h, 16) for h in hex_values], dtype=np.uint16)

        # Swap byte order to Big Endian 
        uint16_array_be = uint16_array.byteswap().newbyteorder('>')

        # Save to raw binary file
        uint16_array_be.tofile(output_raw_path)

        print(f"Successfully converted '{input_txt_path}' to '{output_raw_path}'")

    except FileNotFoundError:
        print(f"Error: The file '{input_txt_path}' was not found.")
        sys.exit(1)
    except ValueError as e:
        print(f"Error: Could not convert data. Ensure input contains valid hex. ({e})")
        sys.exit(1)
    except Exception as e:
        print(f"An unexpected error occurred: {e}")
        sys.exit(1)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python script.py <input_txt_path> <output_raw_path>")
        sys.exit(1)

    input_path = sys.argv[1]
    output_path = sys.argv[2]

    hw_output_txt_to_raw(input_path, output_path)

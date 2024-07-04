import argparse

def hex_to_raw(input_filename, output_filename):
    with open(input_filename, 'r') as file, open(output_filename, 'wb') as outfile:
        for line in file:
            # Split the line into hex values
            hex_values = line.strip().split()
            # Convert each hex value to a binary byte and write to output file
            for hex_value in hex_values:
                # Convert hex to integer, then pack into a byte
                byte = int(hex_value, 16).to_bytes(2, byteorder='big')
                outfile.write(byte)

def main():
    parser = argparse.ArgumentParser(description="Convert a file of hex values to a raw binary file.")
    parser.add_argument("input_filename", help="The path to the input file containing plain text hex values.")
    parser.add_argument("output_filename", help="The path to the output raw file to save the binary data.")
    args = parser.parse_args()

    hex_to_raw(args.input_filename, args.output_filename)

if __name__ == "__main__":
    main()

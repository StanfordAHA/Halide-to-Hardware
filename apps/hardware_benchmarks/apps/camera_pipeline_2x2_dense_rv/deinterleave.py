import argparse
import sys
import os

def deinterleave_file(input_path, output_path, num_streams=12):
    # Check if input file exists
    if not os.path.exists(input_path):
        print(f"Error: The file '{input_path}' was not found.")
        sys.exit(1)

    try:
        # Read Input
        with open(input_path, 'r') as f:
            raw_text = f.read()

        flat_data = raw_text.split()

        if not flat_data:
            print("Error: The input file is empty.")
            sys.exit(1)

        # Deinterleave
        streams = {}
        for i in range(num_streams):
            streams[i] = flat_data[i::num_streams]

        # Write Output
        with open(output_path, 'w') as f:
            f.write(f"Deinterleaved Data ({num_streams} Streams)\n")
            f.write("=" * 40 + "\n\n")

            for stream_id, data in streams.items():
                data_str = " ".join(data)
                f.write(f"Stream {stream_id}:\n")
                f.write(f"{data_str}\n\n")

        print(f"Success! Processed {len(flat_data)} items.")
        print(f"Output written to: {output_path}")

    except Exception as e:
        print(f"An error occurred: {e}")
        sys.exit(1)

def main():
    parser = argparse.ArgumentParser(description="Deinterleave hex data into 12 streams.")

    # Input file argument
    parser.add_argument(
        '-i', '--input',
        type=str,
        required=True,
        help='Path to the input .txt file containing hex data'
    )

    # Output file argument (Optional)
    parser.add_argument(
        '-o', '--output',
        type=str,
        default='output.txt',
        help='Path to the output file (default: output.txt)'
    )

    args = parser.parse_args()

    deinterleave_file(args.input, args.output)

if __name__ == "__main__":
    main()
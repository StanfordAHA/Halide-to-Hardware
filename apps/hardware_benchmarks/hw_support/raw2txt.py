import os
import sys

def raw_to_txt(raw_file_path, txt_file_path):
    try:
        # Open the .raw file in binary mode
        with open(raw_file_path, 'rb') as raw_file:
            # Read the binary data
            raw_data = raw_file.read()

        # Open the .txt file in write mode
        with open(txt_file_path, 'w') as txt_file:
            # Ensure the data length is a multiple of 2 (for 16-bit words)
            if len(raw_data) % 2 != 0:
                raise ValueError("The .raw file contains incomplete 16-bit words.")

            # Convert the binary data to 16-bit words (2 bytes per word)
            word_data = [raw_data[i:i+2] for i in range(0, len(raw_data), 2)]

            # Write 8 words per line, with a space separating each word
            for i in range(0, len(word_data), 8):
                line_words = word_data[i:i+8]
                hex_words = [word.hex() for word in line_words]
                txt_file.write(' '.join(hex_words) + '\n')

        print(f"Successfully converted {raw_file_path} to {txt_file_path}")

    except Exception as e:
        print(f"An error occurred: {e}")

if __name__ == "__main__":
    # Example usage:
    if len(sys.argv) != 3:
        print("Usage: python raw_to_txt.py <input.raw> <output.txt>")
    else:
        raw_file = sys.argv[1]
        txt_file = sys.argv[2]

        if not os.path.exists(raw_file):
            print(f"Error: The file {raw_file} does not exist.")
        else:
            raw_to_txt(raw_file, txt_file)

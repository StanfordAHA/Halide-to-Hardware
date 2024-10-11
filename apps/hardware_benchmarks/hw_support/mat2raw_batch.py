import scipy.io
import numpy as np
import sys
import os

def mat_to_raw(input_name, output_name):
    print(f"[mat2raw] Converting {input_name} to {output_name}...")

    mat = scipy.io.loadmat(input_name)
    # Assuming there's only one variable of interest and it's not one of the
    # automatic variables added by MATLAB ('__version__', '__header__', and '__globals__')
    data_keys = [key for key in mat.keys() if not key.startswith('__')]
    if len(data_keys) != 1:
        raise ValueError(f"The .mat file {input_name} contains none or more than one variable.")
    
    array = np.array(mat[data_keys[0]], dtype=np.uint16)
    # Transpose the array. Reverse the axes.
    transposed_array = array.transpose(np.arange(array.ndim)[::-1])

    transposed_array.byteswap().tofile(output_name)
    print(f"[mat2raw] Conversion of {output_name} Complete.")

def convert_all_mat_files(directory):
    for root, dirs, files in os.walk(directory):
        for file in files:
            if file.endswith(".mat"):
                input_path = os.path.join(root, file)
                output_path = os.path.join(root, os.path.splitext(file)[0] + ".raw")
                mat_to_raw(input_path, output_path)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python mat2raw_batch.py <directory_path>")
        sys.exit(1)

    directory = sys.argv[1]
    if not os.path.isdir(directory):
        print(f"Error: {directory} is not a valid directory.")
        sys.exit(1)

    convert_all_mat_files(directory)

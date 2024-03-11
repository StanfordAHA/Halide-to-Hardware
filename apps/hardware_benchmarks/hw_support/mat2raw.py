import scipy.io
import numpy as np
import sys

input_name = sys.argv[1]
output_name = sys.argv[2]
print(f"[mat2raw] Converting {input_name} to {output_name}...")

mat = scipy.io.loadmat(input_name)
# Assuming there's only one variable of interest and it's not one of the
# automatic variables added by MATLAB ('__version__', '__header__', and '__globals__')
data_keys = [key for key in mat.keys() if not key.startswith('__')]
if len(data_keys) != 1:
    raise ValueError("The .mat file contains none or more than one variable.")
array = np.array(mat[data_keys[0]], dtype=np.uint16)

# Transpose the array. Reverse the axes.
# This changes the storage order in the file to have the first dimension changing fastest
transposed_array = array.transpose(np.arange(array.ndim)[::-1])

transposed_array.byteswap().tofile(output_name)
print(f"[mat2raw] Conversion of {output_name} Complete.")

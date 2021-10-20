import scipy.io
import numpy as np
import sys

input_name = "output_cpu.mat"
output_name = "output_cpu.raw"

# overwrite
input_name = sys.argv[1]
output_name = sys.argv[2]
print(f"[mat2raw] Converting {input_name} to {output_name}...")

mat = np.array(scipy.io.loadmat(input_name))
array = np.array(mat.item(0)['output_cpu'])
array.astype('int16').tofile(output_name)
print(f"[mat2raw] Conversion of {output_name} Complete.")

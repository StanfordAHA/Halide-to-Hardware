import scipy.io
import numpy as np
import sys
import os
#input_name = "bin/output_cpu.mat"
#output_name = "bin/output_cpu.pgm"

input_name = sys.argv[1]
output_name = sys.argv[2]

print(f"[mat2pgm] Converting {input_name} to {output_name}...")
# load .mat files
input_mat = np.array(scipy.io.loadmat(input_name))
# process first .mat file
input_base = os.path.basename(input_name)
input_array = np.array(input_mat.item(0)[os.path.splitext(input_base)[0]])
input_array = input_array.transpose(2, 1, 0)
input_size = input_array.size
print ("input shape: ", input_array.shape)
print ("input size: ", input_size)
flat_input = np.ndarray.flatten(input_array)
# print("Flat Input:\n", flat_input)
# process second .mat file
total_size = input_size 
total_input = np.zeros([total_size], dtype='>i2')
input_idx = 0
kernel_idx = 0
print(f"Start converting total {total_size} pixels.")
for i in range(total_size):
    total_input[i] = flat_input[i]
# np.set_printoptions(threshold=np.inf)
# print("Total Input:", total_input)
fout = open(output_name, 'wb')
pgmHeader = 'P5\n' + str(1) + ' ' + str(total_size) + '\n' + str(65535) +  '\n'
pgmHeader_byte = bytearray(pgmHeader, 'utf-8')
fout.write(pgmHeader_byte)
total_input_str = ' '.join(map(str, total_input))
#fout.write(bytearray(total_input_str, 'utf-8'))
fout.write(total_input)
fout.close()
print(f"[mat2pgm] Conversion of {output_name} Complete.")

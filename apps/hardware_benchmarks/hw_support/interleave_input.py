import scipy.io
import numpy as np
import sys
import os
# input_name = sys.argv[1]
# kernel_name = sys.argv[2]
# output_name = sys.argv[3]
input_name = "input_padded.mat"
kernel_name = "kernel.mat"
output_name = sys.argv[1]
#output_name = "output.pgm"
print(f"[interleave_input] Converting {input_name} + {kernel_name} to {output_name}...")
# load .mat files
input_mat = np.array(scipy.io.loadmat(input_name))
kernel_mat = np.array(scipy.io.loadmat(kernel_name))
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
kernel_base = os.path.basename(kernel_name)
kernel_array = np.array(kernel_mat.item(0)[os.path.splitext(kernel_name)[0]])
kernel_array = kernel_array.transpose(3, 2, 1, 0)
kernel_size = kernel_array.size
print ("kernel shape: ", kernel_array.shape)
print ("kernel size: ", kernel_size)
flat_kernel = np.ndarray.flatten(kernel_array)
# print("Flat Kernel:\n", flat_kernel)
total_size = np.max([input_size, kernel_size]) * 2
#total_size = 784*9 
#total_input = np.zeros([total_size], dtype='int16')
total_input = np.zeros([total_size], dtype='>i2')
input_idx = 0
kernel_idx = 0
print(total_size)
for i in range(total_size):
    #if (i % 9) < 8:
    if (i % 2) == 0:
        total_input[i] = flat_input[input_idx]
        input_idx += 1
    else:
        if kernel_idx < kernel_size:
            total_input[i] = flat_kernel[kernel_idx]
            kernel_idx += 1
        else:
            total_input[i] = 0x11
print(total_input[0])
print(total_input[1])
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
print(f"[interleave_input] Conversion of {output_name} Complete.")

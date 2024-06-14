import numpy as np
import pandas as pd


trace_file_path = "./out.txt"


im_width = 1250
im_height = 1120


my_scale_factors = np.zeros((im_height, im_width))

trace_file = open(trace_file_path, "r")

line = trace_file.readline() 
while(line):
    if 'Store' in line:
        line = line.strip()
        components = line.split(' = ')

        indices = components[0].split('(')[1].strip(')').split(', ')
        pixel_value = float(components[1])

        x = int(indices[0])
        y = int(indices[1])


        my_scale_factors[y][x] = pixel_value
        
    line = trace_file.readline()
 
trace_file.close()
print(f"Max value: {np.max(my_scale_factors)}")
print(f"Min value: {np.min(my_scale_factors)}")
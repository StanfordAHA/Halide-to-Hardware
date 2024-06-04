import numpy as np
import pandas as pd





trace_file_path = "./out.txt"


im_width = 1264
im_height = 1136


my_weight_darks = np.zeros((im_height, im_width))

trace_file = open(trace_file_path, "r")

line = trace_file.readline() 
while(line):
    if 'Store' in line:
        line = line.strip()
        components = line.split(' = ')

        indices = components[0].split('(')[1].strip(')').split(', ')
        weight_dark_value = float(components[1])

        x = int(indices[0])
        y = int(indices[1])


        my_weight_darks[y][x] = weight_dark_value
        
    line = trace_file.readline()
 
trace_file.close()
#print(my_weight_darks)


# Now, compare to Kayvon's norm_dark_weight output 
tolerance = 0.001
k_norm_dark_weight_filepath = f"/aha/cs348v_camera_asst/norm_dark_weight_out.txt" 


k_norm_dark_weight_file= open(k_norm_dark_weight_filepath, 'r')
line = k_norm_dark_weight_file.readline()

y = 0
while(line):
    line = line.strip()
    row = line.split(', ')

    for x in range(len(row)):
       kayvon_weight_dark_value = float(row[x])

       abs_difference = abs(kayvon_weight_dark_value - my_weight_darks[y][x])
        
       if (abs_difference > tolerance):
            print(f"At x = {x}, y = {y}, expected {kayvon_weight_dark_value} but got {my_weight_darks[y][x]}. Difference is {abs_difference}.")

    line = k_norm_dark_weight_file.readline()
    y += 1
k_norm_dark_weight_file.close()
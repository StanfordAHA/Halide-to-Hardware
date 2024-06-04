import numpy as np
import pandas as pd

trace_file_path = "./out.txt"

pyramid_width_list = [1264, 632, 316, 158]
pyramid_height_list = [1136, 568, 284, 142]

pyramid_level = 0

pyramid_width = pyramid_width_list[pyramid_level]
pyramid_height = pyramid_height_list[pyramid_level]

kayvon_pyramid = np.zeros((pyramid_height, pyramid_width))



# First load Kayvon's weight pyramid
k_bright_weight_gPyramid_filepath = f"/aha/cs348v_camera_asst/bright_weight_gPyrmaid_level_{pyramid_level}.txt" 


k_bright_weight_gPyramid_file = open(k_bright_weight_gPyramid_filepath, 'r')
line = k_bright_weight_gPyramid_file.readline()

y = 0
while(line):
    line = line.strip()
    row = line.split(', ')

    for x in range(len(row)):
       kayvon_pyramid[y][x] = float(row[x])
       
    line = k_bright_weight_gPyramid_file.readline()
    y += 1

k_bright_weight_gPyramid_file.close()


# print(kayvon_pyramid[0][1][0])
# print(kayvon_pyramid[0][1][1])
# print(kayvon_pyramid[0][1][2])




# Now, compare to my exposure fusion output 
tolerance = 0.001

trace_file = open(trace_file_path, "r")

line = trace_file.readline() 
while(line):
    if 'Store' in line:
        line = line.strip()
        components = line.split(' = ')

        indices = components[0].split('(')[1].strip(')').split(', ')
        my_weight_value = float(components[1])

        x = int(indices[0])
        y = int(indices[1])

        abs_difference = abs(my_weight_value - kayvon_pyramid[y][x])
        
        if (abs_difference > tolerance):
            print(f"At x = {x}, y = {y}, expected {kayvon_pyramid[y][x]} but got {my_weight_value}. Difference is {abs_difference}.")
        
    line = trace_file.readline()
 
trace_file.close()
#print(my_pixels)



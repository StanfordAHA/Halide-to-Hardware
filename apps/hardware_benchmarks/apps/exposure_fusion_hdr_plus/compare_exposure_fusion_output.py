import numpy as np
import pandas as pd

trace_file_path = "./out.txt"


im_width = 1250
im_height = 1120
num_color_channels = 3

kayvon_image = np.zeros((im_height, im_width, num_color_channels))



# First load Kayvon's exposure fusion output
k_EF_red_filepath = f"/aha/cs348v_camera_asst/post_exposure_fusion_red_out.txt" 
k_EF_green_filepath = f"/aha/cs348v_camera_asst/post_exposure_fusion_green_out.txt" 
k_EF_blue_filepath = f"/aha/cs348v_camera_asst/post_exposure_fusion_blue_out.txt" 


k_EF_red_file = open(k_EF_red_filepath, 'r')
k_EF_green_file = open(k_EF_green_filepath, 'r')
k_EF_blue_file = open(k_EF_blue_filepath, 'r')

red_line = k_EF_red_file.readline()
green_line = k_EF_green_file.readline()
blue_line = k_EF_blue_file.readline()

y = 0
while(red_line):
    red_line = red_line.strip()
    red_row = red_line.split(', ')

    green_line = green_line.strip()
    green_row = green_line.split(', ')

    blue_line = blue_line.strip()
    blue_row = blue_line.split(', ')

    for x in range(len(red_row)):
       kayvon_image[y][x][0] = red_row[x]
       kayvon_image[y][x][1] = green_row[x]
       kayvon_image[y][x][2] = blue_row[x]
       
    red_line = k_EF_red_file.readline()
    green_line = k_EF_green_file.readline()
    blue_line = k_EF_blue_file.readline()
    y += 1

k_EF_red_file.close()
k_EF_green_file.close()
k_EF_blue_file.close()


# print(kayvon_image[0][1][0])
# print(kayvon_image[0][1][1])
# print(kayvon_image[0][1][2])




# Now, compare to my exposure fusion output 
tolerance = 0.001

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
        c = int(indices[2])

        abs_difference = abs(pixel_value - kayvon_image[y][x][c])
        
        if (abs_difference > tolerance):
            print(f"At x = {x}, y = {y}, c = {c} expected {kayvon_image[y][x][c]} but got {pixel_value}. Difference is {abs_difference}.")
        
    line = trace_file.readline()
 
trace_file.close()
#print(my_pixels)



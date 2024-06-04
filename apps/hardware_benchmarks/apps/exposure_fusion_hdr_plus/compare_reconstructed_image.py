import numpy as np
import pandas as pd

trace_file_path = "./out.txt"



reconstructed_image_width = 1264
reconstructed_image_height = 1136

kayvon_reconstructed_image = np.zeros((reconstructed_image_height, reconstructed_image_width))



# First load Kayvon's reconstructed reconstructed_image
k_reconstructed_image_filepath = f"/aha/cs348v_camera_asst/reconstructed.txt" 


k_reconstructed_image_file = open(k_reconstructed_image_filepath, 'r')
line = k_reconstructed_image_file.readline()

y = 0
while(line):
    line = line.strip()
    row = line.split(', ')

    for x in range(len(row)):
       kayvon_reconstructed_image[y][x] = float(row[x])
       
    line = k_reconstructed_image_file.readline()
    y += 1

k_reconstructed_image_file.close()


# print(kayvon_reconstructed_image[0][1][0])
# print(kayvon_reconstructed_image[0][1][1])
# print(kayvon_reconstructed_image[0][1][2])




# Now, compare to my exposure fusion output 
tolerance = 0.001

trace_file = open(trace_file_path, "r")

line = trace_file.readline() 
while(line):
    if 'Store' in line:
        line = line.strip()
        components = line.split(' = ')

        indices = components[0].split('(')[1].strip(')').split(', ')
        my_reconstructed_value = float(components[1])

        x = int(indices[0])
        y = int(indices[1])

        print("Comparing...")
        abs_difference = abs(my_reconstructed_value - kayvon_reconstructed_image[y][x])
        
        if (abs_difference > tolerance):
            print(f"At x = {x}, y = {y}, expected {kayvon_reconstructed_image[y][x]} but got {my_reconstructed_value}. Difference is {abs_difference}.")
        
    line = trace_file.readline()
 
trace_file.close()
#print(my_pixels)



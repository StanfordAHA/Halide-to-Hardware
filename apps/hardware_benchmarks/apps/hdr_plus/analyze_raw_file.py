import numpy as np
import pandas as pd


# Now, compare the  values
k_frame0_filepath = f"./taxi_10bit_0.txt"

#tolerance = 0.001
tolerance = 1.0

# Compare frame 0
k_frame0_file = open(k_frame0_filepath, 'r')
line = k_frame0_file.readline()

y = 0
while(line):
    line = line.strip()
    pixel_row = line.split(', ')

    for x in range(len(pixel_row)):
        pixel_val = float(pixel_row[x])
        
        if (pixel_val > 1022):
            print(f"At coordinate {x}, {y} for frame 0, pixel value is {pixel_val}")

    line = k_frame0_file.readline()
    y += 1
k_frame0_file.close()
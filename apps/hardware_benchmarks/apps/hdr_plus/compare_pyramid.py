import numpy as np
import pandas as pd


trace_file_path = "./out.txt"

pyramid_width_list = [625, 312, 156, 78, 39]
pyramid_height_list = [560, 280, 140, 70, 35]

num_frames = 3
pyramid_level = 2

pyramid_width = pyramid_width_list[pyramid_level]
pyramid_height = pyramid_height_list[pyramid_level]

pyramid = np.zeros((pyramid_height, pyramid_width, num_frames))

trace_file = open(trace_file_path, "r")

line = trace_file.readline() 
while(line):
    if '<' in line:
        line = line.strip()
        components = line.split('<')

        x_indices = components[1].strip('>, ').split(', ')
        y_indices = components[2].strip('>, ').split(', ')
        frame_indices = components[3].strip('>) =').split(', ')
        pixel_values = components[4].strip('>').split(', ')


        for i in range(len(pixel_values)):
            y = int(y_indices[i])
            x = int(x_indices[i])
            frame = int(frame_indices[i]) 
            pixel_val = float(pixel_values[i]) 

            pyramid[y][x][frame] = pixel_val  
        
    line = trace_file.readline()

#print(pyramid)   
trace_file.close()


# Now, compare the pyramid values
k_frame0_pyramid_filepath = f"/aha/cs348v_camera_asst/taxi_raw_pyramid_frame_0_level_{pyramid_level}.txt"
k_frame1_pyramid_filepath = f"/aha/cs348v_camera_asst/taxi_raw_pyramid_frame_1_level_{pyramid_level}.txt"
k_frame2_pyramid_filepath = f"/aha/cs348v_camera_asst/taxi_raw_pyramid_frame_2_level_{pyramid_level}.txt"

tolerance = 0.001

# Compare frame 0
k_frame0_pyramid_file = open(k_frame0_pyramid_filepath, 'r')
line = k_frame0_pyramid_file.readline()

y = 0
while(line):
    line = line.strip()
    pixel_row = line.split(', ')

    for x in range(len(pixel_row)):
        pixel_val = float(pixel_row[x])
        
        difference = abs(pixel_val - pyramid[y][x][0])
        
        if (difference > tolerance):
            print(f"At coordinate {x}, {y} for frame 0, difference is {difference}")

    line = k_frame0_pyramid_file.readline()
    y += 1
k_frame0_pyramid_file.close()


# Compare frame 1
k_frame1_pyramid_file = open(k_frame1_pyramid_filepath, 'r')
line = k_frame1_pyramid_file.readline()

y = 0
while(line):
    line = line.strip()
    pixel_row = line.split(', ')

    for x in range(len(pixel_row)):
        pixel_val = float(pixel_row[x])
        
        difference = abs(pixel_val - pyramid[y][x][1])
        
        if (difference > tolerance):
            print(f"At coordinate {x}, {y} for frame 1, difference is {difference}")

    line = k_frame1_pyramid_file.readline()
    y += 1
k_frame1_pyramid_file.close()


# Compare frame 2 
k_frame2_pyramid_file = open(k_frame2_pyramid_filepath, 'r')
line = k_frame2_pyramid_file.readline()

y = 0
while(line):
    line = line.strip()
    pixel_row = line.split(', ')

    for x in range(len(pixel_row)):
        pixel_val = float(pixel_row[x])
        
        difference = abs(pixel_val - pyramid[y][x][2])
        
        if (difference > tolerance):
            print(f"At coordinate {x}, {y} for frame 2, difference is {difference}")

    line = k_frame2_pyramid_file.readline()
    y += 1
k_frame2_pyramid_file.close()
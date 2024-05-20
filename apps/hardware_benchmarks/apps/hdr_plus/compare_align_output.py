import numpy as np
import pandas as pd



num_tx_list = [40, 20, 10, 5, 3] 
num_ty_list = [35, 18, 9, 5, 3] 

trace_file_path = "./out.txt"
level = 0
num_frames = 3

num_tx = num_tx_list[level]
num_ty = num_ty_list[level]

align_coords = np.zeros((num_frames, num_ty, num_tx, 2))

trace_file = open(trace_file_path, "r")

line = trace_file.readline() 
while(line):
    if 'Store' in line:
        line = line.strip()
        components = line.split(' = ')

        indices = components[0].split('(')[1].strip(')').split(', ')
        value = components[1]

        tx = int(indices[0])
        ty = int(indices[1])
        xy = int(indices[2])
        frame_num = int(indices[3])


        align_coords[frame_num][ty][tx][xy] = value  
        
    line = trace_file.readline()

#print(align_coords)   
trace_file.close()


# Now, compare the alignment values
k_frame0_align_filepath = f"/aha/cs348v_camera_asst/align_offsets_frame_0_level_{level}.txt" 
k_frame1_align_filepath = f"/aha/cs348v_camera_asst/align_offsets_frame_1_level_{level}.txt" 
k_frame2_align_filepath = f"/aha/cs348v_camera_asst/align_offsets_frame_2_level_{level}.txt" 



# Compare frame 1
k_frame1_align_file= open(k_frame1_align_filepath, 'r')
line = k_frame1_align_file.readline()

ty = 0
while(line):
    line = line.strip()
    tx_row = line.split(', ')

  
    for tx in range(len(tx_row)):
        xy_pair = tx_row[tx]
        xy_pair = xy_pair.lstrip('[')
        xy_pair = xy_pair.strip(']')
        xy_pair_components = xy_pair.split(' ')

        x_align_coord = int(xy_pair_components[0])
        y_align_coord = int(xy_pair_components[1])
        
        if (x_align_coord != align_coords[1][ty][tx][0] or y_align_coord != align_coords[1][ty][tx][1]):
            print(f"At tile {tx}, {ty} for frame 1, got {align_coords[1][ty][tx][0]}, {align_coords[1][ty][tx][1]} instead of {x_align_coord}, {y_align_coord}")

    line = k_frame1_align_file.readline()
    ty += 1
k_frame1_align_file.close()


# Compare frame 2
k_frame2_align_file= open(k_frame2_align_filepath, 'r')
line = k_frame2_align_file.readline()

ty = 0
while(line):
    line = line.strip()
    tx_row = line.split(', ')

  
    for tx in range(len(tx_row)):
        xy_pair = tx_row[tx]
        xy_pair = xy_pair.lstrip('[')
        xy_pair = xy_pair.strip(']')
        xy_pair_components = xy_pair.split(' ')

        x_align_coord = int(xy_pair_components[0])
        y_align_coord = int(xy_pair_components[1])
        
        if (x_align_coord != align_coords[2][ty][tx][0] or y_align_coord != align_coords[2][ty][tx][1]):
            print(f"At tile {tx}, {ty} for frame 2, got {align_coords[2][ty][tx][0]}, {align_coords[2][ty][tx][1]} instead of {x_align_coord}, {y_align_coord}")

    line = k_frame2_align_file.readline()
    ty += 1
k_frame2_align_file.close()
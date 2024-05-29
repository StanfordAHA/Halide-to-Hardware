import numpy as np
import pandas as pd




trace_file_path = "./weights_out.txt"


num_tx = 156
num_ty = 139
num_frames = 3

# First, load my weights 
trace_file = open(trace_file_path, "r")

my_weights = np.zeros((num_frames, num_ty, num_tx))

line = trace_file.readline() 
tolerance = 0.001
while(line):
    if 'Store' in line:
        line = line.strip()
        components = line.split(' = ')


        weight_values = components[1].strip('<').strip('>').split(', ')
        indices = components[0].split('(')[1].strip(')').split('>, ')
        

        tx_indices = indices[0].strip('<').split(', ')
        ty_indices = indices[1].strip('<').split(', ')
        frame_indices = indices[2].strip('<').strip('>').split(', ')

        for i in range(len(weight_values)):
            tx = int(tx_indices[i])
            ty = int(ty_indices[i])
            frame_num = int(frame_indices[i])

            my_weights[frame_num][ty][tx] = float(weight_values[i])
                
    line = trace_file.readline()

#print(my_weights)   
trace_file.close()



# Now, compare to Kayvon's weights 
tolerance = 0.001

# Compare frame 1 
k_frame1_weights_filepath = f"/aha/cs348v_camera_asst/weights_frame_1.txt" 

k_frame1_weights_file= open(k_frame1_weights_filepath, 'r')
line = k_frame1_weights_file.readline()

ty = 0
while(line):
    line = line.strip()
    tx_row = line.split(', ')

    for tx in range(len(tx_row)):
        kayvon_weight = float(tx_row[tx])

        abs_difference = abs(kayvon_weight - my_weights[1][ty][tx])
        if(abs_difference > tolerance):
            print(f"At tx {tx}, ty {tx}, for frame 1, expected {kayvon_weight} but got {my_weights[1][ty][tx]}. Difference is {abs_difference}.")
                
    ty+=1

    line = k_frame1_weights_file.readline()
  
k_frame1_weights_file.close()


# Compare frame 2 
k_frame2_weights_filepath = f"/aha/cs348v_camera_asst/weights_frame_2.txt" 

k_frame2_weights_file= open(k_frame2_weights_filepath, 'r')
line = k_frame2_weights_file.readline()

ty = 0
while(line):
    line = line.strip()
    tx_row = line.split(', ')

    for tx in range(len(tx_row)):
        kayvon_weight = float(tx_row[tx])

        abs_difference = abs(kayvon_weight - my_weights[2][ty][tx])
        if(abs_difference > tolerance):
            print(f"At tx {tx}, ty {tx}, for frame 2, expected {kayvon_weight} but got {my_weights[2][ty][tx]}. Difference is {abs_difference}.")
                
    ty+=1

    line = k_frame2_weights_file.readline()
  
k_frame2_weights_file.close()
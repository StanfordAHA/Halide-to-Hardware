import numpy as np
import pandas as pd




trace_file_path = "./out.txt"


num_tx = 156
num_ty = 139
tsize_x = 16
tsize_y = 16

kayvon_divided_tiles = np.zeros((num_ty, num_tx, tsize_y, tsize_x))

# First, load Kayvon's divided tiles 
k_divided_tiles_filepath = f"/aha/cs348v_camera_asst/divided_tiles.txt" 

k_divided_tiles_file= open(k_divided_tiles_filepath, 'r')
line = k_divided_tiles_file.readline()

ty = 0
tx = 0
yi = 0
xi = 0
while(line):
    line = line.strip()

    if('Tile' in line):
        components = line.split("_")
        tx = int(components[1])
        ty = int(components[2])
        yi = 0
        xi = 0

    else: 
        row = line.split(', ')
    
        for xi in range(len(row)):
            pixel_val = float(row[xi])
            kayvon_divided_tiles[ty][tx][yi][xi] = pixel_val

        yi+=1

    line = k_divided_tiles_file.readline()
  
k_divided_tiles_file.close()

#print(kayvon_divided_tiles)


# Now, compare to my divided tiles 
trace_file = open(trace_file_path, "r")

lineCount = 0
line = trace_file.readline() 
tolerance = 0.001
while(line):
    if 'Store' in line:
        line = line.strip()
        components = line.split(' = ')

        indices = components[0].split('(')[1].strip(')').split(', ')
        value = float(components[1])


        xi = int(indices[0])
        yi = int(indices[1])
        tx = int(indices[2])
        ty = int(indices[3])
        

        abs_difference = abs(value - kayvon_divided_tiles[ty][tx][yi][xi])
        if(abs_difference > tolerance):
            print(f"At tx {tx}, ty {tx}, xi {xi}, yi {yi}, expected {kayvon_divided_tiles[ty][tx][yi][xi]} but got {value}. Difference is {abs_difference}.")
                
    line = trace_file.readline()
    lineCount += 1
    if(lineCount == 256):
        break

#print(align_coords)   
trace_file.close()



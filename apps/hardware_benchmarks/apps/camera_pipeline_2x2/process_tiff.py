import cv2
import math
import numpy as np

def int_to_binary(int_in):
    binary_string = ""
    curr_int = int_in
    for i in range(8):
        remainder = int(curr_int%2)
       
        binary_string = str(remainder) + binary_string
        curr_int = int(curr_int/2)

    #binary_string = reversed(binary_string)
    return binary_string


def frac_to_binary(frac_in):
    #print("---STARTING FRAC TO BINARY---")
    binary_string = ""
    curr_frac = frac_in
    for i in range(8):
        curr_frac = curr_frac * 2
        parts = math.modf(curr_frac)
        integer_part = parts[1]
        binary_string += str(int(integer_part))
        #print(integer_part)
        curr_frac = parts[0]
    return binary_string


def binary_to_uint16(binary_in):
    out = 0
    for i in range(16):
        out += int(binary_in[15-i]) * math.pow(2, i)
    return int(out)


def binary_to_int16(binary_in):
    out = 0
    out += -1 * int(binary_in[0]) * math.pow(2, 15)
    for i in range(15):
        out += int(binary_in[15-i]) * math.pow(2, i)
    return int(out)

def invert_and_add_one(binary_in):
    tmp = ""
    for i in range(16):
        if binary_in[i] == "1":
            tmp += "0"
        elif binary_in[i] == "0":
            tmp += "1"

    base_10_equivalent = int(tmp, 2)
    base_10_equivalent += 1
    out = bin(base_10_equivalent)

    # Want 16 bits (also account for 0b in front of the string, so add 2)
    while(len(out) < 16+2):
        out = out[0:2] + "0" + out[2:]
    
    return out[-16:]
        

def float_to_fixed(float_in, signed=False):
    if signed:
        # Negative number 
        if float_in < 0:
            float_in = -float_in
            float_parts = math.modf(float_in)
            fixed_integer_part = int_to_binary(float_parts[1])
            fixed_fraction_part = frac_to_binary(float_parts[0])
            tmp_fixed_binary_string = fixed_integer_part + fixed_fraction_part
            final_fixed_binary_string = invert_and_add_one(tmp_fixed_binary_string)

        else:
            float_parts = math.modf(float_in)
            fixed_integer_part = int_to_binary(float_parts[1])
            fixed_fraction_part = frac_to_binary(float_parts[0])
            final_fixed_binary_string = fixed_integer_part + fixed_fraction_part

        
        return binary_to_int16(final_fixed_binary_string)
    
    else:
        float_parts = math.modf(float_in)
        fixed_integer_part = int_to_binary(float_parts[1])
        fixed_fraction_part = frac_to_binary(float_parts[0])

        final_fixed_binary_string = fixed_integer_part + fixed_fraction_part

        return binary_to_uint16(final_fixed_binary_string)
    


def deinterleave(lens_shading_map):
    deinterleaved_lens_shading_map = np.zeros((2*lens_shading_map.shape[0], 2*lens_shading_map.shape[1]))
    
    # Setting a 2x2 grid (R G G B) on each loop iteration 
    for y in range(0, deinterleaved_lens_shading_map.shape[0], 2):
        for x in range(0, deinterleaved_lens_shading_map.shape[1], 2):
            deinterleaved_lens_shading_map[y][x] = lens_shading_map[int(y/2)][int(x/2)][0] # RED
            deinterleaved_lens_shading_map[y][x+1] = lens_shading_map[int(y/2)][int(x/2)][1] # GREEN_R
            deinterleaved_lens_shading_map[y+1][x] = lens_shading_map[int(y/2)][int(x/2)][2] # GREEN_B
            deinterleaved_lens_shading_map[y+1][x+1] = lens_shading_map[int(y/2)][int(x/2)][3] # BLUE
    
    return deinterleaved_lens_shading_map
    

# @param tiff: lens shading map of shape [13*2][17*2]
# @return expanded_lens_shading_map of shape [shape_out_outer][shape_out_inner] 
def bilinear_interp(tiff, shape_out_outer, shape_out_inner):
   
    print(tiff.shape)

    #expanded_lens_shading_map = np.zeros((shape_out_outer, shape_out_inner, 4))
    expanded_lens_shading_map = np.zeros((shape_out_outer, shape_out_inner))

    #print(expanded_lens_shading_map.shape)
    #exit()
    valid_ys = set()
    valid_xs = set()
    # Set values that don't need interpolation
    for y in range(tiff.shape[0]):
        for x in range(tiff.shape[1]):
            image_y = int((shape_out_outer-1)/(tiff.shape[0]-1) * y)
            image_x = int((shape_out_inner-1)/(tiff.shape[1]-1) * x) 
            expanded_lens_shading_map[image_y][image_x] = tiff[y][x]
            valid_ys.add(image_y)
            valid_xs.add(image_x)

    sorted_valid_ys = sorted(valid_ys)
    sorted_valid_xs = sorted(valid_xs)
    print(sorted_valid_ys)
    print(sorted_valid_xs)
    #exit()

    # Perform interpolation for intermediate values;
    tile_size_y = int((shape_out_outer-1)/(tiff.shape[0]-1)) + 1
    tile_size_x = int((shape_out_inner-1)/(tiff.shape[1]-1)) + 1
    #tile_size_y = int((shape_out_outer)/(tiff.shape[0]))
    #tile_size_x = int((shape_out_inner)/(tiff.shape[1])) 
    print(tile_size_y)
    print(tile_size_x)
    #exit()

    for valid_y_index in range(len(sorted_valid_ys)-1):
        for valid_x_index in range(len(sorted_valid_xs)-1):
            ll_y = sorted_valid_ys[valid_y_index]
            ll_x = sorted_valid_xs[valid_x_index]
            ur_y = sorted_valid_ys[valid_y_index + 1]
            ur_x = sorted_valid_xs[valid_x_index + 1]

            #print(f"Bounding box indices are ll_x:{ll_x}, ll_y:{ll_y}, ur_x:{ur_x}, ur_y:{ur_y}!")

            for y in range(ll_y, ur_y+1):
                for x in range(ll_x, ur_x+1):
                    
                    # Skip locations that were already set previously 
                    #if np.count_nonzero(expanded_lens_shading_map[y][x]) > 1:
                    if expanded_lens_shading_map[y][x] == 0:

                        # STEP 1: Find tile_y and tile_x
                        #tile_y = int(y/tile_size_y)
                        #tile_x = int(x/tile_size_x)

                        #print(f"Tile y and Tile x are {tile_y} and {tile_x} for y = {y} and x = {x}")

                        # STEP 2: From tile_y and tile_x, determine 4 nearest neighbors (bounding box)
                        #ll_x = max(0, (tile_x * tile_size_x)-1)
                        #ll_y = max(0, (tile_y * tile_size_y)-1)
                        #ll_x = tile_x * tile_size_x
                        #ll_y = tile_y * tile_size_y
                        #ur_x = ((tile_x+1) * tile_size_x)-1
                        #ur_y = ((tile_y+1) * tile_size_y)-1
                        #ur_x = ll_x + tile_size_x
                        #ur_y = ll_y + tile_size_y

                        if (ll_x < 0 or ll_y < 0 or ur_x > shape_out_inner-1 or ur_y > shape_out_outer-1):
                            raise IndexError(f"BB index out of bounds for {ll_x}, {ll_y}, {ur_x}, {ur_y}")
                        
                        if ((ll_x not in valid_xs) or (ur_x not in valid_xs) or (ll_y not in valid_ys) or (ur_y not in valid_ys)):
                            raise IndexError(f"Invalid bounding box indices for {ll_x}, {ll_y}, {ur_x}, {ur_y}!")
                        

                        #assert(np.count_nonzero(expanded_lens_shading_map[ll_y][ll_x]) != 0)
                        #assert(np.count_nonzero(expanded_lens_shading_map[ur_y][ll_x]) != 0)
                        #assert(np.count_nonzero(expanded_lens_shading_map[ll_y][ur_x]) != 0)
                        #assert(np.count_nonzero(expanded_lens_shading_map[ur_y][ur_x]) != 0)

                        assert(expanded_lens_shading_map[ll_y][ll_x] != 0)
                        assert(expanded_lens_shading_map[ur_y][ll_x] != 0)
                        assert(expanded_lens_shading_map[ll_y][ur_x] != 0)
                        assert(expanded_lens_shading_map[ur_y][ur_x] != 0)
                        
                        #print(f"The bbox coordinates are {ll_x}, {ll_y}, {ur_x}, {ur_y}!")

                        # STEP 3: Compute distances to each corner of the bounding box
                        d_ll = np.linalg.norm(np.array([x, y]) - np.array([ll_x, ll_y]))
                        d_ul = np.linalg.norm(np.array([x, y]) - np.array([ll_x, ur_y]))
                        d_lr = np.linalg.norm(np.array([x, y]) - np.array([ur_x, ll_y]))
                        d_ur = np.linalg.norm(np.array([x, y]) - np.array([ur_x, ur_y]))

                        # STEP 4: Using distances, compute 4 normalized weights
                        distance_sum = d_ll + d_ul + d_lr + d_ur
                        w_ll = distance_sum/d_ll
                        w_ul = distance_sum/d_ul
                        w_lr = distance_sum/d_lr
                        w_ur = distance_sum/d_ur

                        weight_sum = w_ll + w_ul + w_lr + w_ur

                        # Normalize weights
                        w_ll /= weight_sum
                        w_ul /= weight_sum
                        w_lr /= weight_sum
                        w_ur /= weight_sum

                        normalized_weight_sum = w_ll + w_ul + w_lr + w_ur
                        #print(normalized_weight_sum)
                        assert(0.99 < normalized_weight_sum < 1.01)


                        # STEP 5: Using weights, set value to be a weighted sum
                        final_val = w_ll * expanded_lens_shading_map[ll_y][ll_x] + w_ul * expanded_lens_shading_map[ur_y][ll_x] + w_lr \
                                    + w_lr * expanded_lens_shading_map[ll_y][ur_x] + w_ur * expanded_lens_shading_map[ur_y][ur_y]

                        expanded_lens_shading_map[y][x] = final_val

    print(tile_size_y)
    print(tile_size_x)

    
    print(f"Nonzero count: {np.count_nonzero(expanded_lens_shading_map)}")  
    print(expanded_lens_shading_map)     
    return expanded_lens_shading_map


def write_to_txt(lens_shading_map):
    ls_file = open("lens_shading_factors.txt", "w")
    ls_file.write("{")

    for y in range(lens_shading_map.shape[0]):
        for x in range(lens_shading_map.shape[1]):
            if x == 0:
                ls_file.write("{")
            ls_file.write(f"{float_to_fixed(lens_shading_map[y][x], signed=False)}")
            #ls_file.write(f"{str(int(lens_shading_map[y][x]))}")
                

            if x != lens_shading_map.shape[1]-1:
                ls_file.write(", ")
            
            
        if y == lens_shading_map.shape[0]-1:
            ls_file.write("}")
        else:
            ls_file.write("},\n")


    ls_file.write("};")
    ls_file.close()


def write_to_txt_no_brackets(lens_shading_map):
    ls_file = open("lens_shading_factors.txt", "w")

    for y in range(lens_shading_map.shape[0]):
        for x in range(lens_shading_map.shape[1]):
            ls_file.write(f"{float_to_fixed(lens_shading_map[y][x], signed=False)}")
            #ls_file.write(f"{str(int(lens_shading_map[y][x]))}")
                

            if x != lens_shading_map.shape[1]-1:
                ls_file.write(",")
            
            
        if y != lens_shading_map.shape[0]-1:
            ls_file.write("\n")

    ls_file.close()




# Read the .tiff file
tiff = cv2.imread("../hdr_plus/images/tiff/20171106_subset_bursts_0039_20141006_110442_472_lens_shading_map_N000.tiff", cv2.IMREAD_UNCHANGED)
print("TIFF FACTORS")
#print(tiff)
print(np.min(tiff))
print(np.max(tiff))
#print(max(tiff))
giraffe = cv2.imread("../hdr_plus/images/pgm/giraffe/20171106_subset_bursts_0039_20141006_110442_472_payload_N000.pgm")
dog = cv2.imread("../../../images/bayer_raw.png")

# Load the .bmp file
taxi = cv2.imread("../hdr_plus/images/bmp/taxi/taxi.bmp", cv2.IMREAD_UNCHANGED)

# Check if the image was loaded successfully
if taxi is None:
    print("Error: Unable to load taxi image")
    exit(1)

# Save the image as .png
result = cv2.imwrite("taxi.png", taxi)

png_taxi = cv2.imread("taxi.png")
print("PNG taxi dimensions:", png_taxi.shape)
print("PNG taxi datatype:", png_taxi.dtype)

hdr_plus_taxi = cv2.imread("../hdr_plus/bin/output_cpu.png")
print("HDR plus taxi dimensions:", hdr_plus_taxi.shape)
print("HDR plus taxi datatype:", hdr_plus_taxi.dtype)


#for y in range(png_taxi.shape[0]):
#     for x in range(png_taxi.shape[1]):
#         print(png_taxi[y][x][0])

for y in range(hdr_plus_taxi.shape[0]):
     for x in range(hdr_plus_taxi.shape[1]):
         c0 = hdr_plus_taxi[y][x][0]
         c1 = hdr_plus_taxi[y][x][1]
         c2 = hdr_plus_taxi[y][x][2]
        

         assert(c0 == c1 and c1 == c2)

# Check if the image was saved successfully
if not result:
    print("Error: Unable to save image")
    exit(1)

print("Image converted successfully to taxi.png")

if tiff is None:
    print("Error: Could not open the tiff file.")
    exit()

# Display tiff properties
print("tiff dimensions:", tiff.shape)
print("tiff datatype:", tiff.dtype)

print("giraffe dimensions:", giraffe.shape)
print("giraffe datatype:", giraffe.dtype)

# for y in range(giraffe.shape[0]):
#     for x in range(giraffe.shape[1]):
#         c0 = giraffe[y][x][0]
#         c1 = giraffe[y][x][1]
#         c2 = giraffe[y][x][2]
        
#         assert(c0 == c1 and c1 == c2)

print("dog dimensions:", dog.shape)
print("dog datatype:", dog.dtype)



#deinterleaved_tiff = deinterleave(tiff)
#print(deinterleaved_tiff)
#interpolated_deinterleaved_tiff = bilinear_interp(deinterleaved_tiff, giraffe.shape[0], giraffe.shape[1])
#write_to_txt_no_brackets(interpolated_deinterleaved_tiff)
#write_to_txt(np.zeros((giraffe.shape[0], giraffe.shape[1])))


# ls_file = open("lens_shading_factors.txt", "w")


# ls_file.write("{")
# for y in range(tiff.shape[0]):
#     for x in range(tiff.shape[1]):
#         image_y = int((giraffe.shape[0]-1)/(tiff.shape[0]-1) * y)
#         image_x = int((giraffe.shape[1]-1)/(tiff.shape[1]-1) * x)
#         if x == 0:
#             ls_file.write("{")
#         for channel in range(4):
#             if channel == 0:
#                 ls_file.write("{")
#             ls_file.write(f"{float_to_fixed(tiff[y][x][channel], signed=True)}")
#             if channel != 3:
#                 ls_file.write(", ")
#         ls_file.write("}")

#         if x != tiff.shape[1]-1:
#             ls_file.write(", ")

#         #print(f"Shading map {x}, {y} of value {float_to_fixed(tiff[y][x][2], signed=True)} for R channel maps onto {image_x}, {image_y} on the actual image")

#     if y == tiff.shape[0]-1:
#         ls_file.write("}")
#     else:
#         ls_file.write("},\n")
# ls_file.write("};")
# ls_file.close()


# print(float_to_fixed(1.859375, signed=True))
# print(float_to_fixed(-0.703125, signed=True))
# print(float_to_fixed(-0.15625, signed=True))
# print(float_to_fixed(-0.234375, signed=True))
# print(float_to_fixed(1.6640625, signed=True))
# print(float_to_fixed(-0.4296875, signed=True))
# print(float_to_fixed(0.0078125, signed=True))
# print(float_to_fixed(-0.65625, signed=True))
# print(float_to_fixed(1.640625, signed=True))


    

# Process the tiff (example: convert to grayscale)
#gray_tiff = cv2.cvtColor(tiff, cv2.COLOR_BGR2GRAY)

# Display the processed tiff
#cv2.imshow("Original tiff", tiff)
#cv2.imshow("Processed tiff (Grayscale)", gray_tiff)
#cv2.waitKey(0)
#cv2.destroyAllWindows()

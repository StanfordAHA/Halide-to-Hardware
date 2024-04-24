import cv2
import argparse
import numpy as np


parser = argparse.ArgumentParser()

parser.add_argument("-i", "--input_file")
parser.add_argument("-o", "--output_file")

args = parser.parse_args()


# Load the .bmp file
input_file_path = args.input_file
output_file_path = args.output_file

input_image = cv2.imread(input_file_path, cv2.IMREAD_UNCHANGED)

# Check if the image was loaded successfully
if input_image is None:
    print("Error: Unable to load input_image image")
    exit(1)

print(input_image.shape)
print(input_image[0][10][0])
print(input_image[0][10][1])
print(input_image[0][10][2])

output_image=input_image

Y_channel = np.zeros((input_image.shape[0], input_image.shape[1]))
U_channel = np.zeros((input_image.shape[0], input_image.shape[1]))
V_channel = np.zeros((input_image.shape[0], input_image.shape[1]))

print(Y_channel.shape)

for y in range(input_image.shape[0]):
    for x in range(input_image.shape[1]):
        r = input_image[y][x][0]
        g = input_image[y][x][1]
        b = input_image[y][x][2]


        # RGB to YUV
        Y_channel[y][x] = 0.299 * r + 0.587 * g + 0.114 * b
        U_channel[y][x] = 0.492 * (b - Y_channel[y][x])
        #U_channel[y][x] = 0
        V_channel[y][x] = 0.877 * (r - Y_channel[y][x])


        # YUV to RGB
        output_image[y][x][0] = Y_channel[y][x] + 1.14 * V_channel[y][x]
        output_image[y][x][1] = Y_channel[y][x] - 0.395 * U_channel[y][x] - 0.581 * V_channel[y][x]
        output_image[y][x][2] = Y_channel[y][x] + 2.033 * U_channel[y][x]


        if(Y_channel[y][x] + 1.14 * V_channel[y][x] < 0):
            print("YO!")
            print(Y_channel[y][x] + 1.14 * V_channel[y][x])

        #if(Y_channel[y][x] - 0.395 * U_channel[y][x] - 0.581 * V_channel[y][x]< 0):
        #    print("YO!")
        #    print(Y_channel[y][x] - 0.395 * U_channel[y][x] - 0.581 * V_channel[y][x])
        #    print(output_image[y][x][1])

        if(Y_channel[y][x] + 2.033 * U_channel[y][x]< 0):
            print("YO!")
            print(Y_channel[y][x] + 2.033 * U_channel[y][x])
            print(output_image[y][x][2])

print(np.min(Y_channel))
print(np.max(Y_channel))

print(np.min(U_channel))
print(np.max(U_channel))

print(np.min(V_channel))
print(np.max(V_channel))
#Y_channel = 
#U_channel = 
#V_channel = 


# Save the image as .png
result = cv2.imwrite(output_file_path, output_image)

# Check if the image was saved successfully
if not result:
    print("Error: Unable to save image")
    exit(1)

print(f"Image converted successfully to {output_file_path}")
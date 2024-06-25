import cv2
import argparse
import numpy as np


parser = argparse.ArgumentParser()

parser.add_argument("-i", "--input_file")

args = parser.parse_args()


# Load the .png file
input_file_path = args.input_file

scene = cv2.imread(input_file_path, cv2.IMREAD_UNCHANGED)

# Check if the image was loaded successfully
if scene is None:
    print("Error: Unable to load scene image")
    exit(1)

pixel_max = np.max(scene)
print(f"The maximum pixel value is {pixel_max}")


print(scene.shape)
for y in range(scene.shape[0]):
    for x in range(scene.shape[1]):
        print(f"R: {scene[y][x][0]}   G: {scene[y][x][1]}    B: {scene[y][x][2]}")
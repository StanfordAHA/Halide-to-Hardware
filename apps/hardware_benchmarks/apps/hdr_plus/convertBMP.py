import cv2
import argparse


parser = argparse.ArgumentParser()

parser.add_argument("-i", "--input_file")
parser.add_argument("-o", "--output_file")

args = parser.parse_args()


# Load the .bmp file
input_file_path = args.input_file
output_file_path = args.output_file

scene = cv2.imread(input_file_path, cv2.IMREAD_UNCHANGED)

# Check if the image was loaded successfully
if scene is None:
    print("Error: Unable to load scene image")
    exit(1)

# Save the image as .png
result = cv2.imwrite(output_file_path, scene)

# Check if the image was saved successfully
if not result:
    print("Error: Unable to save image")
    exit(1)

print(f"Image converted successfully to {output_file_path}")
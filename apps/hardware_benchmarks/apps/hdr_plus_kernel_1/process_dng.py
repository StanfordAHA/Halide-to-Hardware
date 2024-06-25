import rawpy
import numpy as np
import imageio
import matplotlib.pyplot as plt

# Open the DNG file
#raw = rawpy.imread('images/dng/20171106_subset_bursts_0039_20141006_110442_472_payload_N000.dng')
raw = rawpy.imread('taxi_merged.dng')

# Get the Bayer raw data
bayer_raw = raw.raw_image

out_file = open("dng_out.txt", "w")
for y in range(bayer_raw.shape[0]):
    for x in range(bayer_raw.shape[1]):
        out_file.write(str(bayer_raw[y][x]))
        
        if x != bayer_raw.shape[1]-1:
            out_file.write(", ")
    
    if y != bayer_raw.shape[0]-1:
        out_file.write("\n")


# bayer_raw is a NumPy array containing the raw sensor data
print(bayer_raw.shape)  # Shape of the array
print(bayer_raw.dtype)
print(bayer_raw[0][0])
print(np.max(bayer_raw))
print(np.min(bayer_raw))

#print(raw.raw_colors)
raw_colors = raw.raw_colors
print(raw.color_matrix)

print(raw.black_level_per_channel)
print(raw.camera_whitebalance)
print(raw.daylight_whitebalance)

print("BEFORE XYZ MATRIX")
print(raw.rgb_xyz_matrix)
print("AFTER XYZ MATRIX")
print(raw.white_level)
      
params = rawpy.Params(
    output_bps=16,
    use_camera_wb=True,
    no_auto_bright=True,
    auto_bright_thr=0)
rgb_image = raw.postprocess(params=params)
print("This is the shape")
print(rgb_image.shape)
print(rgb_image[0][0][0])
print(rgb_image[0][0][1])
print(rgb_image[0][0][2])
print("Above is the shape")
rgb_image = rgb_image.astype(np.float32) / (2**16 - 1)
#imageio.imsave('test.bmp', rgb_image)
plt.imshow(rgb_image)
plt.axis('off')
plt.savefig("RGB_image.png")


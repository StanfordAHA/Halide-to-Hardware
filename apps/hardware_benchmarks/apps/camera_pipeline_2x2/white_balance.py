import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from skimage import io, img_as_ubyte
from skimage.io import imread, imshow
from matplotlib.patches import Rectangle


def white_patch(image, percentile=100):
    """
    Returns a plot comparison of original and corrected/white balanced image 
    using the White Patch algorithm.

    Parameters
    ----------
    image : numpy array
            Image to process using white patch algorithm
    percentile : integer, optional
                  Percentile value to consider as channel maximum
    """
    white_patch_image = img_as_ubyte(
        (image * 1.0 / np.percentile(image, 
                                     percentile, 
                                     axis=(0, 1))).clip(0, 1))
    # Plot the comparison between the original and white patch corrected images
    fig, ax = plt.subplots(1, 2, figsize=(10, 10))
    ax[0].imshow(image)
    ax[0].set_title('Original Image')
    ax[0].axis('off')

    ax[1].imshow(white_patch_image, cmap='gray')
    ax[1].set_title('White Patch Corrected Image')
    ax[1].axis('off')

    plt.savefig("white_patch_corrected.png")
    #plt.show()

def gray_world(image):
    """
    Returns a plot comparison of original and corrected/white balanced image 
    using the Gray World algorithm.

    Parameters
    ----------
    image : numpy array
            Image to process using gray world algorithm
    """
    # Apply the Gray World algorithm
    image_grayworld = ((image * (image.mean() / image.mean(axis=(0, 1)))).clip(0, 255).astype(int))

    # Exclude alpha or opacity channel (transparency)
    if image.shape[2] == 4:
        image_grayworld[:, :, 3] = 255

    # Plot the comparison between the original and gray world corrected images
    fig, ax = plt.subplots(1, 2, figsize=(14, 10))
    ax[0].imshow(image)
    ax[0].set_title('Original Image')
    ax[0].axis('off')

    ax[1].imshow(image_grayworld)
    ax[1].set_title('Gray World Corrected Image')
    ax[1].axis('off')

    plt.savefig("gray_world_corrected.png")


# Read the input image
image = imread('../hdr_plus/images/pgm/giraffe/20171106_subset_bursts_0039_20141006_110442_472_payload_N000.pgm')
image = imread('bin/output_cpu.png')

# Call the function to implement white patch algorithm
white_patch(image, 95)
#gray_world(image)
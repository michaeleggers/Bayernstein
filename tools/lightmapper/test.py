# Python packages
import numpy as np
import cv2
import math
from pathlib import Path
from tqdm import tqdm               # for progress bar
from scipy.spatial import KDTree    # for illegal pixel neares neighbour search
import random


# Renderer
from renderer import Renderer
# Data structures
from data_structures.scene import Scene
from data_structures.vector3f import Vector3f
from data_structures.triangle import Triangle
# Utility functions
import util.hemicube as hemicube
import util.geometry as geometry


def save_lightmap_as_png_preserving_range(lightmap_path: Path):
        # Load the HDR image with floating-point precision
        hdr_image = cv2.imread(str(lightmap_path), cv2.IMREAD_UNCHANGED)

        if hdr_image is None:
            print(f"Failed to load image from {str(lightmap_path)}")
            return

        # Check if it's loaded in BGR format and convert to RGB
        if hdr_image.shape[2] == 3:  # Only convert if it's a 3-channel image
            hdr_image = cv2.cvtColor(hdr_image, cv2.COLOR_BGR2RGB)

        # Calculate the maximum value in the HDR image
        hdr_max = np.max(hdr_image)
        if hdr_max == 0:
            print("HDR image is empty or contains only black pixels.")
            return

        # Scale the HDR image to fit within SDR range [0, 1] non-linearly
        scaling_factor = hdr_max
        scaled_hdr_image = hdr_image / scaling_factor

        # Apply a non-linear compression (logarithmic mapping)
        compressed_image = np.log1p(scaled_hdr_image) / np.log1p(1.0)  # Compress into SDR range

        # Gamma correction (assume 2.2 gamma)
        gamma_corrected_image = np.power(compressed_image, 1.0 / 2.2)

        # Scale the gamma-corrected image to 0-255 range and convert to 8-bit
        png_image = np.clip(gamma_corrected_image * 255, 0, 255).astype(np.uint8)

        # Convert back to BGR format for saving as PNG
        png_image_bgr = cv2.cvtColor(png_image, cv2.COLOR_RGB2BGR)

        # Save the PNG image
        lightmap_filename = lightmap_path.stem
        output_path = lightmap_path.parent / f'{lightmap_filename}_scaled.png'
        cv2.imwrite(str(output_path), png_image_bgr)

        # Print the scaling factor for remapping
        print(f"Scaling factor: {scaling_factor}")
        print(f"The original HDR values can be approximated by scaling the SDR image by {scaling_factor} and applying inverse gamma correction.")

save_lightmap_as_png_preserving_range(Path('D:/data/Informatik/GamesEngineering/Bayernstein/assets/compiled/Milestone4/Milestone4.hdr'))
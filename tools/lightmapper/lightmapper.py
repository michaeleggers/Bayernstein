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

class Lightmapper:

    def __init__(self, scene: Scene, renderer: Renderer):

        self.scene = scene
        self.renderer = renderer
        self.base_path = Path(__file__).resolve().parent


    def generate_lightmap(self, lightmap_path: Path, iterations=5, patch_resolution=0.0625):
        lightmap_size_pixels = self.scene.light_map_resolution
        correction_map = hemicube.generate_correction_map(self.renderer.viewport_size)  # Precompute once
        light_map = self.scene.light_map

        for iteration in range(iterations):
            new_lightmap = light_map.copy()
            self.all_positions = []
            self.all_normals = []

            actual_sum_patches = 0
            theoretical_sum_patches = 0

            for frame_index, frame in enumerate(tqdm(self.scene.frames, desc="Rendering Iteration")):
                # Initialize arrays for the frame
                frame_light_map = np.full((frame.frame_height_pixels, frame.frame_width_pixels, 3), 0.0, dtype=np.float32)
                frame_processed_map = np.zeros((frame.frame_height_pixels, frame.frame_width_pixels), dtype=bool)
                legality_map = frame.frameArrayLegality

                # Generate views for this frame only once
                frame_views = hemicube.generate_hemicube_views(frame.frame_normal)

                for adaptive_step in range(20):
                    # Precompute lightmap statistics
                    valid_pixels = frame_light_map[frame_processed_map]
                    if valid_pixels.size > 0:
                        mean_value = np.mean(valid_pixels)
                    else:
                        mean_value = 1e-6  # Avoid divide-by-zero
                    factor = 2

                    # Collect legal pixels and their properties
                    processed_indices = np.argwhere(frame_processed_map)
                    kdtree = KDTree(processed_indices) if len(processed_indices) > 0 else None
                    legal_pixels, positions, directions, up_vectors, frustums = [], [], [], [], []

                    for i, j in zip(*np.where(legality_map & ~frame_processed_map)):
                        # Calculate probability using gradients if neighbors exist
                        if kdtree is not None:
                            distances, indices = kdtree.query((i, j), k=min(6, len(processed_indices)))
                            neighbor_values = frame_light_map[processed_indices[indices, 0], processed_indices[indices, 1]]
                            gradient = abs(np.std(neighbor_values))
                            normalized_gradient = gradient / max(mean_value, 1e-6)
                            probability = min(normalized_gradient * factor, 1.0)
                        else:
                            probability = np.random.random() if len(legal_pixels) > 0 else 1

                        if probability > 0.90:
                            legal_pixels.append((i, j))
                            for view in frame_views:
                                directions.append(view['direction'])
                                up_vectors.append(view['up_vector'])
                                frustums.append(view['frustum'])
                                positions.append(frame.frameArrayPositions[i, j])

                    # Render in batches
                    if positions:
                        images = self.renderer.render_batch_images(positions, directions, up_vectors)
                        for i, (x, y) in enumerate(legal_pixels):
                            batch_index = i * len(frame_views)
                            cut_images = [
                                hemicube.cut_image_with_frustum(images[batch_index + j], frustums[j])
                                for j in range(len(frame_views))
                            ]

                            hc = hemicube.merge_views_hemicube(*cut_images) * correction_map
                            sum_rgb = np.sum(hc, axis=(0, 1)) + frame.frameArrayIncommingLight[x, y]
                            frame_light_map[x, y] = sum_rgb
                            frame_processed_map[x, y] = True
                    else:
                        break

                # Interpolate unprocessed pixels
                legal_processed_pixels = np.argwhere(frame_processed_map & legality_map)
                if len(legal_processed_pixels) > 0:
                    kdtree = KDTree(legal_processed_pixels)
                    unprocessed_indices = np.argwhere(~frame_processed_map & legality_map)

                    for x, y in unprocessed_indices:
                        distances, indices = kdtree.query((x, y), k=min(4, len(legal_processed_pixels)))
                        # Ensure indices is always a 1D array
                        if np.isscalar(indices):
                            indices = np.array([indices])
                            distances = np.array([distances])

                        # Normalize weights
                        weights = 1 / (distances + 1e-6)
                        weights /= np.sum(weights)

                        # Get neighbor coordinates
                        neighbors = legal_processed_pixels[indices]
                        
                        # Interpolate value
                        frame_light_map[x, y] = np.sum(frame_light_map[neighbors[:, 0], neighbors[:, 1]] * weights[:, None], axis=0)

                # Interpolate illegal pixels (nearest neighbor)
                illegal_pixels = np.argwhere(~legality_map)
                if len(illegal_pixels) > 0 and len(legal_processed_pixels) > 0:
                    kdtree = KDTree(np.argwhere(legality_map))
                    for x, y in illegal_pixels:
                        _, nearest_index = kdtree.query((x, y))
                        nearest_pixel = np.argwhere(legality_map)[nearest_index]
                        frame_light_map[x, y] = frame_light_map[nearest_pixel[0], nearest_pixel[1]]

                # Update global lightmap
                new_lightmap[frame.frame_v_start:frame.frame_v_end, frame.frame_u_start:frame.frame_u_end, :] = frame_light_map
                actual_sum_patches += len(legal_processed_pixels)
                theoretical_sum_patches += np.sum(legality_map)

            self.scene.light_map = new_lightmap
            self.renderer.update_light_map()

        self.scene.generate_light_map(lightmap_path)
        self.save_lightmap_as_png_with_exposure(lightmap_path, 1000.0)
        #self.save_lightmap_as_png_preserving_range(lightmap_path)

        print("Theoretical sum patches:", theoretical_sum_patches)
        print("Actual sum patches:", actual_sum_patches)


    def save_lightmap_as_png_with_exposure(self, lightmap_path: Path, exposure: float):
        # Load the HDR image with floating-point precision
        hdr_image = cv2.imread(str(lightmap_path), cv2.IMREAD_UNCHANGED)

        if hdr_image is None:
            print(f"Failed to load image from {str(lightmap_path)}")
            return

        # Check if it's loaded in BGR format and convert to RGB
        if hdr_image.shape[2] == 3:  # Only convert if it's a 3-channel image
            hdr_image = cv2.cvtColor(hdr_image, cv2.COLOR_BGR2RGB)

        # Apply the exposure-based tone mapping
        tone_mapped_image = 1.0 - np.exp(-hdr_image * exposure)

        # Gamma correction (assume 2.2 gamma)
        tone_mapped_image = np.power(tone_mapped_image, 1.0 / 2.2)

        # Scale the tone-mapped image back to 0-255 range and convert to 8-bit
        png_image = np.clip(tone_mapped_image * 255, 0, 255).astype(np.uint8)

        # Convert back to BGR format for saving as PNG
        png_image_bgr = cv2.cvtColor(png_image, cv2.COLOR_RGB2BGR)

        # Save the PNG image
        lightmap_filename = lightmap_path.stem
        output_path = lightmap_path.parent / f'{lightmap_filename}.png'
        cv2.imwrite(str(output_path), png_image_bgr)


    def save_lightmap_as_png_preserving_range(self, lightmap_path: Path):
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


    def quit(self):
        self.renderer.destroy()
        pass


if __name__ == '__main__':

    lightmapper = Lightmapper()
    
    lightmapper.generate_lightmap()
    lightmapper.quit()
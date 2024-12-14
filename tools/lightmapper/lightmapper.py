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


    def generate_lightmap(self,  lightmap_path: Path, iterations=5, patch_resolution=0.0625):

        lightmap_size_pixels = self.scene.light_map_resolution
        correction_map = hemicube.generate_correction_map(self.renderer.viewport_size)
        light_map = self.scene.light_map
        
        for iteration in range(iterations):

            new_lightmap = light_map.copy()
            self.all_positions = []
            self.all_normals = []

            for frame_index, frame in enumerate(tqdm(self.scene.frames, desc="Rendering Iteration")):
                frame_light_map = np.zeros((frame.frame_height_pixels, frame.frame_width_pixels, 3), dtype=np.float32)

                # Batch legal pixels for rendering
                positions = []
                directions = []
                up_vectors = []
                frustums = []

                # Collect all views for batching
                for i, normal in enumerate(frame.legal_normals):
                    views = hemicube.generate_hemicube_views(normal)
                    for view in views:
                        directions.append(view['direction'])
                        up_vectors.append(view['up_vector'])
                        frustums.append(view['frustum'])
                        positions.append(frame.legal_positions[i])
                    
                    self.all_positions.append(frame.legal_positions[i])
                    self.all_normals.append(views[0]['direction'])

                # Render all views in a single batch
                images = self.renderer.render_batch_images(positions, directions, up_vectors)

                # Process each legal pixel
                for i, legal_pixel in enumerate(frame.legal_pixels):
                    batch_index = i * len(views)
                    cut_images = [
                        hemicube.cut_image_with_frustum(images[batch_index + j], frustums[batch_index + j])
                        for j in range(len(views))
                    ]

                    hc = hemicube.merge_views_hemicube(*cut_images)
                    hc_corrected = hc * correction_map

                    sum_r = np.sum(hc_corrected[:, :, 0])  # Sum of the Red channel
                    sum_g = np.sum(hc_corrected[:, :, 1])  # Sum of the Green channel
                    sum_b = np.sum(hc_corrected[:, :, 2])  # Sum of the Blue channel

                    frame_light_map[legal_pixel[1], legal_pixel[0], 0] = sum_r
                    frame_light_map[legal_pixel[1], legal_pixel[0], 1] = sum_g
                    frame_light_map[legal_pixel[1], legal_pixel[0], 2] = sum_b

                if len(frame.legal_pixels) > 0:
                    # Set illegal pixels to the color of the nearest legal neighbor
                    frame_light_map_copy = frame_light_map.copy()
                    kdtree = KDTree(frame.legal_pixels)
                    valid_colors = np.array([frame_light_map_copy[v, u] for u, v in frame.legal_pixels])
                    for u_pixel, v_pixel in frame.illegal_pixels:
                        _, idx = kdtree.query((u_pixel, v_pixel))
                        frame_light_map[v_pixel, u_pixel, :] = valid_colors[idx]

                new_lightmap[frame.frame_v_start:frame.frame_v_end, frame.frame_u_start:frame.frame_u_end, :] = frame_light_map
                    

            self.scene.light_map = new_lightmap
            self.renderer.update_light_map()

        self.scene.generate_light_map(lightmap_path)
        self.save_lightmap_as_png_with_exposure(lightmap_path, 1000.0)

 



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

    def quit(self):
        self.renderer.destroy()
        pass


if __name__ == '__main__':

    lightmapper = Lightmapper()
    
    lightmapper.generate_lightmap()
    lightmapper.quit()
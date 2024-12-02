from pathlib import Path
import matplotlib.pyplot as plt
import numpy as np
import cv2
from tqdm import tqdm
from PIL import Image
from typing import List, Optional, Tuple
import math


from renderer import Renderer
from data_structures.scene import Scene
from data_structures.color import Color
from data_structures.vector3f import Vector3f
from data_structures.triangle import Triangle
import util.hemicube as hemicube


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

            for frame_index, frame in enumerate(tqdm(self.scene.frames, desc="Processing frames")):

                frame_width_ws = frame.bounding_box[2]
                frame_height_ws = frame.bounding_box[3]
                frame_origin_u_pixels = math.ceil(frame.bounding_box[0] * patch_resolution)
                frame_origin_v_pixels = math.ceil(frame.bounding_box[1] * patch_resolution)
                frame_width_pixels = math.ceil(frame_width_ws * patch_resolution)
                frame_height_pixels = math.ceil(frame_height_ws * patch_resolution)

                frame_light_map = np.zeros((frame_height_pixels, frame_width_pixels, 3), dtype=np.float32)

                total_iterations = frame_width_pixels * frame_height_pixels
                for v_pixel in range(frame_height_pixels):
                    for u_pixel in range(frame_width_pixels):
                        u = u_pixel / (frame_width_pixels -1)
                        if u_pixel != 0 and v_pixel != 0 and u_pixel != frame_height_pixels-1 and v_pixel != frame_height_pixels -1:
                            v = v_pixel / (frame_height_pixels -1)
                            views = hemicube.generate_hemicube_views(frame.frame_normal)
                            worldspace_position = self.interpolate_uv_to_world(
                                frame.triangles[0],
                                frame.lm_uvs_bbox_space[0],
                                (u, v)
                            )

                            images = []
                            for view in views:
                                
                                image_array = self.renderer.render_single_image(worldspace_position, view['direction'], view['up_vector'])
                                cut_image = hemicube.cut_image_with_frustum(image_array, view['frustum'])
                                images.append(cut_image)
                            hc = hemicube.merge_views_hemicube(images[0], images[1], images[2], images[3], images[4])
                            hc_corrected = hc * correction_map

                            sum_r = np.sum(hc_corrected[:, :, 0])  # Sum of the Red channel
                            sum_g = np.sum(hc_corrected[:, :, 1])  # Sum of the Green channel
                            sum_b = np.sum(hc_corrected[:, :, 2])  # Sum of the Blue channel
                            
                            frame_light_map[v_pixel, u_pixel, 0] = sum_r
                            frame_light_map[v_pixel, u_pixel, 1] = sum_g
                            frame_light_map[v_pixel, u_pixel, 2] = sum_b

                
                frame_u_start = int(frame_origin_u_pixels)
                frame_v_start = int(frame_origin_v_pixels)

                frame_u_end = frame_u_start + int(frame_width_pixels)
                frame_v_end = frame_v_start + int(frame_height_pixels)

                new_lightmap[frame_v_start:frame_v_end, frame_u_start:frame_u_end, :] = frame_light_map

            self.scene.light_map = new_lightmap
            self.renderer.update_light_map()

        self.scene.generate_light_map(lightmap_path)
        self.save_lightmap_as_png(lightmap_path)


        pass

    def calculate_barycentric(self, uv_coords: np.ndarray, uv_point: np.ndarray) -> np.ndarray:
        """
        Calculate barycentric coordinates for a UV point within a UV triangle.

        Args:
            uv_coords (np.ndarray): UV coordinates of the triangle.
            uv_point (np.ndarray): UV coordinates of the point.

        Returns:
            np.ndarray: Barycentric coordinates (w0, w1, w2).
        """
        u0, v0 = uv_coords[0]
        u1, v1 = uv_coords[1]
        u2, v2 = uv_coords[2]
        u_p, v_p = uv_point

        # Compute the area of the full triangle in UV space
        denom = (v1 - v2) * (u0 - u2) + (u2 - u1) * (v0 - v2)
        if denom == 0:
            raise ValueError("The triangle's UV coordinates are degenerate (zero area).")

        # Barycentric coordinates for uv_point
        w0 = ((v1 - v2) * (u_p - u2) + (u2 - u1) * (v_p - v2)) / denom
        w1 = ((v2 - v0) * (u_p - u2) + (u0 - u2) * (v_p - v2)) / denom
        w2 = 1 - w0 - w1

        return np.array([w0, w1, w2])

    def interpolate_uv_to_world(self, triangle: Triangle, uv_coords: np.ndarray, uv_point: tuple[float, float]) -> tuple[np.ndarray, np.ndarray]:
        """
        Interpolate world coordinates and normal at a given UV point within a triangle.

        Args:
            vertices: Vertices of the triangle in world space.
            uv_coords: UV coordinates of the triangle.
            uv_point (tuple[float, float]): UV coordinates of the point to interpolate.
        """
        vertices = np.array([(vertex.x, vertex.y, vertex.z) for vertex in triangle.vertices])
        uv_coords = np.array(uv_coords)
        uv_point = np.array(uv_point)

        bary_coords = self.calculate_barycentric(uv_coords, uv_point)

        # Use barycentric coordinates to interpolate the world space position
        world_position = bary_coords[0] * vertices[0] + bary_coords[1] * vertices[1] + bary_coords[2] * vertices[2]

        return Vector3f(world_position[0], world_position[1], world_position[2])

    def generate_lightmap2(self,  lightmap_path: Path, iterations=5):

        

        for iteration in range(iterations):
            a = 0
            correction_map = hemicube.generate_correction_map(self.renderer.viewport_size)

            new_lightmap = self.scene.light_map.copy()
            for i, patch in tqdm(enumerate(self.scene.patches), total=len(self.scene.patches), desc=f'Iteration {iteration + 1}/{iterations}'):
                
                if patch.is_emissive == False:

                    position = patch.worldspace_coord
                    normal = patch.normal
                    views = hemicube.generate_hemicube_views(normal)

                    images = []
                    for view in views:
                        
                        image_array = self.renderer.render_single_image(position, view['direction'], view['up_vector'])
                        cut_image = hemicube.cut_image_with_frustum(image_array, view['frustum'])
                        images.append(cut_image)
                    hc = hemicube.merge_views_hemicube(images[0], images[1], images[2], images[3], images[4])
                    hc_corrected = hc * correction_map

                    sum_r = np.sum(hc_corrected[:, :, 0])  # Sum of the Red channel
                    sum_g = np.sum(hc_corrected[:, :, 1])  # Sum of the Green channel
                    sum_b = np.sum(hc_corrected[:, :, 2])  # Sum of the Blue channel
                    
                    new_lightmap[patch.x_tex_coord, patch.y_tex_coord, 0] = sum_r
                    new_lightmap[patch.x_tex_coord, patch.y_tex_coord, 1] = sum_g
                    new_lightmap[patch.x_tex_coord, patch.y_tex_coord, 2] = sum_b


                    #if a < 300 and views[0]['direction'][2] == 1.0 and position.z == 0.0:
                    #    a = a+1

                    #    print(i, position, views)
                    #    # Save hemicube hc as an image
                    #    hemicube_image = (hc * 255).astype(np.uint8)  # Convert to 8-bit per channel
                    #    hemicube_image_pil = Image.fromarray(hemicube_image)
                    #    hemicube_image_pil.save(f'hemicube_{i}.png')  # Save the hemicube image

            new_lightmap = self.fill_in_illegal_pixels(new_lightmap)
            self.scene.light_map = new_lightmap
            #temporary_lightmap_path = Path(self.base_path / 'temp' / 'lightmap.hdr')
            #self.scene.generate_light_map(temporary_lightmap_path)
            self.renderer.update_light_map()
            

        self.scene.generate_light_map(lightmap_path)
        #self.print_and_convert_hdr_image(lightmap_path, "debug_lightmap.png")
        self.save_lightmap_as_png(lightmap_path)

    def save_lightmap_as_png(self, lightmap_path: Path):
        # Load the HDR image with floating-point precision
        hdr_image = cv2.imread(str(lightmap_path), cv2.IMREAD_UNCHANGED)

        if hdr_image is None:
            print(f"Failed to load image from {str(lightmap_path)}")
            return
        
        # Check if it's loaded in BGR format and convert to RGB
        if hdr_image.shape[2] == 3:  # Only convert if it's a 3-channel image
            hdr_image = cv2.cvtColor(hdr_image, cv2.COLOR_BGR2RGB)

        # Convert HDR image (float32) back to 8-bit image for PNG saving
        # We scale the float values back into the 0-255 range
        normalized_image = cv2.normalize(hdr_image, None, 0, 255, cv2.NORM_MINMAX)
        png_image = np.clip(normalized_image, 0, 255).astype(np.uint8)

        # Convert back to BGR format for saving as PNG
        png_image_bgr = cv2.cvtColor(png_image, cv2.COLOR_RGB2BGR)

        # Save the PNG image
        lightmap_filename = lightmap_path.stem
        output_path = lightmap_path.parent / f'{lightmap_filename}.png'
        cv2.imwrite(str(output_path), png_image_bgr)

    def fill_in_illegal_pixels(self, lightmap):
        for illegal_pixel in self.scene.illegal_pixels:
            lightmap[illegal_pixel[0], illegal_pixel[1]] = lightmap[illegal_pixel[2], illegal_pixel[3]]
        return lightmap

    def print_and_convert_hdr_image(self, filepath: Path, output_png: str, sample_size: int = 5) -> None:
        # Load the HDR image with floating-point precision
        hdr_image = cv2.imread(str(filepath), cv2.IMREAD_UNCHANGED)
        
        if hdr_image is None:
            print(f"Failed to load image from {str(filepath)}")
            return

        # Check if it's loaded in BGR format and convert to RGB
        if hdr_image.shape[2] == 3:  # Only convert if it's a 3-channel image
            hdr_image = cv2.cvtColor(hdr_image, cv2.COLOR_BGR2RGB)
        
        # Print image dimensions and data type
        print(f"Image dimensions: {hdr_image.shape}")
        print(f"Image data type: {hdr_image.dtype}")
        
        # Print a sample of the pixel values for inspection
        #print(f"Sample pixel values (first {sample_size} rows and columns):")
        #print(hdr_image[:sample_size, :sample_size])

        # Optional: Print the min and max values to see the value range
        print(f"Min pixel value: {np.min(hdr_image)}")
        print(f"Max pixel value: {np.max(hdr_image)}")

        # Convert HDR image (float32) back to 8-bit image for PNG saving
        # We scale the float values back into the 0-255 range
        normalized_image = cv2.normalize(hdr_image, None, 0, 255, cv2.NORM_MINMAX)
        png_image = np.clip(normalized_image, 0, 255).astype(np.uint8)

        # Convert back to BGR format for saving as PNG
        png_image_bgr = cv2.cvtColor(png_image, cv2.COLOR_RGB2BGR)

        # Save the PNG image
        cv2.imwrite(output_png, png_image_bgr)
        print(f"Saved image as {output_png}")
        plt.imshow(hdr_image)
        plt.axis('off') 
        plt.show()

    def quit(self):
        self.renderer.destroy()
        pass


if __name__ == '__main__':

    lightmapper = Lightmapper()
    
    lightmapper.generate_lightmap()
    lightmapper.quit()
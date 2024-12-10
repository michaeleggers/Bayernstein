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
import util.geometry as geometry
from scipy.spatial import KDTree

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
                
                external_triangles = [tri for tri in self.scene.triangles_ds if tri not in frame.triangles]

                legal_pixels = []
                illegal_pixels = []



                frame_width_ws = frame.bounding_box[2]
                frame_height_ws = frame.bounding_box[3]
                frame_origin_u_pixels = math.ceil(frame.bounding_box[0] * patch_resolution)
                frame_origin_v_pixels = math.ceil(frame.bounding_box[1] * patch_resolution)
                frame_width_pixels = math.ceil(frame_width_ws * patch_resolution)
                frame_height_pixels = math.ceil(frame_height_ws * patch_resolution)

                frame_width_uvs = frame_width_pixels / lightmap_size_pixels
                frame_height_uvs = frame_height_pixels / lightmap_size_pixels

                pixel_size_u = 1 / (frame_width_pixels -1)
                pixel_size_v = 1 / (frame_height_pixels -1)
                half_pixel_size_u = pixel_size_u / 2
                half_pixel_size_v = pixel_size_v / 2

                frame_light_map = np.zeros((frame_height_pixels, frame_width_pixels, 3), dtype=np.float32)
                triangles = [triangle for triangle in frame.lm_uvs_bbox_space]

                total_iterations = frame_width_pixels * frame_height_pixels
                for v_pixel in range(frame_height_pixels):
                    v = v_pixel / (frame_height_pixels -1)
                    for u_pixel in range(frame_width_pixels):
                        u = u_pixel / (frame_width_pixels -1)

                        pixel_corner_uvs = [
                            (u - half_pixel_size_u, v - half_pixel_size_v),
                            (u - half_pixel_size_u, v + half_pixel_size_v),
                            (u + half_pixel_size_u, v + half_pixel_size_v),
                            (u + half_pixel_size_u, v - half_pixel_size_v)
                        ]
                        

                        if geometry.square_triangles_overlap(pixel_corner_uvs, triangles):
                            
                            #u_ws = u * frame_width_ws
                            #v_ws = v * frame_height_ws
                            #distance = geometry.point_to_line_distance_right_of_segment((u_ws, v_ws), frame.intersections)
                            #if distance and distance < 1/patch_resolution:
                                #print(distance, frame_width_uvs, frame_height_uvs)
                                #illegal_pixels.append((u_pixel, v_pixel))
                                #continue
                                #pass

                            worldspace_position = self.interpolate_uv_to_world(
                                frame.triangles[0],
                                frame.lm_uvs_bbox_space[0],
                                (u, v)
                            )
                            
                            # Check if patch is covered by triangle on the same plane
                            if geometry.point_is_covered_by_triangle(worldspace_position, frame.frame_normal, frame.close_triangles):
                                illegal_pixels.append((u_pixel, v_pixel))
                                continue


                            distance = geometry.distance_to_closest_triangle_facing_away(worldspace_position, frame.frame_normal, frame.intersection_segments, frame.intersection_normals)
                            #distance_to_closest_triangle = geometry.closest_plane_intersection(worldspace_position, frame.frame_normal, external_triangles)
                            #print(distance_to_closest_triangle)
                            #print(distance_to_closest_triangle, 1/patch_resolution *  5)
                            if distance < 1/patch_resolution *  2:
                                illegal_pixels.append((u_pixel, v_pixel))
                                continue

                            views = hemicube.generate_hemicube_views(frame.frame_normal)


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

                            legal_pixels.append((u_pixel, v_pixel))
                        else:
                            illegal_pixels.append((u_pixel, v_pixel))


                if len(legal_pixels) > 0:
                    # Set illegal pixels to the color of the neares legal neighbour
                    frame_light_map_copy = frame_light_map.copy()
                    kdtree = KDTree(legal_pixels)
                    valid_colors = np.array([frame_light_map_copy[v, u] for u, v in legal_pixels])
                    for u_pixel, v_pixel in illegal_pixels:
                        _, idx = kdtree.query((u_pixel, v_pixel))
                        frame_light_map[v_pixel, u_pixel, :] = valid_colors[idx]


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

    def quit(self):
        self.renderer.destroy()
        pass


if __name__ == '__main__':

    lightmapper = Lightmapper()
    
    lightmapper.generate_lightmap()
    lightmapper.quit()
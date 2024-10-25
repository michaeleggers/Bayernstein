import numpy as np
from tqdm import tqdm
import cv2
from pathlib import Path
import pyrr
import math
import json

import util.uv_mapper as uv_mapper
import util.geometry as geometry
from data_structures.entity import Entity
from data_structures.color import Color
from data_structures.patch import Patch
from data_structures.vector3f import Vector3f as vec

from typing import List



class Scene:
    """Represents a 3D scene consisting of entities, triangles, and lightmaps."""

    def __init__(self) -> None:
        self.patches: List[Patch] = [] # Type hinting for patches attribute

    def create(self, entities: list, patches_resolution: float = 0.0625) -> 'Scene':
        """
        Create the scene by generating triangles, UV mapping, and creating patches.

        Args:
            entities (list): List of entities to include in the scene.
            patches_resolution (float): Resolution of the patches for the lightmap.
        """
        # Step 1: convert vertices and indices into triangles
        self.triangles = []
        self.triangle_colors = []
        self.triangle_emissions = []
        for entity in entities:
            triangles, colors, emission = self.generate_triangles(entity)
            self.triangles.extend(triangles)
            self.triangle_colors.extend(colors)
            self.triangle_emissions.extend(emission)

        # Step 2: uv map the triangles
        self.triangle_uvs, uv_map_world_size = uv_mapper.map_triangles(self.triangles, debug=True)
        self.light_map_resolution = math.ceil(uv_map_world_size * patches_resolution)

        # Step 3: generate patches and lightmap
        self.patches, self.light_map, uvs = self.generate_patches(self.triangles, self.triangle_uvs, self.light_map_resolution, self.triangle_emissions)

        # Step 4: generate vertex array for the GPU
        self.vertex_array = self.generate_vertex_array(self.triangles, self.triangle_colors, uvs)
        self.vertex_count = len(self.vertex_array) // 8  # Each vertex is made out of x, y, z, u, v

        return self


    def load(self, scene_path: Path) -> 'Scene':
        """
        Load the scene from a file.

        Args:
            scene_path (Path): Path to the scene file.
        """
        with open(scene_path, 'r') as f:
            data = json.load(f)

        self.patches = data['patches']
        self.light_map = np.array(data['light_map'], dtype=np.float32)
        self.vertex_array = np.array(data['vertex_array'], dtype=np.float32)
        self.vertex_count = len(self.vertex_array) // 8  # Update vertex count based on loaded data

        return self

    def save(self, scene_path: Path) -> 'Scene':
        """
        Save the scene to a file.

        Args:
            scene_path (Path): Path to save the scene file.
        """
        # Serialize patches using the serialize method of the Patch class
        serialized_patches = [patch.serialize() for patch in self.patches]
        
        # Prepare data for saving
        data = {
            'patches': serialized_patches,
            'light_map': self.light_map.tolist(),  # Convert to list for JSON serialization
            'vertex_array': self.vertex_array.tolist(),  # Convert to list for JSON serialization
        }
        
        # Write data to the specified file
        with open(scene_path, 'w') as f:
            json.dump(data, f, ensure_ascii=False, indent=4)  # Add indent for pretty printing

        return self


    def generate_vertex_array(self, triangles: list[list[float]], triangle_colors, uvs: list[list[tuple[float, float]]]) -> np.ndarray:
        """
        Create a vertex array from triangles and UV coordinates.

        Args:
            triangles (list[list[float]]): List of triangles in world coordinates.
            uvs (list[list[tuple[float, float]]]): Corresponding UV coordinates for each triangle.

        Returns:
            np.ndarray: A flat array of vertex data for the GPU.
        """
        vertex_list = []
        for i, triangle in enumerate(triangles):
            triangle_color = triangle_colors[i]
            for j, vertex in enumerate(triangle):
                x, y, z = vertex
                r, g, b = triangle_color.r, triangle_color.g, triangle_color.b
                u, v = uvs[i][j]
                vertex_list.extend([x, y, z, r, g, b, u, v])

        return np.array(vertex_list, dtype=np.float32)

    def generate_patches(self, triangles: list[list[float]], uvs: list[list[tuple[float, float]]], texture_map_resolution: int, triangle_emissions: list[Color]) -> tuple[list, np.ndarray, list]:
        """
        Generate patches and a lightmap for the scene.

        Args:
            triangles (list[list[float]]): List of triangles.
            uvs (list[list[tuple[float, float]]]): UV coordinates for triangles.
            texture_map_resolution (int): Resolution of the texture map.
            triangle_emissions (list[Color]): Emission colors for each triangle.

        Returns:
            tuple: A tuple containing:
                - patches (list): List of lightmap patches.
                - light_map (np.ndarray): Lightmap array.
                - uvs (list): Original UV coordinates.
        """
        patches = []
        pixel_size_uv = (1 / texture_map_resolution)
        half_pixel_size_uv = pixel_size_uv / 2

        light_map = np.zeros((texture_map_resolution, texture_map_resolution, 3), dtype=np.float32)

        for idx in tqdm(range(texture_map_resolution**2), desc="Generating patches"):
            y = idx // texture_map_resolution
            x = idx % texture_map_resolution

            # Convert pixel coordinates to UV (normalized)
            u = y * pixel_size_uv + half_pixel_size_uv
            v = x * pixel_size_uv + half_pixel_size_uv

            # Check which triangle the pixel is inside
            for i, triangle in enumerate(triangles):
                uv_a, uv_b, uv_c = uvs[i]

                pixel_corner_uvs = [
                    (u - half_pixel_size_uv, v - half_pixel_size_uv),
                    (u - half_pixel_size_uv, v + half_pixel_size_uv),
                    (u + half_pixel_size_uv, v + half_pixel_size_uv),
                    (u + half_pixel_size_uv, v - half_pixel_size_uv)
                ]

                if geometry.square_triangle_overlap(pixel_corner_uvs, (uv_a, uv_b, uv_c)):
                    # Apply emission to lightmap
                    triangle_emission = triangle_emissions[i]
                    light_map[x, y] = [triangle_emission.r, triangle_emission.g, triangle_emission.b]
                    is_emissive = triangle_emission.sum() > 0  # Check if any emission exists

                    # Interpolate world space coordinates and normals
                    world_coords, normal = self.interpolate_uv_to_world(triangle, uvs[i], (u, v))
                    patches.append(Patch(x, y, vec(world_coords[0], world_coords[1], world_coords[2]), vec(normal[0], normal[1], normal[2]), (u, v), (uv_a, uv_b, uv_c), is_emissive))
                    break  # Exit the triangle loop once found

        return patches, light_map, uvs
    
    def generate_triangles(self, entity: Entity) -> tuple[list[list[float]], list[Color], list[Color]]:
        """
        Generate triangles, colors, and emissions from an entity.

        Args:
            entity (Entity): The entity to generate triangles from.

        Returns:
            tuple: A tuple containing:
                - triangles (list[list[float]]): List of generated triangles.
                - colors (list[tuple[float, float, float]]): Base colors for the triangles.
                - emission (list[tuple[float, float, float]]): Emission colors for the triangles.
        """
        vertices = np.array(entity.vertices)
        indices = entity.indices

        triangles = []
        colors = []
        emissions = []
        for face in indices:
            triangle = [vertices[vertex_index][[0, 2, 1]] for vertex_index in face[::-1]]  # Reverse order and reformat
            triangles.append(triangle)
            colors.append(entity.base_color)
            emissions.append(entity.emission)

        return triangles, colors, emissions
    
    def interpolate_uv_to_world(self, vertices: np.ndarray, uv_coords: np.ndarray, uv_point: tuple[float, float]) -> tuple[np.ndarray, np.ndarray]:
        """
        Interpolate world coordinates and normal at a given UV point within a triangle.

        Args:
            vertices (np.ndarray): Vertices of the triangle in world space.
            uv_coords (np.ndarray): UV coordinates of the triangle.
            uv_point (tuple[float, float]): UV coordinates of the point to interpolate.

        Returns:
            tuple: A tuple containing:
                - world_position (np.ndarray): Interpolated world position.
                - normal (np.ndarray): Normal vector of the triangle.
        """
        vertices = np.array(vertices)
        uv_coords = np.array(uv_coords)
        uv_point = np.array(uv_point)

        bary_coords = self.calculate_barycentric(uv_coords, uv_point)

        # Use barycentric coordinates to interpolate the world space position
        world_position = bary_coords[0] * vertices[0] + bary_coords[1] * vertices[1] + bary_coords[2] * vertices[2]
        normal = geometry.calculate_normal(vertices[0], vertices[1], vertices[2])

        return world_position, normal
    
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
    
    def generate_light_map(self, light_map_file_path: Path) -> None:
        """
        Generate and save the lightmap to a file.

        Args:
            light_map_file_path (Path): Path to save the lightmap image.
        """
        # Ensure the light map is in float32 format for higher precision
        light_map_float32 = self.light_map.astype(np.float32)

        # Convert from RGB to BGR for OpenCV
        light_map_bgr = cv2.cvtColor(light_map_float32, cv2.COLOR_RGB2BGR)

        # Save the image in a high-precision format (e.g., OpenEXR or HDR)
        cv2.imwrite(str(light_map_file_path), light_map_bgr)

    def get_model_transform(self) -> np.ndarray:
        """
        Get the model transformation matrix.

        Returns:
            np.ndarray: The identity transformation matrix.
        """
        return pyrr.matrix44.create_identity(dtype=np.float32)
        
    
    


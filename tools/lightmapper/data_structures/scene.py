import numpy as np
from tqdm import tqdm
from PIL import Image
import cv2
from pathlib import Path
import pyrr
import math
import json
import os
import random
from PIL import Image
from scipy.spatial import KDTree

import util.uv_mapper as uv_mapper
import util.geometry as geometry
from data_structures.color import Color
from data_structures.patch import Patch
from data_structures.vector3f import Vector3f as vec
from data_structures.compiled_vertex import CompiledVertex
from data_structures.compiled_triangle import CompiledTriangle
from data_structures.triangle import Triangle
from data_structures.frame import Frame


from typing import List



class Scene:
    """Represents a 3D scene, holds datastructures for the the lightmap calculations """

    def __init__(self, map_path, assets_path, lightmap_path = None) -> None:

        textures_directory = Path(assets_path) / 'textures'
        self.triangles, self.texture_uvs, self.lightmap_uvs, self.textures, self.emissions = self.load_from_json(map_path, assets_path)
        self.texture_array, self.texture_array_uvs, self.texture_index_mapping = self.create_texture_array(self.textures, self.texture_uvs, textures_directory)
        if lightmap_path:
            self.load_lightmap(lightmap_path)
        
        self.patches: List[Patch] = []
        self.frames: List[Frame] = []
        self.triangles_ds: List[Triangle] = []

    def load_from_json(self, json_path: Path, assets_path: Path):
        triangles = []
        texture_uvs = []
        lightmap_uvs = []
        textures = {}
        emission = []

        # Cache to store texture dimensions (width, height) for each unique texture
        texture_dimensions_cache = {}

        # Load JSON data
        with open(json_path, 'r') as file:
            data = json.load(file)

        # Loop over polygons in JSON data
        for triangle_index, triangle in enumerate(data):
            # Each polygon has a set of vertices and a texture
            triangle_vertices = []
            triangle_texture_uvs = []
            triangle_lightmap_uvs = []
            for vertex_data in triangle['vertices']:
                triangle_vertices.append((vertex_data['pos'][0], vertex_data['pos'][1], vertex_data['pos'][2]))
                triangle_texture_uvs.append(tuple(vertex_data['uv']))
                triangle_lightmap_uvs.append(tuple(vertex_data.get('uv_lightmap', (0, 0))))

            triangles.append(triangle_vertices)
            texture_uvs.append(triangle_texture_uvs)
            lightmap_uvs.append(triangle_lightmap_uvs)

            # Check if texture exists, otherwise set to "default.png"
            texture_name = triangle['textureName']
            texture_path = f"{assets_path}/textures/{texture_name}.png"
            if not Path(texture_path).exists():
                texture_name = "default"
                texture_path = f"{assets_path}/textures/{texture_name}.png"

            # Cache texture size if not already cached
            if texture_name not in texture_dimensions_cache:
                with Image.open(texture_path) as img:
                    texture_dimensions_cache[texture_name] = img.size  # (width, height)

            # Record which triangles use this texture
            if texture_name not in textures:
                textures[texture_name] = []
            textures[texture_name].append(triangle_index)

            emission.append(triangle.get('emission', 0.0))

        # Normalize UVs after loading all texture names and gathering data
        normalized_texture_uvs = []
        for texture_name, triangle_indices in textures.items():
            texture_width, texture_height = texture_dimensions_cache[texture_name]

            # Normalize UVs for each triangle that uses this texture
            for triangle_index in triangle_indices:
                normalized_triangle_uvs = []
                for u, v in texture_uvs[triangle_index]:
                    # Repeat and normalize UVs
                    u_normalized = u / texture_width # (u % texture_width) / texture_width
                    v_normalized = v / texture_width # (v % texture_height) / texture_height
                    normalized_triangle_uvs.append((u_normalized, v_normalized))
                normalized_texture_uvs.append(normalized_triangle_uvs)

        # Convert lists to numpy arrays
        triangles = np.array(triangles, dtype=np.float64)
        texture_uvs = np.array(normalized_texture_uvs, dtype=np.float32)
        lightmap_uvs = np.array(lightmap_uvs, dtype=np.float32)
        emission = np.array(emission, dtype=np.float32)

        return triangles, texture_uvs, lightmap_uvs, textures, emission

    def save_to_json(self, json_path: Path, assets_path: Path) -> 'Scene':
        """
        Save the triangle data to a JSON file.

        Parameters:
        -----------
        triangles : np.ndarray
            Array of triangle vertex positions with shape (N, 3, 3).
        texture_uvs : np.ndarray
            Array of texture UV coordinates for each vertex with shape (N, 3, 2).
        lightmap_uvs : np.ndarray
            Array of lightmap UV coordinates for each vertex with shape (N, 3, 2).
        textures : dict
            Dictionary mapping texture names to lists of triangle indices.
        emission : np.ndarray
            Array of emission values for each triangle with shape (N,).
        json_path : Path
            The path to save the JSON file to.
        """
        data = []

        # Cache texture dimensions to avoid opening the same texture multiple times
        texture_dimensions = {}

        # Loop over each triangle and reconstruct the JSON format
        for i, (triangle, tex_uv, lm_uv, emit) in enumerate(zip(self.triangles, self.texture_uvs, self.lightmap_uvs, self.emissions)):
            # Ensure numpy elements are converted to native Python types
            triangle = [[vertex[0], vertex[1], vertex[2]] for vertex in triangle]
            tex_uv = [[float(u), float(v)] for u, v in tex_uv]
            lm_uv = [[float(u), float(v)] for u, v in lm_uv]
            emit = float(emit)  # Ensure emission is a Python float

            # Find the texture name associated with this triangle
            texture_name = None
            for tex_name, tri_indices in self.textures.items():
                if i in tri_indices:
                    texture_name = tex_name
                    break

            # Get texture dimensions (width, height), open the texture only if necessary
            if texture_name not in texture_dimensions:
                texture_path = f"{assets_path}/textures/{texture_name}.png"
                with Image.open(texture_path) as img:
                    texture_dimensions[texture_name] = img.size  # (width, height)

            tex_width, tex_height = texture_dimensions[texture_name]

            # Convert UVs back to pixel space
            pixel_tex_uv = [[u * tex_width, v * tex_height] for u, v in tex_uv]

            # Create vertex dictionary with positions, texture UVs, and lightmap UVs
            vertices = [
                {"pos": triangle[j], "uv": pixel_tex_uv[j], "uv_lightmap": lm_uv[j]}
                for j in range(len(triangle))
            ]
            
            # Construct the triangle entry
            triangle_data = {
                "vertices": vertices,
                "textureName": texture_name,
                "emission": emit
            }

            data.append(triangle_data)

        # Ensure the directory exists
        json_path.parent.mkdir(parents=True, exist_ok=True)
        # Write JSON data to file
        with open(json_path, 'w') as file:
            json.dump(data, file, indent=4)

        return self
    
    def save_to_binary(self, binary_path: Path) -> 'Scene':
        compiled_triangles = []

        for i, (triangle, tex_uv, lm_uv, emit) in enumerate(zip(self.triangles, self.texture_uvs, self.lightmap_uvs, self.emissions)):
            
            triangle_positions = [[vertex[0], vertex[1], vertex[2]] for vertex in triangle]
            tex_uvs = [[float(u), float(v)] for u, v in tex_uv]
            lm_uvs = [[float(u), float(v)] for u, v in lm_uv]
            normal_array = geometry.calculate_normal(np.asarray(triangle_positions[0]),
                                                    np.asarray(triangle_positions[1]),
                                                    np.asarray(triangle_positions[2]))
            normal = (float(normal_array[0]), float(normal_array[1]), float(normal_array[2]))


            # Create 3 vertices for the current triangle
            vertices = []
            bc = [(0,1), (0,1),(0,1)]
            for j in range(3):
                vertex = CompiledVertex(
                    pos=triangle_positions[j],
                    normal=normal,
                    uv_texture=tex_uvs[j],
                    uv_lightmap=lm_uvs[j],
                    bc=bc[j],
                    color=(0,0,0)
                )
                vertices.append(vertex)

            # Create a CompiledTriangle with these 3 vertices
            compiled_triangle = CompiledTriangle(
                vertices=vertices,
                textureName='default.png',  # Example texture name, adjust as needed
                surfaceFlags=1,
                contentFlags=1
            )

            compiled_triangles.append(compiled_triangle)

        # Write to binary file
        with open(binary_path, 'wb') as f:
            for triangle in compiled_triangles:
                f.write(triangle.to_binary())

    def create_frames(self, patch_resolution: float = 0.0625) -> 'Scene':
        
        # Step 1: Create Triangle Data Structure
        triangles_ds = []
        for vertex_tuple in self.triangles:
            vertices = tuple(vec(*v) for v in vertex_tuple)
            triangles_ds.append(Triangle(vertices))
        self.triangles_ds = triangles_ds

        # Step 2: Create Frames
        self.frames, self.lightmap_uvs, uv_map_ws_size = uv_mapper.create_frames(triangles_ds, patch_resolution, debug=True)
        self.light_map_resolution = math.ceil(uv_map_ws_size * patch_resolution)
        self.light_map = np.zeros((self.light_map_resolution, self.light_map_resolution, 3), dtype=np.float32)

        # Step 3: Precalculate geometry intersections (for illegal pixel calculations)
        [frame.calculate_intersections(triangles_ds) for frame in self.frames]

    def create_texture_array(self, textures, uvs, textures_directory):
        # 1. Load all textures and find the largest resolution
        images = {}
        max_width, max_height = 0, 0
        texture_index_mapping = {}  # Dictionary to map texture names to indices

        for index, texture_name in enumerate(textures):
            path = Path(textures_directory) / f"{texture_name}.png"
            image = Image.open(path).convert("RGBA")
            images[texture_name] = image
            max_width = max(max_width, image.width)
            max_height = max(max_height, image.height)
            texture_index_mapping[texture_name] = index  # Map texture name to its index

        # 2. Resize all images to the largest resolution
        for texture_name, image in images.items():
            if image.size != (max_width, max_height):
                images[texture_name] = image.resize((max_width, max_height), Image.LANCZOS)

        # 3. Create a texture array with appropriate dimensions
        num_textures = len(images)
        texture_array = np.zeros((num_textures, max_height, max_width, 4), dtype=np.uint8)

        for index, (texture_name, image) in enumerate(images.items()):
            texture_array[index] = np.array(image)

        # 4. Adjust UVs (these remain unchanged)
        adjusted_uvs = [None] * len(uvs)  # Initialize a list to hold adjusted UVs for each triangle

        return texture_array, adjusted_uvs, texture_index_mapping

    def load_lightmap(self, lightmap_path):

        lightmap_image = cv2.imread(str(lightmap_path), cv2.IMREAD_UNCHANGED)
        # Convert from BGR to RGB if needed
        light_map = cv2.cvtColor(lightmap_image, cv2.COLOR_BGR2RGB)

        # Assign the light map to the scene
        self.light_map = light_map
    
    def generate_vertex_array(self) -> 'Scene':
        """
        Create a vertex array from triangles and UV coordinates.
        """
        vertex_list = []
        
        for i, triangle in enumerate(self.triangles):
            # Find the texture name associated with this triangle
            texture_name = None
            for key, value in self.textures.items():
                if i in value:  # If the current triangle index is in the list for this texture
                    texture_name = key
                    break
            
            texture_index = self.texture_index_mapping.get(texture_name, -1)  # Default to -1 if not found

            for j, vertex in enumerate(triangle):
                x, y, z = vertex
                u_t, v_t = self.texture_uvs[i][j]
                u_l, v_l = self.lightmap_uvs[i][j]

                vertex_list.extend([x, y, z, u_t, v_t, u_l, v_l, texture_index])

        self.vertex_array = np.array(vertex_list, dtype=np.float32)
        self.vertex_count = len(self.vertex_array) // 8  # Each vertex now includes the texture index

        return self
    
    
        return False  # No triangle was found in front of the patch=
    
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

        
    
    


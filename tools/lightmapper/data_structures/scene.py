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
from data_structures.shape import Shape


from typing import List



class Scene:
    """Represents a 3D scene consisting of entities, triangles, and lightmaps."""

    def __init__(self, map_path, assets_path, lightmap_path = None) -> None:

        textures_directory = Path(assets_path) / 'textures'
        self.triangles, self.texture_uvs, self.lightmap_uvs, self.textures, self.emissions = self.load_from_json(map_path, assets_path)
        self.texture_array, self.texture_array_uvs, self.texture_index_mapping = self.create_texture_array(self.textures, self.texture_uvs, textures_directory)
        if lightmap_path:
            self.load_lightmap(lightmap_path)
        
        self.patches: List[Patch] = []
        self.frames: List[Shape] = []

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
            for j in range(3):
                vertex = CompiledVertex(
                    pos=triangle_positions[j],
                    normal=normal,
                    uv_texture=tex_uvs[j],
                    uv_lightmap=lm_uvs[j]
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
        
        # Step 1: Create Triangles Data Structure
        triangles_ds = []
        for vertex_tuple in self.triangles:
            vertices = tuple(vec(*v) for v in vertex_tuple)
            triangles_ds.append(Triangle(vertices))

        # Step 2: Create Frames
        self.frames, self.lightmap_uvs, uv_map_ws_size = uv_mapper.create_frames(triangles_ds, patch_resolution, debug=True)
        self.light_map_resolution = math.ceil(uv_map_ws_size * patch_resolution)
        self.light_map = np.zeros((self.light_map_resolution, self.light_map_resolution, 3), dtype=np.float32)


    def create_patches(self, patches_resolution: float = 0.0625) -> 'Scene':

        triangles_ds = []
        for vertex_tuple in self.triangles:
            vertices = tuple(vec(*v) for v in vertex_tuple)
            triangles_ds.append(Triangle(vertices))

        self.lightmap_uvs, uv_map_world_size = uv_mapper.map_triangles(triangles_ds, patches_resolution, debug=True)
        self.light_map_resolution = math.ceil(uv_map_world_size * patches_resolution)

        self.patches, self.light_map, self.illegal_pixels = self.generate_patches(self.triangles, self.lightmap_uvs, self.light_map_resolution, self.emissions)

        return self
    
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
        self.illegal_pixels = np.array(data['illegal_pixels'])

        return self
    
    def load_lightmap(self, lightmap_path):

        lightmap_image = cv2.imread(str(lightmap_path), cv2.IMREAD_UNCHANGED)
        # Convert from BGR to RGB if needed
        light_map = cv2.cvtColor(lightmap_image, cv2.COLOR_BGR2RGB)

        # Assign the light map to the scene
        self.light_map = light_map
    
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
            'illegal_pixels': self.illegal_pixels.tolist(),
        }
        
        # Write data to the specified file
        with open(scene_path, 'w') as f:
            json.dump(data, f, ensure_ascii=False, indent=4)  # Add indent for pretty printing

        return self

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
    
    def generate_patches(self, triangles: list[list[float]], uvs: list[list[tuple[float, float]]], texture_map_resolution: int, triangle_emissions: list[Color]):
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
        illegal_pixels_list = []

        for idx in tqdm(range(texture_map_resolution**2), desc="Generating patches"):
            y = idx // texture_map_resolution
            x = idx % texture_map_resolution

            # Convert pixel coordinates to UV (normalized)
            u = y * pixel_size_uv + half_pixel_size_uv
            v = x * pixel_size_uv + half_pixel_size_uv

            # Check which triangle the pixel is inside
            pixel_is_within_triangle = False
            for i, triangle in enumerate(triangles):
                uv_a, uv_b, uv_c = uvs[i]

                pixel_corner_uvs = [
                    (u - half_pixel_size_uv, v - half_pixel_size_uv),
                    (u - half_pixel_size_uv, v + half_pixel_size_uv),
                    (u + half_pixel_size_uv, v + half_pixel_size_uv),
                    (u + half_pixel_size_uv, v - half_pixel_size_uv)
                ]

                if geometry.square_triangle_overlap(pixel_corner_uvs, (uv_a, uv_b, uv_c)):
                #if geometry.is_point_in_triangle(np.array((u, v)), np.array(uv_a), np.array(uv_b), np.array(uv_c)):
                    # Apply emission to lightmap
                    triangle_emission = triangle_emissions[i]
                    light_map[x, y] = [triangle_emission, triangle_emission, triangle_emission]
                    is_emissive = triangle_emission.sum() > 0  # Check if any emission exists

                    # Interpolate world space coordinates and normals
                    world_coords, normal = self.interpolate_uv_to_world(triangle, uvs[i], (u, v))
                    if self.is_patch_in_front_of_triangle(world_coords, normal, triangles, 0.001):
                        break
                    
                    pixel_is_within_triangle = True
                    new_patch = Patch(x, y, vec(world_coords[0], world_coords[1], world_coords[2]), vec(normal[0], normal[1], normal[2]), (u, v), (uv_a, uv_b, uv_c), is_emissive)
                    patches.append(new_patch)
                    break  # Exit the triangle loop once found
            
            if pixel_is_within_triangle == False:
                # this pixel is not within a triangle => illegal pixel
                illegal_pixels_list.append((x, y))

        illegal_pixels_map = self.precalculate_illegal_pixels(np.array(illegal_pixels_list), patches)

        self.__debug_patch_mapping(patches, texture_map_resolution)
        return patches, light_map, illegal_pixels_map
    
    def is_patch_in_front_of_triangle(self, patch_point: np.ndarray, patch_normal: np.ndarray, triangles: List[List[tuple[float, float, float]]], epsilon=1e-5) -> bool:
        
        for triangle in triangles:
            # Get triangle vertices
            p0, p1, p2 = [np.array(vertex) for vertex in triangle]
            
            # Calculate the triangle's normal vector
            triangle_normal = geometry.calculate_normal(p0, p1, p2)

        
            # Check if the patch lies on the plane of the triangle
            # Plane equation: (point - p1) • normal = 0
            distance_to_plane = np.dot(patch_point - p0, triangle_normal)
            
            if abs(distance_to_plane) <= epsilon:
                # Check if the triangle's normal points in the opposite direction to the patch's normal
                if np.dot(triangle_normal, patch_normal) < 0:
                    # Check if the patch lies within the triangle using barycentric coordinates
                    if geometry.is_point_in_triangle(patch_point, p0, p1, p2):
                        return True  # Patch is within the triangle and in front of it

        return False  # No triangle was found in front of the patch
    
    def precalculate_illegal_pixels(self, illegal_pixels: List[tuple[int, int]], patches: List[Patch]) -> np.ndarray:
        # Create an array of patch coordinates
        patch_coords = np.array([(patch.x_tex_coord, patch.y_tex_coord) for patch in patches])
        
        # Create a KDTree for the patch coordinates for efficient nearest neighbor lookup
        kdtree = KDTree(patch_coords)

        # Prepare result array
        result = []

        # Process each illegal pixel
        for x1, y1 in tqdm(illegal_pixels, desc="Precalculating illegal pixels", unit="pixel"):
            # Query the KDTree for the nearest patch
            distance, closest_idx = kdtree.query((x1, y1))
            
            # Get the closest patch coordinates and color
            x2, y2 = patch_coords[closest_idx]

            # Append the result as (x1, y1, x2, y2, R, G, B)
            result.append((x1, y1, x2, y2))
    
        return np.array(result)
    
    
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

    def __debug_patch_mapping(self, patches, image_size=148):
        """Fills in pixels with patches onto an image with random colors and saves it."""

        def generate_random_color():
            """Generate a random RGB color."""
            return (random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))
        
        # Create a blank image
        image = Image.new("RGB", (image_size, image_size), (255, 255, 255))
        pixels = image.load()
        
        for patch in patches:
            x, y = patch.x_tex_coord, patch.y_tex_coord
            random_color = generate_random_color()
            
            # Check if the patch coordinates are within the image bounds
            if 0 <= x < image_size and 0 <= y < image_size:
                color = random_color
                pixels[x, y] = color  # Set pixel color

        # Define the path for the debug folder relative to the current script's location
        debug_path = Path(__file__).resolve().parent / "debug" 

        # Ensure the debug directory exists
        debug_path.mkdir(parents=True, exist_ok=True)

        # Save the image
        image.save(debug_path / "debug_uv_maps.png")

        
    
    


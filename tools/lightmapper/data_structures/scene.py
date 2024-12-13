# Python packages
import numpy as np
import cv2
import pyrr
import math
import json
from PIL import Image
from tqdm import tqdm
from PIL import Image
from pathlib import Path
from typing import List

# Data structures
from data_structures.patch import Patch
from data_structures.vector3f import Vector3f as vec
from data_structures.compiled_vertex import CompiledVertex
from data_structures.compiled_triangle import CompiledTriangle
from data_structures.triangle import Triangle
from data_structures.frame import Frame

# Utility functions
import util.uv_mapper as uv_mapper
import util.geometry as geometry


class Scene:
    """Represents a 3D scene, holds datastructures for the the lightmap calculations """

    def __init__(self, map_path, assets_path, lightmap_path = None) -> None:

        # Load the map from JSON
        textures_directory = Path(assets_path) / 'textures'
        self.triangles, self.texture_uvs, self.lightmap_uvs, self.textures, self.emissions = self.load_from_json(map_path, assets_path)
        # Create the Texture Array using the geometries texture informations
        self.texture_array, self.texture_index_mapping = self.create_texture_array(self.textures, self.texture_uvs, textures_directory)
        # If a lightmap exists (which may be the case when testing): load it
        if lightmap_path:
            self.load_lightmap(lightmap_path)
        
        # A frame is a collection of (for now planar) triangles, usually forming a quad
        self.frames: List[Frame] = []
        # This holds all the scene's triangles as a Triangle data structure
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
            texture_path = f"{assets_path}/textures/{texture_name}.tga"
            if not Path(texture_path).exists():                
                print(f'(load_from_json): Cannot load texture path: ${texture_path}')
                texture_name = "default"
                texture_path = f"{assets_path}/textures/{texture_name}.tga"

            # Cache texture size if not already cached
            if texture_name not in texture_dimensions_cache:
                with Image.open(texture_path) as img:
                    texture_dimensions_cache[texture_name] = img.size  # (width, height)

            # Record which triangles use this texture
            if texture_name not in textures:
                textures[texture_name] = []
            textures[texture_name].append(triangle_index)

            emission.append(triangle.get('emission', 0.0))

        # Convert lists to numpy arrays
        triangles = np.array(triangles, dtype=np.float64)
        texture_uvs = np.array(texture_uvs, dtype=np.float32)
        lightmap_uvs = np.array(lightmap_uvs, dtype=np.float32)
        emission = np.array(emission, dtype=np.float32)

        return triangles, texture_uvs, lightmap_uvs, textures, emission

    def save_to_json(self, json_path: Path, assets_path: Path) -> 'Scene':
        """
        Save the triangle data to a JSON file.
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
                texture_path = f"{assets_path}/textures/{texture_name}.tga"
                with Image.open(texture_path) as img:
                    texture_dimensions[texture_name] = img.size  # (width, height)

            # Convert UVs back to pixel space
            pixel_tex_uv = [[u, v] for u, v in tex_uv]

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
        """
        Save the triangle data to a binary file.
        """

        compiled_triangles = []

        for i, (triangle, tex_uv, lm_uv, emit) in enumerate(zip(self.triangles, self.texture_uvs, self.lightmap_uvs, self.emissions)):
            
            triangle_positions = [[vertex[0], vertex[1], vertex[2]] for vertex in triangle]
            tex_uvs = [[float(u), float(v)] for u, v in tex_uv]
            lm_uvs = [[float(u), float(v)] for u, v in lm_uv]
            normal_array = geometry.calculate_normal(np.asarray(triangle_positions[0]), np.asarray(triangle_positions[1]), np.asarray(triangle_positions[2]))
            normal = (float(normal_array[0]), float(normal_array[1]), float(normal_array[2]))

            # Create 3 vertices for the current triangle
            vertices = []
            # These barycentric coordinates are placeholders
            bc = [[1.0,0.0, 0.0], [0.0,1.0, 0.0],[0.0, 0.0, 1.0]]
            for j in range(3):
                vertex = CompiledVertex(
                    pos=triangle_positions[j],
                    normal=normal,
                    uv_texture=tex_uvs[j],
                    uv_lightmap=lm_uvs[j],
                    bc=bc[j],
                    color=[float(0), float(0), float(0)]
                )
                vertices.append(vertex)

            # Find the texture name associated with this triangle
            texture_name = None
            for tex_name, tri_indices in self.textures.items():
                if i in tri_indices:
                    texture_name = tex_name
                    break

            # Create a CompiledTriangle with these 3 vertices
            compiled_triangle = CompiledTriangle(
                vertices=vertices,
                textureName=texture_name,  
                surfaceFlags=1, # Placeholder till souper returns flags
                contentFlags=1  # Placeholder till souper returns flags
            )

            compiled_triangles.append(compiled_triangle)

        # Write to binary file
        with open(binary_path, 'wb') as f:
            for triangle in compiled_triangles:
                f.write(triangle.to_binary())

        return self

    def create_texture_array(self, textures, uvs, textures_directory):
            # Step 1: Load all textures and find the largest resolution
            images = {}
            max_width, max_height = 0, 0
            texture_index_mapping = {}  # Dictionary to map texture names to indices

            for index, texture_name in enumerate(textures):
                path = Path(textures_directory) / f"{texture_name}.tga"
                image = Image.open(path).convert("RGBA")

                images[texture_name] = image
                max_width = max(max_width, image.width)
                max_height = max(max_height, image.height)
                texture_index_mapping[texture_name] = index  # Map texture name to its index

            # Step 2: Resize all images to the largest resolution
            for texture_name, image in images.items():
                if image.size != (max_width, max_height):
                    images[texture_name] = image.resize((max_width, max_height), Image.LANCZOS)

            # Step 3: Create a texture array with appropriate dimensions
            num_textures = len(images)
            texture_array = np.zeros((num_textures, max_height, max_width, 4), dtype=np.uint8)

            for index, (texture_name, image) in enumerate(images.items()):
                texture_array[index] = np.array(image)

            return texture_array, texture_index_mapping

    def create_frames(self, patch_resolution: float = 0.0625) -> 'Scene':
        """
        Frames are a datastructures which hold (planar) triangles to which the patch placement is deligated to.
        """
        
        # Step 1: Create Triangle Data Structures
        triangles_ds = []
        for vertex_tuple in self.triangles:
            vertices = tuple(vec(*v) for v in vertex_tuple)
            triangles_ds.append(Triangle(vertices))
        self.triangles_ds = triangles_ds

        # Step 2: Construct Frames and use them to create the uv mapping
        self.frames = self.__construct_frames(triangles_ds, 1/patch_resolution)
        self.frames, self.lightmap_uvs, uv_map_ws_size = uv_mapper.create_uv_mapping(self.frames, triangles_ds, patch_resolution, debug=True)
        self.light_map_resolution = math.ceil(uv_map_ws_size * patch_resolution)
        self.light_map = np.zeros((self.light_map_resolution, self.light_map_resolution, 3), dtype=np.float32)

        # Step 3: Precalculate geometry intersections (for illegal pixel calculations)
        [frame.calculate_intersections(triangles_ds) for frame in tqdm(self.frames, desc="Calculating intersections")]

        # Step 4: Generate Patches
        [frame.generate_patches(patch_resolution) for frame in tqdm(self.frames, desc="Generating patches")]

    
    def __construct_frames(self, triangles: List[Triangle], padding: float) -> List[Frame]:
        """Group triangles into shapes based on planar adjacency."""
        used = set()
        frames: List[Frame] = []

        for i, triangle in enumerate(triangles):
            if i in used:
                continue

            # Find a neighboring triangle to form a planar surface
            for j, other_triangle in enumerate(triangles):
                if i != j and j not in used and geometry.count_shared_vertices(triangle, other_triangle) == 2:
                    if geometry.are_coplanar(triangle, other_triangle):
                        frames.append(Frame(triangles=[triangle, other_triangle], triangles_indices=[i, j], patch_ws_size=padding))
                        used.update([i, j])
                        break
            else:
                # If no match is found, add the triangle as a single shape
                frames.append(Frame(triangles=[triangle], triangles_indices=[i, j], patch_ws_size=padding))
                used.add(i)

        return frames


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
                #u_t = random.random()
                #v_t = random.random()

                #print(u_t, v_t)
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

        
    
    


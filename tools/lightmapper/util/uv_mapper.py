import numpy as np
import math
import random
from PIL import Image, ImageDraw
from typing import List, Optional, Tuple
from pathlib import Path

from data_structures.triangle import Triangle
from data_structures.frame import Frame
from data_structures.vector3f import Vector3f

import util.geometry as geometry


def __pack_shapes(shapes: List[Frame], patch_size: float):

    # Sort shapes by height (descending order)
    sorted_shapes = sorted(shapes, key=lambda s: s.bounding_box[3], reverse=True)

    # Calculate the total area of all shapes
    total_area = sum(w * h for _, _, w, h in (shape.bounding_box for shape in sorted_shapes))

    # Calculate the target width based on total area
    target_width = math.sqrt(total_area)


    current_row_width = 0
    current_row_height = 0
    current_row_max_height = 0
    max_row_width = 0

    # append each shape to the current row, if the row exceedes the max_row_width create new row
    for shape in sorted_shapes:
        shape_width =  shape.bounding_box[2]
        shape_height = shape.bounding_box[3]
        snapped_width = geometry.snap_to_multiple(shape_width, patch_size)
        snapped_height = geometry.snap_to_multiple(shape_height, patch_size)
        if current_row_width + snapped_width> target_width:
            # create a new row, reset current_row metrics
            max_row_width = max(max_row_width, current_row_width)
            current_row_height = current_row_height + current_row_max_height
            current_row_max_height = 0
            current_row_width = 0
        
        shape.bounding_box = [current_row_width, current_row_height, snapped_width, snapped_height]

        current_row_max_height = max(current_row_max_height, snapped_height)
        current_row_width = current_row_width + snapped_width

    final_height = current_row_height + current_row_max_height
    final_width = max_row_width

    return sorted_shapes, final_width, final_height

def create_uv_mapping(frames: List[Frame], triangles: List[Triangle], patch_resolution: float, debug=False):

    packed_frames, total_width, total_height = __pack_shapes(frames,  1/patch_resolution)
    map_world_size = max(total_width, total_height)
    lightmap_resolution = int(map_world_size / (1/patch_resolution))


    # Normalize UVs to fit the final square texture size
    uv_coordinates = [None] * len(triangles)
    for packed_frame in packed_frames:
        x = packed_frame.bounding_box[0]
        y = packed_frame.bounding_box[1]
        width = packed_frame.bounding_box[2]
        height = packed_frame.bounding_box[3]
        
        # Normalize bounding box coordinates to UV coordinates
        min_u = x / map_world_size
        min_v = y / map_world_size
        max_u = (x + width) / map_world_size
        max_v = (y + height) / map_world_size
        bbox_uv_width = max_u - min_u
        bbox_uv_height = max_v - min_v
        
        # Normalize each vertex of the projected triangle to the bounding box and map to UV space
        for i, projected in enumerate(packed_frame.projected_triangles):
            uvs = []
            lm_uvs_bbox_space = []
            for vertex in projected:
                # Normalize within the bounding box and map to UV space
                u = vertex[0] / width  # Normalize within the bounding box width
                v = vertex[1] / height  # Normalize within the bounding box height
                # Map to UV space
                uvs.append((min_u + (u * bbox_uv_width), min_v + (v * bbox_uv_height)))
                lm_uvs_bbox_space.append((u, v))
            
            packed_frame.lm_uvs_bbox_space.append(lm_uvs_bbox_space)
            packed_frame.lm_uvs.append(uvs)
            uv_coordinates[packed_frame.triangles_indices[i]] = uvs

    if debug:
        __debug_uv_mapping(triangles, uv_coordinates, image_size=lightmap_resolution)
        __debug_frame_placement(packed_frames, map_world_size, image_size=lightmap_resolution)

    return packed_frames, uv_coordinates, map_world_size


def __debug_uv_mapping(triangles, uvs, image_size=148):
    """ Projects triangles onto an image based on their UVs and colors them in. """

    def generate_random_color():
        """Generate a random RGB color."""
        return (random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))
    
    def is_point_in_triangle(p, a, b, c):
        """Check if point p (x, y) is inside the triangle defined by a, b, c."""
        def sign(p1, p2, p3):
            return (p1[0] - p3[0]) * (p2[1] - p3[1]) - (p2[0] - p3[0]) * (p1[1] - p3[1])
        
        d1 = sign(p, a, b)
        d2 = sign(p, b, c)
        d3 = sign(p, c, a)
        
        has_neg = (d1 < 0) or (d2 < 0) or (d3 < 0)
        has_pos = (d1 > 0) or (d2 > 0) or (d3 > 0)

        return not (has_neg and has_pos)
    
    def fill_triangle(image, vertices, color):
        """Draw a triangle onto the image."""
        a, b, c = vertices
        
        min_x = max(0, int(min(a[0], b[0], c[0])))
        max_x = min(image.size[0], int(max(a[0], b[0], c[0])))
        min_y = max(0, int(min(a[1], b[1], c[1])))
        max_y = min(image.size[1], int(max(a[1], b[1], c[1])))
        
        draw = ImageDraw.Draw(image)
        
        # Loop through the bounding box and draw the triangle
        count = 0
        for x in range(min_x, max_x):
            for y in range(min_y, max_y):
                if is_point_in_triangle((x, y), a, b, c):
                    count += 1
                    draw.point((x, y), fill=color)
        return image, count

    # Create a blank image
    image = Image.new("RGB", (image_size, image_size), (255, 255, 255))

    for i, (triangle, uv_coords) in enumerate(zip(triangles, uvs)):
        color = generate_random_color()

        # Scale UVs to image dimensions
        scaled_uvs = [(int(u * image_size), int(v * image_size)) for u, v in uv_coords]

        # Fill the triangle in the image
        _, count = fill_triangle(image, scaled_uvs, color)

    # Define the path for the debug folder relative to the current script's location
    debug_path = Path(__file__).resolve().parent / "debug" 

    # Ensure the debug directory exists
    debug_path.mkdir(parents=True, exist_ok=True)

    # Save the image
    image.save(debug_path / "debug_uv_maps.png")


def __debug_frame_placement(frames: List[Frame], map_world_size, image_size=148):
    """ Projects shapes onto an image based on their bbox and colors them in. """

    def generate_random_color():
        """Generate a random RGB color."""
        return (random.randint(0, 255), random.randint(0, 255), random.randint(0, 255))

    
    def fill_bbox(image, bbox, color):
        """Draw a triangle onto the image."""
        draw = ImageDraw.Draw(image)
        
        # Loop through the bounding box and draw the triangle
        count = 0
        for x in range(bbox[0], bbox[0] + bbox[2]):
            for y in range(bbox[1], bbox[1] + bbox[3]):
                count += 1
                draw.point((x, y), fill=color)
        return image, count

    # Create a blank image
    image = Image.new("RGB", (image_size, image_size), (255, 255, 255))

    for i, frame in enumerate(frames):
        color = generate_random_color()

        bbox = frame.bounding_box
        uv_bounding_box = [bbox[0] / map_world_size, bbox[1] / map_world_size, bbox[2] / map_world_size, bbox[3] / map_world_size]
        pixel_bounding_box = [int(uv_bounding_box[0] * image_size), int(uv_bounding_box[1] * image_size), int(uv_bounding_box[2] * image_size), int(uv_bounding_box[3] * image_size)]

        # Fill the triangle in the image
        _, count = fill_bbox(image, pixel_bounding_box, color)

    # Define the path for the debug folder relative to the current script's location
    debug_path = Path(__file__).resolve().parent / "debug" 

    # Ensure the debug directory exists
    debug_path.mkdir(parents=True, exist_ok=True)

    # Save the image
    image.save(debug_path / "debug_uv_maps_frames.png")
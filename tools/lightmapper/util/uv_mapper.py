import numpy as np
import math
import random
from PIL import Image, ImageDraw

def __project_to_2d(triangle):
    """ Projects the 3D triangle onto a 2D plane based on its normal """
    v0, v1, v2 = triangle
    # Calculate the normal of the triangle
    normal = np.cross(np.subtract(v1, v0), np.subtract(v2, v0))
    normal = normal / np.linalg.norm(normal)  # Normalize the normal

    # Find the dominant axis of the normal to project onto 2D
    abs_normal = np.abs(normal)
    if abs_normal[0] > abs_normal[1] and abs_normal[0] > abs_normal[2]:
        # Dominant axis is X, project onto YZ plane
        projected = [(v[1], v[2]) for v in triangle]
    elif abs_normal[1] > abs_normal[0] and abs_normal[1] > abs_normal[2]:
        # Dominant axis is Y, project onto XZ plane
        projected = [(v[0], v[2]) for v in triangle]
    else:
        # Dominant axis is Z, project onto XY plane
        projected = [(v[0], v[1]) for v in triangle]
    
    return projected

def __calculate_bounding_box_2d(projected_triangle, triangle_index):
    """ Calculates a bounding box for a triangle projected onto a 2D plane """
    min_x = min(v[0] for v in projected_triangle)
    max_x = max(v[0] for v in projected_triangle)
    min_y = min(v[1] for v in projected_triangle)
    max_y = max(v[1] for v in projected_triangle)

    width = max_x - min_x if max_x != min_x else 1.0  # Avoid division by zero
    height = max_y - min_y if max_y != min_y else 1.0 # Avoid division by zero
    
    #width = math.ceil(width / 16) * 16
    #height = math.ceil(height / 16) * 16

    return (0, 0, width, height, triangle_index)

def __pack_bounding_boxes(bboxes, padding=64):
    #TODO: padding, doesn not currently work with values other than 0
    # Sort bounding boxes by height (tallest first)
    bboxes = sorted(bboxes, key=lambda b: b[3], reverse=True)
    
    packed_positions = []
    shelves = []  # Each shelf is a list of bounding boxes
    shelf_heights = []
    
    # Start with an initial empty shelf
    shelves.append([])
    shelf_heights.append(0)
    
    for bbox in bboxes:
        width, height, index = bbox[2], bbox[3], bbox[4]
        
        # Find the shelf with the smallest current width
        shelf_index = None
        min_x_extend = float('inf')
        sum_shelf_heights = sum(shelf_heights)
        for i, shelf in enumerate(shelves):
            current_width = sum(box[2] for box in shelf)
            if current_width + width <= sum_shelf_heights and current_width < min_x_extend:
                min_x_extend = current_width
                shelf_index = i
        
        # If no suitable shelf is found, create a new one
        if shelf_index is None:
            shelves.append([])
            shelf_heights.append(0)
            shelf_index = len(shelves) - 1
        
        # Add the bounding box to the selected shelf
        shelves[shelf_index].append(bbox)
        shelf_heights[shelf_index] = max(shelf_heights[shelf_index], height)
    
    # Now, calculate the packed positions for each bounding box
    max_width = 0
    current_y = 0
    for i, shelf in enumerate(shelves):
        current_x = padding
        for bbox in shelf:
            width, height, index = bbox[2], bbox[3], bbox[4]
            packed_positions.append((current_x, current_y, width, height, index))
            current_x += width + padding
        current_y += shelf_heights[i] + padding
        max_width = max(max_width, current_x)
    
    max_height = current_y
    return sorted(packed_positions, key=lambda x: x[4]), max_width, max_height

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

    # Save the image
    image.save("debug_uv_maps.png")

def map_triangles(triangles, patch_resolution: float, debug=False):

    bounding_boxes = []
    projected_triangles = []

    for index, tri in enumerate(triangles):
        # Project the triangle onto its local 2D plane
        projected = __project_to_2d(tri)
        bounding_box = __calculate_bounding_box_2d(projected, index)
        bounding_boxes.append(bounding_box)
        projected_triangles.append(projected)

    # Pack bounding boxes and get final packing positions
    packed_boxes, total_width, total_height = __pack_bounding_boxes(bounding_boxes, padding=1/patch_resolution)

    map_world_size = max(total_width, total_height)

    # Normalize UVs to fit the final square texture size
    uv_coordinates = []
    for bbox, (x, y, width, height, index), projected in zip(bounding_boxes, packed_boxes, projected_triangles):
        # Normalize bounding box coordinates to UV coordinates
        min_u = x / map_world_size
        min_v = y / map_world_size
        max_u = (x + width) / map_world_size
        max_v = (y + height) / map_world_size
        
        # Normalize each vertex of the projected triangle to the bounding box and map to UV space
        uvs = []
        min_u_triangle = min(v[0] for v in projected)
        min_v_triangle = min(v[1] for v in projected)
        for v in projected:
            # Normalize within the bounding box and map to UV space
            u = (v[0] - min_u_triangle) / bbox[2]  # Normalize within the bounding box width
            v = (v[1] - min_v_triangle) / bbox[3]  # Normalize within the bounding box height
            # Map to UV space
            uvs.append((min_u + u * (max_u - min_u), min_v + v * (max_v - min_v)))
        
        uv_coordinates.append(uvs)

    if debug:
        __debug_uv_mapping(triangles, uv_coordinates)

    return uv_coordinates, map_world_size
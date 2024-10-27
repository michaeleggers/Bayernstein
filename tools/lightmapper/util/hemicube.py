import numpy as np
import matplotlib.pyplot as plt
import cv2

from data_structures.vector3f import Vector3f

def generate_hemicube_views(direction: Vector3f):

    direction_array = direction.to_array()
    up_vector = calculate_camera_up(direction_array)
    right_vector = np.cross(direction_array, up_vector)

    # Store rotated vectors to avoid multiple calls
    rotated_views = [
        rotate_vectors_90_degrees(direction_array, up_vector, right_vector),
        rotate_vectors_90_degrees(direction_array, up_vector, -right_vector),
        rotate_vectors_90_degrees(direction_array, up_vector, up_vector),
        rotate_vectors_90_degrees(direction_array, up_vector, -up_vector)
    ]

    # Define views with directions and up vectors
    views = [
        {'direction': direction_array, 'up_vector': up_vector},  # Forward
        {'direction': rotated_views[0][0], 'up_vector': rotated_views[0][1]},  # Up
        {'direction': rotated_views[1][0], 'up_vector': rotated_views[1][1]},  # Down
        {'direction': rotated_views[2][0], 'up_vector': rotated_views[2][1]},  # Left
        {'direction': rotated_views[3][0], 'up_vector': rotated_views[3][1]}   # Right
    ]

    # Define frustums and view sizes
    #frustums = [
    #    [-near_plane, near_plane, -near_plane, near_plane],
    #    [-near_plane, 0, -near_plane, near_plane],
    #    [0, near_plane, -near_plane, near_plane],
    #    [-near_plane, near_plane, 0, near_plane],
    #    [-near_plane, near_plane, -near_plane, 0]
    #]
    
    frustums = [
        # l  r   u  d
        [-1, 1, -1, 1], 
        [-1, 1,  0, 1], # up view
        [-1, 1, -1, 0], # down view
        [ 0, 1, -1, 1], # left view
        [-1, 0, -1, 1]  # right view
    ]

    view_sizes = [
        [1,   1],
        [0.5, 1],
        [0.5, 1],
        [1, 0.5],
        [1, 0.5]
    ]

    # Combine view data with frustums and sizes
    for index, view in enumerate(views):
        view.update({
            'frustum': frustums[index],
            'size': view_sizes[index]
        })

    return views

def calculate_camera_up(direction):
    global_up = np.array([0, 1, 0])
    # Check if the direction is close to global up or down
    if np.abs(np.dot(direction, global_up)) > 0.9999:  # Almost parallel
        # Use x-axis as the up vector if direction is nearly vertical
        return np.array([1, 0, 0])
    else:
        # Perpendicular to both direction and global up
        return np.cross(direction, np.cross(global_up, direction))
    
def rotate_vectors_90_degrees(direction, camera_up, axis):
    # Normalize the axis to ensure proper rotation
    axis = axis / np.linalg.norm(axis)
    
    # Rotation matrix for 90 degrees (π/2) around the given axis
    cos_theta = 0  # cos(90°)
    sin_theta = 1  # sin(90°)
    
    # Rodrigues' rotation formula components
    rotation_matrix = np.array([
        [cos_theta + axis[0]**2 * (1 - cos_theta),
         axis[0] * axis[1] * (1 - cos_theta) - axis[2] * sin_theta,
         axis[0] * axis[2] * (1 - cos_theta) + axis[1] * sin_theta],

        [axis[1] * axis[0] * (1 - cos_theta) + axis[2] * sin_theta,
         cos_theta + axis[1]**2 * (1 - cos_theta),
         axis[1] * axis[2] * (1 - cos_theta) - axis[0] * sin_theta],

        [axis[2] * axis[0] * (1 - cos_theta) - axis[1] * sin_theta,
         axis[2] * axis[1] * (1 - cos_theta) + axis[0] * sin_theta,
         cos_theta + axis[2]**2 * (1 - cos_theta)]
    ])

    # Apply the rotation to both the direction and camera_up vectors
    rotated_direction = np.dot(rotation_matrix, direction)
    rotated_camera_up = np.dot(rotation_matrix, camera_up)
    
    return rotated_direction, rotated_camera_up

def generate_correction_map(size):

    distortion_view = calculate_lambert_map_front(size)
    distortion_map = merge_views_cosine_map(distortion_view)

    lambert_map_front = calculate_lambert_map_front(size)
    lambert_map_side = calculate_lambert_map_sides(size, size//2, fov=90)
    lambert_map = merge_views_lambert_map(lambert_map_front, lambert_map_side, lambert_map_side, lambert_map_side, lambert_map_side)
    
    correction_map = normalize_map(distortion_map * lambert_map)
    
    save_correction_map_as_png(distortion_map, 'debug_cosine_map')
    save_correction_map_as_png(lambert_map, 'debug_lambert_map')
    save_correction_map_as_png(correction_map, 'debug_correction_map')

    expanded_correction = np.repeat(correction_map[:, :, np.newaxis], 3, axis=2)



    return expanded_correction

def normalize_map(map):
    # Normalize the result
    total_sum = np.sum(map)  # Calculate the sum of all elements
    if total_sum != 0:  # Check to avoid division by zero
        normalized_map = map / total_sum
    else:
        normalized_map = map  # Keep it the same if sum is 0

    return normalized_map

def merge_views_hemicube(front, up, down, left, right):
    """Merge the images into a plus-shaped canvas."""
    # Assuming all images have the same height as the front image's height
    image_height, image_width, _ = front.shape

    # Create a blank canvas for the "plus" shape
    canvas = np.zeros((image_height * 2, image_width * 2, 3), dtype=np.float32)
    # Place the images in the "plus" shape, not ethe invertet axis
    # front
    canvas[image_height // 2: image_height // 2 + image_height, image_width // 2: image_width // 2 + image_width] = front
    # up
    canvas[image_height // 2: image_height // 2 + image_height, image_width // 2 + image_width:] = right
    # down
    canvas[image_height // 2: image_height // 2 + image_height, : image_width // 2] = left
    # lefts
    canvas[:image_height // 2, image_width // 2: image_width // 2 + image_width] = up
    # right
    canvas[image_height + image_height // 2:, image_width // 2: image_width // 2 + image_width] = down

    return canvas

def merge_views_lambert_map(front, up, down, left, right):
    """Merge the images into a plus-shaped canvas."""
    # Assuming all images have the same height as the front image's height
    image_height, image_width = front.shape

    # Create a blank canvas for the "plus" shape
    canvas = np.zeros((image_height * 2, image_width * 2), dtype=np.float32)
    # Place the images in the "plus" shape, not ethe invertet axis
    # front
    canvas[image_height // 2: image_height // 2 + image_height, image_width // 2: image_width // 2 + image_width] = front
    # up
    canvas[image_height // 2: image_height // 2 + image_height, image_width // 2 + image_width:] = np.rot90(right, k=-1)
    # down
    canvas[image_height // 2: image_height // 2 + image_height, : image_width // 2] = np.rot90(left)
    # lefts
    canvas[:image_height // 2, image_width // 2: image_width // 2 + image_width] = up
    # right
    canvas[image_height + image_height // 2:, image_width // 2: image_width // 2 + image_width] = np.flip(down)

    return canvas

def merge_views_cosine_map(original_image):
    # Get the dimensions of the original image
    size = original_image.shape[0]
    
    # Create a new canvas twice the size of the original image, initialized with zeros (black)
    canvas = np.zeros((size * 2, size * 2))
    
    # Calculate positions
    # Center the original image
    canvas[size // 2: size // 2 + size, size // 2: size // 2 + size] = original_image
    
    # Place the right half of the original image in the left border
    canvas[size // 2: size // 2 + size, :size // 2] = original_image[:, size // 2:]

    # Place the left half of the original image in the right border
    canvas[size // 2: size // 2 + size, size + size // 2:] = original_image[:, :size // 2]

    # Place the upper half of the original image in the lower border
    canvas[:size // 2, size // 2: size // 2 + size] = original_image[size // 2:, :]

    # Place the lower half of the original image in the upper border
    canvas[size + size // 2:, size // 2: size // 2 + size] = original_image[:size // 2, :]

    return canvas

def calculate_cosine_map(width, height, fov=90):
    # Create a 2D array to hold the angles
    angles = np.zeros((height, width), dtype=float)

    # Calculate the half width and half height
    half_width = width / 2
    half_height = height / 2

    # Convert FOV to radians
    max_angle = fov / 2  # Maximum angle from the center to the edge

    # Calculate angles for each pixel
    for y in range(height):
        for x in range(width):
            # Compute normalized coordinates from center
            normalized_x = (x - half_width) / half_width
            normalized_y = (y - half_height) / half_height
            
            # Calculate angle from the center
            angle = np.sqrt(normalized_x**2 + normalized_y**2) * max_angle  # Magnitude
            
            # Cap the angle to the maximum FOV limit
            angles[y, x] = np.cos(np.radians(angle))

    return angles

def calculate_lambert_map_front(size):
    # Create a 2D array to hold the angles
    angles = np.zeros((size, size), dtype=float)

    # Calculate angles for each pixel
    for y in range(size):
        for x in range(size):
            
            normalized_x = 2 * (x / size) - 1
            normalized_y = 2 * (y / size) - 1

            vector = [normalized_x, 1, normalized_y]
            angle = angle_with_xz_plane(vector)
            angles[y, x] = np.cos(np.radians(90 - angle))

    return angles

def calculate_lambert_map_sides(width, height, fov=45):
    # Create a 2D array to hold the angles
    angles = np.zeros((height, width), dtype=float)

    # Calculate angles for each pixel
    for y in range(height):
        for x in range(width):
            
            normalized_x = 2 * (x / width) - 1
            normalized_y = y / height

            vector = [normalized_x, normalized_y, 1]
            angle = angle_with_xz_plane(vector)
            angles[y, x] = np.cos(np.radians(90 - angle))

    return angles


def angle_with_xz_plane(vector):
    # Ensure the vector is a numpy array
    vector = np.array(vector)
    
    # Calculate the angle in radians
    angle_rad = np.arctan2(vector[1], np.sqrt(vector[0]**2 + vector[2]**2))
    
    # Convert to degrees
    angle_deg = np.degrees(angle_rad)
    
    return angle_deg


def cut_image_with_frustum(image, frustum):
    # Get the original image dimensions
    width, height, channels = image.shape

    # Frustum values
    left, right, bottom, top = frustum

    # Calculate pixel boundaries
    x_start = int((left + 1) / 2 * width)  # Map from [-1, 1] to [0, width]
    x_end = int((right + 1) / 2 * width)
    y_start = int((bottom + 1) / 2 * height)  # Map from [-1, 1] to [0, height]
    y_end = int((top + 1) / 2 * height)

    # Slice the image based on the calculated boundaries
    cut_image = image[y_start:y_end, x_start:x_end]  # Note the order of slicing

    return cut_image

def save_correction_map_as_png(hemicube, name):
    # Create a new figure
    plt.figure()

    # Create the plot
    plt.imshow(hemicube[:, :], cmap='viridis', aspect='auto')
    plt.colorbar()
    plt.axis('off')

    # Save the figure
    plt.savefig(f'{name}.png', bbox_inches='tight', pad_inches=0.1)

    # Close the figure to prevent merging plots on subsequent calls
    plt.close()
import numpy as np


def bounding_box_triangle(A, B, C):
    min_x = min(A[0], B[0], C[0])
    max_x = max(A[0], B[0], C[0])
    min_y = min(A[1], B[1], C[1])
    max_y = max(A[1], B[1], C[1])
    return (min_x, max_x, min_y, max_y)

def bounding_box_square(A, B, C, D):
    min_x = min(A[0], B[0], C[0], D[0])
    max_x = max(A[0], B[0], C[0], D[0])
    min_y = min(A[1], B[1], C[1], D[1])
    max_y = max(A[1], B[1], C[1], D[1])
    return (min_x, max_x, min_y, max_y)

def bounding_boxes_overlap(box1, box2):
    # box1 and box2 are tuples: (min_x, max_x, min_y, max_y)
    return not (box1[1] < box2[0] or box1[0] > box2[1] or box1[3] < box2[2] or box1[2] > box2[3])

def is_point_in_bounding_box(A, B, C, D, P):
    # Unpack square coordinates
    x1, y1 = A
    x2, y2 = B
    x3, y3 = C
    x4, y4 = D
    px, py = P

    # Calculate bounding box of the square
    min_x = min(x1, x2, x3, x4)
    max_x = max(x1, x2, x3, x4)
    min_y = min(y1, y2, y3, y4)
    max_y = max(y1, y2, y3, y4)

    # Check if the point is within the bounding box
    return min_x <= px <= max_x and min_y <= py <= max_y

def sign(p1, p2, p3):
    return (p1[0] - p3[0]) * (p2[1] - p3[1]) - (p2[0] - p3[0]) * (p1[1] - p3[1])

def is_point_in_triangle(pt: np.ndarray, v1: np.ndarray, v2: np.ndarray, v3: np.ndarray) -> bool:
    # Barycentric coordinate method
    v0 = v2 - v1
    v1_to_v3 = v3 - v1
    v1_to_pt = pt - v1
    
    # Compute dot products
    dot00 = np.dot(v0, v0)
    dot01 = np.dot(v0, v1_to_v3)
    dot02 = np.dot(v0, v1_to_pt)
    dot11 = np.dot(v1_to_v3, v1_to_v3)
    dot12 = np.dot(v1_to_v3, v1_to_pt)

    # Compute barycentric coordinates
    denom = dot00 * dot11 - dot01 * dot01
    if denom == 0:
        return False  # Degenerate triangle
    inv_denom = 1 / denom
    u = (dot11 * dot02 - dot01 * dot12) * inv_denom
    v = (dot00 * dot12 - dot01 * dot02) * inv_denom

    # Check if point is in triangle
    return (u >= 0) and (v >= 0) and (u + v <= 1)

def edges_intersect(p1, p2, q1, q2):
    # Helper function to check if two line segments intersect
    def orientation(p, q, r):
        val = (q[1] - p[1]) * (r[0] - q[0]) - (q[0] - p[0]) * (r[1] - q[1])
        if val == 0: return 0  # Collinear
        return 1 if val > 0 else -1  # Clockwise or counterclockwise

    def on_segment(p, q, r):
        return min(p[0], r[0]) <= q[0] <= max(p[0], r[0]) and min(p[1], r[1]) <= q[1] <= max(p[1], r[1])

    o1 = orientation(p1, p2, q1)
    o2 = orientation(p1, p2, q2)
    o3 = orientation(q1, q2, p1)
    o4 = orientation(q1, q2, p2)

    # General case: segments intersect
    if o1 != o2 and o3 != o4:
        return True

    # Special cases: segments are collinear and overlap
    if o1 == 0 and on_segment(p1, q1, p2): return True
    if o2 == 0 and on_segment(p1, q2, p2): return True
    if o3 == 0 and on_segment(q1, p1, q2): return True
    if o4 == 0 and on_segment(q1, p2, q2): return True

    return False

def square_triangle_overlap2(square, triangle):
    A, B, C, D = square
    T1, T2, T3 = triangle

    # Create bounding boxes for square and triangle
    square_bbox = bounding_box_square(A, B, C, D)
    triangle_bbox = bounding_box_triangle(T1, T2, T3)

    # Check if the bounding boxes overlap
    if not bounding_boxes_overlap(square_bbox, triangle_bbox):
        return False

    # Proceed with detailed checks if bounding boxes overlap
    if is_point_in_bounding_box(A, B, C, D, T1) or is_point_in_bounding_box(A, B, C, D, T2) or is_point_in_bounding_box(A, B, C, D, T3):
        return True

    if is_point_in_triangle(T1, T2, T3, A) or is_point_in_triangle(T1, T2, T3, B) or is_point_in_triangle(T1, T2, T3, C) or is_point_in_triangle(T1, T2, T3, D):
        return True

    square_edges = [(A, B), (B, C), (C, D), (D, A)]
    triangle_edges = [(T1, T2), (T2, T3), (T3, T1)]

    for edge1 in square_edges:
        for edge2 in triangle_edges:
            if edges_intersect(edge1[0], edge1[1], edge2[0], edge2[1]):
                return True

    return False

def calculate_normal(v0, v1, v2):
        
    # Calculate vectors
    edge1 = v1 - v0
    edge2 = v2 - v0

    # Compute the normal using the cross product
    normal = np.cross(edge1, edge2)
    normal = normal / np.linalg.norm(normal)  # Normalize the normal

    return normal

def calculate_camera_up(direction):
    global_up = np.array([0, 0, 1])
    # Check if the direction is close to global up or down
    if np.abs(np.dot(direction, global_up)) > 0.9999:  # Almost parallel
        # Use x-axis as the up vector if direction is nearly vertical
        return np.array([1, 0, 0])
    else:
        # Perpendicular to both direction and global up
        return np.cross(direction, np.cross(global_up, direction))
    
def area_of_polygon(polygon):
    """Calculate the area of a 2D polygon using the shoelace formula."""
    x = [p[0] for p in polygon]
    y = [p[1] for p in polygon]
    return 0.5 * abs(np.dot(x, np.roll(y, 1)) - np.dot(y, np.roll(x, 1)))

def clip_polygon(polygon, edge_start, edge_end):
    """Clip a polygon by an edge using the Sutherland-Hodgman algorithm."""
    def is_inside(point, edge_start, edge_end):
        # Check if a point is on the inside side of an edge
        return (edge_end[0] - edge_start[0]) * (point[1] - edge_start[1]) - (edge_end[1] - edge_start[1]) * (point[0] - edge_start[0]) <= 0
    
    def intersection(p1, p2, edge_start, edge_end):
        # Calculate intersection point of line p1-p2 with edge edge_start-edge_end
        x1, y1 = p1
        x2, y2 = p2
        x3, y3 = edge_start
        x4, y4 = edge_end
        denominator = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4)
        if denominator == 0:
            return None  # Parallel lines
        t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / denominator
        return [x1 + t * (x2 - x1), y1 + t * (y2 - y1)]
    
    clipped_polygon = []
    prev_point = polygon[-1]
    for point in polygon:
        if is_inside(point, edge_start, edge_end):
            if not is_inside(prev_point, edge_start, edge_end):
                clipped_polygon.append(intersection(prev_point, point, edge_start, edge_end))
            clipped_polygon.append(point)
        elif is_inside(prev_point, edge_start, edge_end):
            clipped_polygon.append(intersection(prev_point, point, edge_start, edge_end))
        prev_point = point
    
    return clipped_polygon

def square_triangle_overlap(square, triangle, threshold=0.00000001):
    A, B, C, D = square
    T1, T2, T3 = triangle

    # Step 1: Calculate bounding boxes for early exit
    square_bbox = bounding_box_square(A, B, C, D)
    triangle_bbox = bounding_box_triangle(T1, T2, T3)

    if not bounding_boxes_overlap(square_bbox, triangle_bbox):
        return False

    # Step 2: Clip the triangle by the square's edges
    clipped_polygon = triangle
    square_edges = [(A, B), (B, C), (C, D), (D, A)]
    for edge_start, edge_end in square_edges:
        clipped_polygon = clip_polygon(clipped_polygon, edge_start, edge_end)
        if len(clipped_polygon) == 0:
            return False  # No intersection
    
    # Step 3: Calculate the area of the clipped polygon (intersection area)
    if len(clipped_polygon) < 3:
        return False  # Not a valid polygon
    overlap_area = area_of_polygon(clipped_polygon)

    # Step 4: Check if overlap area is greater than the threshold
    #print(overlap_area)
    return overlap_area > threshold
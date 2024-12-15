import numpy as np
from data_structures.triangle import Triangle
from data_structures.line_segment import LineSegment
from data_structures.vector3f import Vector3f
from typing import List, Tuple, Optional
from dataclasses import dataclass
import math


def are_coplanar(triangle1: Triangle, triangle2: Triangle, tolerance: float = 1e-1) -> bool:
    """Check if two triangles are coplanar."""
    # Extract vertices as numpy arrays
    v1, v2, v3 = (v.to_array() for v in triangle1.vertices)
    w1, w2, w3 = (v.to_array() for v in triangle2.vertices)

    # Compute normals of the triangles
    normal1 = np.cross(v2 - v1, v3 - v1)
    normal2 = np.cross(w2 - w1, w3 - w1)

    # Check if normals are parallel (dot product close to 1 or -1)
    if not np.isclose(np.dot(normal1, normal2) / (np.linalg.norm(normal1) * np.linalg.norm(normal2)), 1, atol=tolerance):
        return False

    # Check if all vertices of triangle2 lie in the plane of triangle1
    plane_d = -np.dot(normal1, v1)  # Plane equation: normal1 · x + d = 0
    for w in [w1, w2, w3]:
        if not np.isclose(np.dot(normal1, w) + plane_d, 0, atol=tolerance):
            return False

    return True

def count_shared_vertices(triangle1: Triangle, triangle2: Triangle, tolerance: float = 1e-6) -> int:
    """Count the number of approximately shared vertices between two triangles."""
    def are_approx_equal(v1: Vector3f, v2: Vector3f) -> bool:
        return np.allclose(v1.to_array(), v2.to_array(), atol=tolerance)
    
    return sum(1 for v1 in triangle1.vertices for v2 in triangle2.vertices if are_approx_equal(v1, v2))

def snap_to_multiple(value, multiple):
    return ((value + multiple - 1) // multiple) * multiple

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

def square_triangle_overlap(square, triangle, threshold=0.001):
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

def square_triangles_overlap(square, triangles, threshold=0.001):
    """
    Check if a square overlaps with at least one triangle from a list of triangles.

    Parameters:
        square: List of 4 points defining the square [(x1, y1), (x2, y2), ...].
        triangles: List of triangles, where each triangle is defined as 
                   [(x1, y1), (x2, y2), (x3, y3)].
        threshold: Minimum overlap area to consider it an intersection.

    Returns:
        bool: True if the square overlaps with at least one triangle, False otherwise.
    """
    for triangle in triangles:
        if square_triangle_overlap(square, triangle, threshold):
            return True
    return False

def compute_intersection(tri1: Triangle, tri2: Triangle) -> Optional[Tuple[Vector3f, Vector3f]]:
    """
    Computes the intersection line segment (if any) between two triangles.
    Returns a tuple of two Vector3f points (start, end) if they intersect, or None otherwise.
    """
    def edge_plane_intersection(p1: Vector3f, p2: Vector3f, tri: Triangle) -> Optional[Vector3f]:
        """
        Finds the intersection point (if any) between a triangle's plane and an edge.
        """
        v0, v1, v2 = tri.vertices
        normal = tri.normal
        edge = p2 - p1
        edge_dot_normal = edge.dot(normal)

        # If edge is parallel to the triangle plane, there's no intersection
        if abs(edge_dot_normal) < 1e-6:
            return None

        # Calculate the intersection point using the plane equation
        t = ((v0 - p1).dot(normal)) / edge_dot_normal
        if 0 <= t <= 1:  # Ensure the intersection lies on the edge segment
            return p1 + Vector3f(edge.x * t, edge.y * t, edge.z * t)
        return None

    def is_point_strictly_in_triangle(pt: Vector3f, tri: Triangle) -> bool:
        """
        Check if a point lies strictly inside a triangle using barycentric coordinates.
        The point must not be on the edges.
        """
        v0, v1, v2 = tri.vertices
        u = v1 - v0
        v = v2 - v0
        w = pt - v0

        uv = u.dot(v)
        wv = w.dot(v)
        vv = v.dot(v)
        wu = w.dot(u)
        uu = u.dot(u)

        denom = uv * uv - uu * vv
        if abs(denom) < 1e-6:  # Degenerate triangle
            return False

        # Calculate barycentric coordinates
        s = (uv * wv - vv * wu) / denom
        t = (uv * wu - uu * wv) / denom

        # Check if point lies strictly inside (not on the edges)
        return s > 0 and t > 0 and (s + t) < 1

    def cross_product(v1: Vector3f, v2: Vector3f) -> Vector3f:
        """ Return the cross product of two 3D vectors. """
        return Vector3f(
            v1.y * v2.z - v1.z * v2.y,
            v1.z * v2.x - v1.x * v2.z,
            v1.x * v2.y - v1.y * v2.x
        )

    def normalize(v):
        """ Normalize a vector. """
        norm = np.linalg.norm(v)
        if norm == 0:
            return v
        return v / norm

    def cross_product(v1, v2):
        """ Return the cross product of two 3D vectors. """
        return np.cross(v1, v2)

    def rotate_vector_90_degrees(v, up_vector, clockwise=True):
        """
        Rotate vector `v` by 90 degrees around `up_vector`.
        If clockwise is True, rotate clockwise (right), otherwise rotate counterclockwise (left).
        """
        # Normalize the vectors
        up_vector = normalize(up_vector)
        v = normalize(v)
        
        # Use the cross product to determine the direction of rotation
        if clockwise:
            return cross_product(up_vector, v)
        else:
            return cross_product(v, up_vector)

    def is_left_of_line(start, end, up_vector, other_normal):
        """
        Determine if the vector `other_normal` is closer to the left vector or right vector
        created by rotating the vector from `start` to `end` using `up_vector`.
        
        Returns True if `other_normal` is closer to the left vector.
        """
        # Step 1: Calculate the vector from start to end (AB)
        AB = end - start
        
        # Step 2: Rotate AB to create left and right vectors
        left_vector = rotate_vector_90_degrees(AB, up_vector, clockwise=False)  # Left: counterclockwise (90°)
        right_vector = rotate_vector_90_degrees(AB, up_vector, clockwise=True)  # Right: clockwise (90°)

        
        
        # Step 3: Calculate the dot products of `other_normal` with the left and right vectors
        left_dot = np.dot(other_normal, left_vector)
        right_dot = np.dot(other_normal, right_vector)
        #print(AB, left_vector, right_vector, left_dot > right_dot)
        # Step 4: Return True if `other_normal` is closer to the left vector (larger dot product with left)
        return left_dot > right_dot

    # Collect potential intersection points
    intersection_points = []

    # Check each edge of tri1 against tri2's plane
    for edge_start, edge_end in zip(tri1.vertices, tri1.vertices[1:] + tri1.vertices[:1]):
        point = edge_plane_intersection(edge_start, edge_end, tri2)
        if point and is_point_strictly_in_triangle(point, tri2):
            intersection_points.append(point)

    # Check each edge of tri2 against tri1's plane
    for edge_start, edge_end in zip(tri2.vertices, tri2.vertices[1:] + tri2.vertices[:1]):
        point = edge_plane_intersection(edge_start, edge_end, tri1)
        if point and is_point_strictly_in_triangle(point, tri1):
            intersection_points.append(point)

    # If we have exactly 2 unique intersection points, return them as a directed segment
    if len(intersection_points) >= 2:
        # Deduplicate points
        unique_points = list({(p.x, p.y, p.z): p for p in intersection_points}.values())
        if len(unique_points) == 2:
            start, end = unique_points
            normal1 = tri1.normal.to_array()
            normal2 = tri2.normal.to_array()
            # Check if the line segment needs to be reversed based on normal orientations
            if not is_left_of_line(start.to_array(), end.to_array(), normal1, normal2):
                start, end = end, start

            return start, end

    return None

def calculate_closest_distance(point: Tuple[float, float], lines: List[LineSegment], x_unit: float, y_unit: float) -> float:
    """
    Calculate the closest distance from a point to a list of directed lines A-B, considering axis scaling.
    Only lines where the point is on the right side of the direction from A to B are considered.
    
    Parameters:
    - point: The 2D point (x, y).
    - lines: A list of tuples, each containing two points (A, B), defining the directed line segment A-B.
    - x_unit: The unit scale factor for the x-axis.
    - y_unit: The unit scale factor for the y-axis.
    
    Returns:
    - The closest distance from the point to a valid line, considering the direction and scaling.
    """
    px, py = point  # Point to calculate distance to

    min_distance = float('inf')  # Initialize to a very large number

    for line in lines:
        (ax, ay) = line.start
        (bx, by) = line.end
        # Scale the coordinates by the respective units
        ax_scaled = ax * x_unit
        ay_scaled = ay * y_unit
        bx_scaled = bx * x_unit
        by_scaled = by * y_unit
        px_scaled = px * x_unit
        py_scaled = py * y_unit

        # Vector AB and AP
        AB = np.array([bx_scaled - ax_scaled, by_scaled - ay_scaled])
        AP = np.array([px_scaled - ax_scaled, py_scaled - ay_scaled])

        # Check if the point is on the right side of the directed line AB using the cross product
        cross_product = AB[0] * AP[1] - AB[1] * AP[0]  # 2D cross product (AB x AP)

        # Only consider lines where the point is on the right (cross_product > 0)
        if cross_product > 0 or True:
            # Perpendicular distance to the line
            line_length = np.linalg.norm(AB)
            if line_length == 0:
                continue  # Skip degenerate line (A and B are the same point)

            # Use the formula for the perpendicular distance from a point to a line
            distance = abs(AB[1] * px_scaled - AB[0] * py_scaled + bx_scaled * ay_scaled - by_scaled * ax_scaled) / line_length

            # Update the minimum distance
            min_distance = min(min_distance, distance)

    return min_distance if min_distance != float('inf') else None  # Return None if no valid line found

def point_to_line_distance_right_of_segment(point, line_segments):
    def is_left(p, p1, p2):
        # Calculate the 2D cross product of (p1 -> p2) and (p1 -> p)
        return (p2[0] - p1[0]) * (p[1] - p1[1]) - (p2[1] - p1[1]) * (p[0] - p1[0])

    def distance_point_to_segment(p, p1, p2):
        # Vector from p1 to p2
        line_vec = np.array(p2) - np.array(p1)
        # Vector from p1 to p
        point_vec = np.array(p) - np.array(p1)
        
        # Project point_vec onto line_vec and compute the scalar projection t
        line_len = np.dot(line_vec, line_vec)  # length squared of the line
        if line_len == 0:
            return np.linalg.norm(point_vec)  # p1 and p2 are the same point
        t = np.dot(point_vec, line_vec) / line_len
        
        # If the projection point falls outside the segment, clamp t to the nearest endpoint
        if t < 0.0:
            projection = np.array(p1)
        elif t > 1.0:
            projection = np.array(p2)
        else:
            projection = np.array(p1) + t * line_vec
        
        # Return the distance from the point to the projection
        return np.linalg.norm(np.array(p) - projection)

    # Filter line segments to only include those where the point is to the left of the segment
    valid_segments = [
        seg for seg in line_segments if is_left(point, seg.start, seg.end) < 0
    ]
    
    # If there are no valid segments, return a large value (or handle appropriately)
    if not valid_segments:
        return float('inf')

    # Calculate the minimum distance to all valid line segments
    distances = [distance_point_to_segment(point, seg.start, seg.end) for seg in valid_segments]
    return min(distances)

@dataclass
class IntersectionResult:
    distance: float
    intersected: bool

def closest_plane_intersection(point: Vector3f, normal: Vector3f, triangles: List[Triangle]) -> float:
    """
    Computes the smallest distance to the closest intersection of the given plane and triangles.

    Parameters:
        point (Vector3f): A point on the plane.
        normal (Vector3f): The normal vector of the plane.
        triangles (List[Triangle]): List of triangles to test for intersection.

    Returns:
        float: The smallest distance to the closest intersection. Returns `None` if no intersection is found.
    """
    tolerance = 0.01
    closest_distance = float('inf')

    print("############")

    for triangle in triangles:
        # Get the triangle normal and vertices
        tri_normal = triangle.normal().normalize()
        v0, v1, v2 = triangle.vertices

        # Check if the triangle lies on the plane within the tolerance
        signed_distances = [((v - point).normalize()).dot(normal) for v in [v0, v1, v2]]

        # Case 1: All vertices are within tolerance of the plane
        if all(abs(d) < tolerance for d in signed_distances):
            # Check if the point falls inside the triangle
            if is_point_in_triangle(point, v0, v1, v2):
                return 0.0  # Distance is 0 for this case
                #pass
            continue  # Otherwise, skip this triangle


        # Case 2: Triangle intersects the plane
        if intersects_plane(signed_distances):
            # Compute the intersection line segment and check distances
            intersection_points = compute_plane_triangle_intersection(point, normal, v0, v1, v2)
            if len(intersection_points) == 2: 
                line_segment_projection = project_point_onto_segment(intersection_points[0], intersection_points[1], point)

                # Check if triangle is facing away from the point (also dont consider triangles that dont cover the patch)
                if line_segment_projection != point:
                    origin_to_projection_vector = (line_segment_projection - point).normalize()
                    dot_product = origin_to_projection_vector.dot(tri_normal)
                    if dot_product < 0.99: # and dot_product > -0.5:
                        continue

                

                distance = distance_between_points(point, line_segment_projection)
                if distance < closest_distance:
                    closest_distance = distance

    return closest_distance if closest_distance != float('inf') else None

def point_is_covered_by_triangle(point: Vector3f, normal: Vector3f, triangles: List[Triangle], threshold=1e-8):
    for triangle in triangles:
        if is_triangle_on_plane(triangle, point, normal, threshold=threshold):
            if is_point_in_triangle(point, triangle):
                return True
            else:
                proj1 = project_point_onto_segment(triangle.vertices[0], triangle.vertices[1], point)
                proj2 = project_point_onto_segment(triangle.vertices[1], triangle.vertices[2], point)
                proj3 = project_point_onto_segment(triangle.vertices[2], triangle.vertices[0], point)
                dis1 = distance_between_points(point, proj1)
                dis2 = distance_between_points(point, proj2)
                dis3 = distance_between_points(point, proj3)

                if dis1 < threshold or dis2 < threshold or dis3 < threshold:
                    return True

    
    return False

def point_to_plane_distance(p: Vector3f, point: Vector3f, normal: Vector3f) -> float:
    return abs((p.x - point.x) * normal.x + (p.y - point.y) * normal.y + (p.z - point.z) * normal.z)

def is_triangle_on_plane(triangle: Triangle, point: Vector3f, normal: Vector3f, threshold: float) -> bool:

    v0, v1, v2 = triangle.vertices

    d0 = point_to_plane_distance(v0, point, normal)
    d1 = point_to_plane_distance(v1, point, normal)
    d2 = point_to_plane_distance(v2, point, normal)

    return max(d0, d1, d2) <= threshold

def distance_to_closest_triangle_facing_away(point: Vector3f, plane_normal: Vector3f, intersection_segments: List[LineSegment], intersection_normals: List[Vector3f]) -> float:

    closest_distance = float('inf')

    for i, segment in enumerate(intersection_segments):

        line_segment_projection = project_point_onto_segment(segment.start, segment.end, point)
        
        # Check if triangle is facing away from the point
        if line_segment_projection != point:
            origin_to_projection_vector = (line_segment_projection - point).normalize()
            triangle_normal = intersection_normals[i]
            projected_intersection_normal = project_vector_onto_plane(triangle_normal, plane_normal).normalize()
            dot_product = origin_to_projection_vector.dot(projected_intersection_normal)
            distance = distance_between_points(point, line_segment_projection)
            # if a traingle covers a patch, the origin_to_projection_vector will be close to the triangles normal 
            if dot_product > 0.999 or distance < 0.01:
                closest_distance = min(closest_distance, distance)
        else:
            # as with snapped geometry this may be quite often the case: 
            # -> return 0 to make sure the pixel will counted as illigal to be sure
            # as otherwise floating point inprecision will cause artefacts
            closest_distance = 0


    return closest_distance

def intersects_plane(signed_distances: List[float]) -> bool:
    """Checks if a triangle intersects the plane."""

    # dont use 0.0 as the geometry to avoid false positives due to inprecisions
    has_positive = any(d > 0.01 for d in signed_distances)
    has_negative = any(d < -0.01 for d in signed_distances)
    return has_positive and has_negative

def compute_plane_triangle_intersection2(plane_point: Vector3f, plane_normal: Vector3f, 
                                        v0: Vector3f, v1: Vector3f, v2: Vector3f) -> List[Vector3f]:
    """
    Computes the intersection points of a triangle with a plane.
    
    Parameters:
        plane_point (Vector3f): A point on the plane.
        plane_normal (Vector3f): The normal vector of the plane.
        v0, v1, v2 (Vector3f): The triangle vertices.

    Returns:
        List[Vector3f]: The intersection points (if any).
    """
    def line_plane_intersection(p0, p1):
        """Computes intersection of a line segment with a plane."""
        direction = p1 - p0
        denom = plane_normal.dot(direction)
        if abs(denom) < 1e-4:
            return None  # Line is parallel to the plane
        t = (plane_point - p0).dot(plane_normal) / denom
        if 0 <= t <= 1:
            return p0 + direction * t
        return None

    # Check each edge of the triangle
    intersections = []
    edges = [(v0, v1), (v1, v2), (v2, v0)]
    for edge in edges:
        intersection = line_plane_intersection(*edge)
        if intersection:
            intersections.append(intersection)

    return intersections

def compute_plane_triangle_intersection(plane_point: Vector3f, plane_normal: Vector3f, triangle: Triangle) -> Optional[LineSegment]:
    """
    Computes the intersection points of a triangle with a plane.
    """
    
    def line_plane_intersection(lineSegment: LineSegment):
        """Computes intersection of a line segment with a plane."""
        
        p0 = lineSegment.start
        p1 = lineSegment.end

        direction = p1 - p0
        denom = plane_normal.dot(direction)
        if abs(denom) < 1e-4:
            return None  # Line is parallel to the plane
        t = (plane_point - p0).dot(plane_normal) / denom
        if 0 <= t <= 1:
            return p0 + direction * t
        return None

    # Check each edge of the triangle
    intersections = []
    edges = triangle.get_edges()
    for edge in edges:
        intersection = line_plane_intersection(edge)
        if intersection:
            intersections.append(intersection)
    
    if len(intersections) == 2:
        return LineSegment(intersections[0], intersections[1])
    else:
        return None



def is_point_in_triangle(point: Vector3f, triangle: Triangle) -> bool:
    def vector_sub(v1: Vector3f, v2: Vector3f) -> Vector3f:
        return Vector3f(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z)

    def vector_cross(v1: Vector3f, v2: Vector3f) -> Vector3f:
        return Vector3f(
            v1.y * v2.z - v1.z * v2.y,
            v1.z * v2.x - v1.x * v2.z,
            v1.x * v2.y - v1.y * v2.x
        )

    def vector_dot(v1: Vector3f, v2: Vector3f) -> float:
        return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z

    # Project point onto triangle plane
    v0, v1, v2 = triangle.vertices
    normal = vector_cross(vector_sub(v1, v0), vector_sub(v2, v0))
    d = vector_dot(normal, v0)
    t = (d - vector_dot(normal, point)) / vector_dot(normal, normal)
    projected_point = Vector3f(
        point.x + normal.x * t,
        point.y + normal.y * t,
        point.z + normal.z * t
    )

    # Barycentric coordinates
    edge0 = vector_sub(v1, v0)
    edge1 = vector_sub(v2, v0)
    edge2 = vector_sub(projected_point, v0)

    dot00 = vector_dot(edge0, edge0)
    dot01 = vector_dot(edge0, edge1)
    dot02 = vector_dot(edge0, edge2)
    dot11 = vector_dot(edge1, edge1)
    dot12 = vector_dot(edge1, edge2)

    denom = dot00 * dot11 - dot01 * dot01
    if denom == 0:
        return True

    u = (dot11 * dot02 - dot01 * dot12) / denom
    v = (dot00 * dot12 - dot01 * dot02) / denom

    return u >= 0 and v >= 0 and (u + v) <= 1

def is_point_in_triangles_2d(point: Tuple[float, float], triangles) -> bool:
    for triangle in triangles:
        if is_point_in_triangle_2d(point, triangle):
            if is_point_on_triangle_edge(point, triangle) == False:
                return True
    return False

def is_point_in_triangle_2d(
    point: Tuple[float, float], 
    triangle: Tuple[Tuple[float, float], Tuple[float, float], Tuple[float, float]]
) -> bool:
    """
    Checks if a 2D point is inside a given triangle using the barycentric method.

    Args:
        point (Tuple[float, float]): The (x, y) coordinates of the point.
        triangle (Tuple[Tuple[float, float], Tuple[float, float], Tuple[float, float]]): 
            The vertices of the triangle, each defined as (x, y).

    Returns:
        bool: True if the point is inside the triangle, False otherwise.
    """
    def sign(p1, p2, p3):
        return (p1[0] - p3[0]) * (p2[1] - p3[1]) - (p2[0] - p3[0]) * (p1[1] - p3[1])

    v1, v2, v3 = triangle
    d1 = sign(point, v1, v2)
    d2 = sign(point, v2, v3)
    d3 = sign(point, v3, v1)

    has_neg = (d1 < 0) or (d2 < 0) or (d3 < 0)
    has_pos = (d1 > 0) or (d2 > 0) or (d3 > 0)

    return not (has_neg and has_pos)


def is_point_on_triangle_edge(
    point: Tuple[float, float], 
    triangle: Tuple[Tuple[float, float], Tuple[float, float], Tuple[float, float]], 
    tolerance: float = 1e-8
) -> bool:
    """Check if a 2D point lies on the edge of a 2D triangle.

    Args:
        point (Tuple[float, float]): The point to check (x, y).
        triangle (Tuple[Tuple[float, float], Tuple[float, float], Tuple[float, float]]): 
            The vertices of the triangle as ((x1, y1), (x2, y2), (x3, y3)).
        tolerance (float): Tolerance for numerical imprecision.

    Returns:
        bool: True if the point lies on an edge of the triangle, otherwise False.
    """

    def is_point_on_line_segment(p: Tuple[float, float], a: Tuple[float, float], b: Tuple[float, float]) -> bool:
        """Check if point `p` lies on the line segment `ab`."""
        # Calculate cross product to check collinearity
        cross_product = (b[1] - a[1]) * (p[0] - a[0]) - (b[0] - a[0]) * (p[1] - a[1])
        if abs(cross_product) > tolerance:
            return False  # Not collinear

        # Check if the point lies within the bounds of the segment
        min_x, max_x = min(a[0], b[0]), max(a[0], b[0])
        min_y, max_y = min(a[1], b[1]), max(a[1], b[1])
        return min_x - tolerance <= p[0] <= max_x + tolerance and min_y - tolerance <= p[1] <= max_y + tolerance

    # Extract triangle vertices
    v0, v1, v2 = triangle

    # Check if the point is on any of the triangle's edges
    return (is_point_on_line_segment(point, v0, v1) or
            is_point_on_line_segment(point, v1, v2) or
            is_point_on_line_segment(point, v2, v0))
    
def project_point_onto_segment(segment_start: Vector3f, segment_end: Vector3f, point: Vector3f) -> Vector3f:
    """
    Projects a point onto a line segment defined by two points.

    Args:
        segment_start (Vector3f): Start point of the line segment.
        segment_end (Vector3f): End point of the line segment.
        point (Vector3f): The point to project onto the line segment.

    Returns:
        Vector3f: The projected point on the line segment.
    """
    # Segment vector
    segment_vector = Vector3f(
        segment_end.x - segment_start.x,
        segment_end.y - segment_start.y,
        segment_end.z - segment_start.z
    )
    
    # Point vector (from segment start to the point)
    point_vector = Vector3f(
        point.x - segment_start.x,
        point.y - segment_start.y,
        point.z - segment_start.z
    )
    
    # Compute the segment length squared
    segment_length_squared = (
        segment_vector.x ** 2 +
        segment_vector.y ** 2 +
        segment_vector.z ** 2
    )
    
    # If the segment length is 0, return the start point
    if segment_length_squared == 0:
        return segment_start
    
    # Compute the projection scalar (dot product of point_vector and segment_vector normalized by segment length)
    t = (
        (point_vector.x * segment_vector.x +
         point_vector.y * segment_vector.y +
         point_vector.z * segment_vector.z) /
        segment_length_squared
    )
    
    # Clamp t to the range [0, 1] to stay within the segment
    t = max(0, min(1, t))
    
    # Compute the projected point
    projected_point = Vector3f(
        segment_start.x + t * segment_vector.x,
        segment_start.y + t * segment_vector.y,
        segment_start.z + t * segment_vector.z
    )
    
    return projected_point

def distance_between_points(point1: Vector3f, point2: Vector3f) -> float:
    """
    Calculates the distance between two points in 3D space.

    Args:
        point1 (Vector3f): The first point.
        point2 (Vector3f): The second point.

    Returns:
        float: The distance between the two points.
    """
    return math.sqrt(
        (point2.x - point1.x) ** 2 +
        (point2.y - point1.y) ** 2 +
        (point2.z - point1.z) ** 2
    )

def signed_distance_to_plane(point: Vector3f, plane_point: Vector3f, plane_normal: Vector3f) -> float:
    """
    Computes the signed perpendicular distance from a point to a plane.
    Positive if the point is in the direction of the plane normal, negative otherwise.
    :param point: Vector3f to compute distance from
    :param plane_point: A point on the plane
    :param plane_normal: Normal vector of the plane (assumed normalized)
    :return: Signed distance from the point to the plane
    """
    # Vector from the plane point to the given point
    vec = Vector3f(point.x - plane_point.x, point.y - plane_point.y, point.z - plane_point.z)
    # Dot product with the plane's normal gives the signed distance
    return vec.x * plane_normal.x + vec.y * plane_normal.y + vec.z * plane_normal.z

def project_vector_onto_plane(vector: Vector3f, plane_normal: Vector3f) -> Vector3f:
    """
    Projects a vector onto a plane defined by a normal vector.

    :param vector: The vector to project.
    :param plane_normal: The normal of the plane (does not need to be normalized).
    :return: The projected vector.
    """
    # Ensure the plane normal is normalized
    plane_normal_normalized = plane_normal.normalize()

    # Compute the component of the vector along the plane normal
    parallel_component = plane_normal_normalized * vector.dot(plane_normal_normalized)

    # Subtract the parallel component from the original vector to get the projection onto the plane
    projected_vector = vector - parallel_component
    return projected_vector




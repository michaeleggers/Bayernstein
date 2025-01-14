import numpy as np
import math
from typing import List, Tuple

from data_structures.triangle import Triangle
from data_structures.vector3f import Vector3f
from data_structures.line_segment import LineSegment
from data_structures.bounding_box import BoundingBox
from data_structures.point_light import PointLight
from util import geometry

class Frame:
    
    # Initialization fields
    triangles: List[Triangle]
    triangles_indices: List[int]
    patch_ws_size = 0
    
    projected_triangles: List[Tuple[Tuple[float]]]
    frame_normal: Vector3f
    bounding_box: Tuple[float, float, float, float] # (origin_x, origin_y, width, height)
    bounding_box_3d: BoundingBox                    # (origin_x, origin_y, width, height)
    lm_uvs: List[Tuple[float, float]]
    lm_uvs_bbox_space: List[Tuple[float, float]]
    
    # Frame intersections (for illegal pixel calculations)
    close_triangles: List[Triangle]
    intersection_segments: List[LineSegment] = []
    intersection_normals: List[Vector3f] = []

    frame_u_start: int
    frame_v_start: int
    frame_u_end: int
    frame_v_end: int

    legal_pixels: List[Tuple[int, int]]
    legal_positions: List[Vector3f]
    legal_normals: List[Vector3f]
    illegal_pixels: List[Tuple[int, int]]



    def __init__(
        self,
        triangles: List[Triangle],
        triangles_indices: List[int],
        patch_ws_size: float
    ):
        # The triangles assigned to this frame
        self.triangles = triangles
        # The indices of this frame's triangles in the scene's triangles list
        self.triangles_indices = triangles_indices
        # How large is a patch?
        self.patch_ws_size = patch_ws_size

        # All the (coplanar) triangles are projected onto their shared plane. 
        # The projected triangles are used to calculate the frame's size (bbox) within its plane which is necessary for the uv mapping
        # The uv mapped frame is the basis for the patch placement
        self.projected_triangles, self.projection_matrix = self.__project_triangles_to_2d(self.triangles)
        self.bounding_box_unpadded = self._calculate_bounding_box_2d()
        frame_width, frame_height = self.bounding_box_unpadded[2], self.bounding_box_unpadded[3]
        # As right now its assumed that all triangles within a frame are coplanar we can just pick the normal of any triangle
        self.frame_normal = self.triangles[0].normal

        # Add padding to the bounding box to ensure no light leakage between neighbouring frames
        self.bounding_box = (
            self.bounding_box_unpadded[0] - self.patch_ws_size, 
            self.bounding_box_unpadded[1] - self.patch_ws_size, 
            self.bounding_box_unpadded[2] + self.patch_ws_size + self.patch_ws_size,
            self.bounding_box_unpadded[3] + self.patch_ws_size + self.patch_ws_size
        )

        # Adjust projected triangles based on bounding box origin (also consider padding)
        self.projected_triangles = [
            [[v[0] - self.bounding_box_unpadded[0] + self.patch_ws_size, v[1] - self.bounding_box_unpadded[1] + self.patch_ws_size] for v in triangle]
            for triangle in self.projected_triangles
        ]

        # Calculate this frame's 3d bounding box
        self.bounding_box_3d = self.__calculate_bounding_box_3d()

        # Initialize this frame's uv lists
        self.tex_uvs = []
        self.lm_uvs = []
        self.lm_uvs_bbox_space = []

        
        self.frame_u_start = 0
        self.frame_v_start = 0
        self.frame_u_end = 0
        self.frame_v_end = 0
        self.frame_width_pixels = 0
        self.frame_height_pixels = 0


    def __project_triangles_to_2d(self, triangles: List[Triangle]) -> Tuple[List[Tuple[Tuple[float, float], Tuple[float, float], Tuple[float, float]]], np.ndarray]:
        """
        Projects the triangles onto their plane, it's assumed all triangles passed are coplanar. Returns the projection matrix used.
        """
        
        if not triangles:
            return [], np.eye(4)  # No triangles, return identity matrix

        # Use the first triangle to compute the plane basis
        v0 = triangles[0].vertices[0].to_array()
        v1 = triangles[0].vertices[1].to_array()
        v2 = triangles[0].vertices[2].to_array()

        # Compute two edges of the first triangle
        edge1 = v1 - v0
        edge2 = v2 - v0

        # Compute the normal of the plane
        normal = np.cross(edge1, edge2)
        normal = normal / np.linalg.norm(normal)  # Normalize the normal vector

        # Find two orthogonal vectors (u and v) in the plane
        u = edge1 / np.linalg.norm(edge1)
        v = np.cross(normal, u)
        v = v / np.linalg.norm(v)

        # Create a 4x4 projection matrix
        projection_matrix = np.eye(4)
        projection_matrix[:3, 0] = u  # X-axis basis vector
        projection_matrix[:3, 1] = v  # Y-axis basis vector
        projection_matrix[:3, 2] = normal  # Z-axis (plane normal)
        projection_matrix[:3, 3] = v0  # Translation to origin of the plane

        # Invert the matrix to transform global coordinates to plane-local coordinates
        projection_matrix = np.linalg.inv(projection_matrix)

        # Project all triangles
        projected_triangles = []
        for triangle in triangles:
            projected_triangle = tuple(
                self.project_point_to_plane(vertex.to_array(), projection_matrix) for vertex in triangle.vertices
            )
            projected_triangles.append(projected_triangle)

        return projected_triangles, projection_matrix
    

    def __calculate_bounding_box_3d(self) -> BoundingBox:
        """
        Calculate the 3D bounding box that encompasses all the triangles ind 3D worldspace.
        """

        if not self.triangles:
            raise ValueError("The list of triangles is empty.")

        min_x = min(vertex.x for triangle in self.triangles for vertex in triangle.vertices)
        min_y = min(vertex.y for triangle in self.triangles for vertex in triangle.vertices)
        min_z = min(vertex.z for triangle in self.triangles for vertex in triangle.vertices)
        max_x = max(vertex.x for triangle in self.triangles for vertex in triangle.vertices)
        max_y = max(vertex.y for triangle in self.triangles for vertex in triangle.vertices)
        max_z = max(vertex.z for triangle in self.triangles for vertex in triangle.vertices)

        return BoundingBox(
            min=Vector3f(min_x, min_y, min_z),
            max=Vector3f(max_x, max_y, max_z),
        )

    def _calculate_bounding_box_2d(self) -> Tuple[float, float, float, float]:
        """
        Calculate the 2D bounding box that encompasses all the projected triangles.
        """

        min_x, min_y = float('inf'), float('inf')
        max_x, max_y = float('-inf'), float('-inf')

        for projected in self.projected_triangles:
            for x, y in projected:
                min_x = min(min_x, x)
                min_y = min(min_y, y)
                max_x = max(max_x, x)
                max_y = max(max_y, y)

        width = max(max_x - min_x, self.patch_ws_size)
        height = max(max_y - min_y, self.patch_ws_size)
        return min_x, min_y, width, height
    
    def project_point_to_plane(self, point: np.ndarray, projection_matrix: np.ndarray) -> np.ndarray:
        """
        Project a 3D point onto the plane using the projection matrix.

        Parameters:
        - point: A 3D point represented as a numpy array (x, y, z).
        - projection_matrix: The 4x4 projection matrix to project the point onto the plane.

        Returns:
        - A 2D point in the plane's local coordinate system.
        """

        # Convert the point to homogeneous coordinates (x, y, z, 1)
        homogeneous_point = np.append(point, 1)  # Now the point is a 4D vector

        # Perform the projection using the transformation matrix
        projected_point_homogeneous = np.dot(projection_matrix, homogeneous_point)

        # Normalize by dividing by the w component to get the actual x, y (homogeneous -> Cartesian)
        w = projected_point_homogeneous[3]
        if w != 0:  # Ensure no division by zero
            projected_point = projected_point_homogeneous[:2] / w  # Return the 2D point (x, y)
        else:
            raise ValueError("The projected point's w component is zero, which indicates an invalid projection.")

        return projected_point
    
    def calculate_intersections(self, all_triangles: List[Triangle]):

        threshold = 0.1

        # Exclude triangles within the current frame
        external_triangles = [tri for tri in all_triangles if tri not in self.triangles]
        # Exclude triangles not intersecting wiht this frames bbox
        self.close_triangles = [tri for tri in external_triangles if self.bounding_box_3d.intersects(tri.bounding_box, tolerance=self.patch_ws_size)]

        intersection_segments = []
        intersection_normals = []

        for tri1 in self.triangles:
            normal = tri1.normal
            center = tri1.triangle_center()
            for tri2 in self.close_triangles:
                # Bounding box overlap check for faster rejection
                if not tri1.bounding_box.intersects(tri2.bounding_box, tolerance=self.patch_ws_size):
                    continue

                # Calculate signed distances of tri2's vertices to tri1's plane
                distances = [geometry.signed_distance_to_plane(v, center, normal) for v in tri2.vertices]
                # Check if at least one distance is negative and at least one distance is positive
                has_negative = any(d < -threshold for d in distances)
                has_positive = any(d > threshold for d in distances)
                if not (has_negative and has_positive):
                    continue  # Skip, as this means no actual edge-plane intersection

                # Check for actual tri2 intersection with tri1's plane
                intersection_segment = geometry.compute_plane_triangle_intersection(center, normal, tri2)

                if intersection_segment and self.line_segment_intersects_triangle(intersection_segment.start, intersection_segment.end, tri1):
                    intersection_segments.append(intersection_segment)
                    intersection_normals.append(tri2.normal)

        self.intersection_segments = intersection_segments
        self.intersection_normals = intersection_normals


    def line_segment_intersects_triangle(
        self,
        line_start: Vector3f, 
        line_end: Vector3f, 
        triangle: Triangle, 
        tolerance: float = 1e-4
    ) -> bool:
        """Check if a line segment intersects a triangle in 3D space."""
        
        def is_point_in_triangle(pt: Vector3f, v0: Vector3f, v1: Vector3f, v2: Vector3f) -> bool:
            """Check if a point is strictly inside a triangle using barycentric coordinates."""
            v0v1 = v1 - v0
            v0v2 = v2 - v0
            v0p = pt - v0

            # Compute dot products
            dot00 = v0v2.dot(v0v2)
            dot01 = v0v2.dot(v0v1)
            dot02 = v0v2.dot(v0p)
            dot11 = v0v1.dot(v0v1)
            dot12 = v0v1.dot(v0p)

            # Compute barycentric coordinates
            denom = dot00 * dot11 - dot01 * dot01
            if abs(denom) < tolerance:
                return False  # Degenerate triangle
            inv_denom = 1 / denom
            u = (dot11 * dot02 - dot01 * dot12) * inv_denom
            v = (dot00 * dot12 - dot01 * dot02) * inv_denom

            # Check if point is inside the triangle but not on edges
            return u > tolerance and v > tolerance and (u + v) < (1 - tolerance)

        def edge_intersects_segment(edge_start: Vector3f, edge_end: Vector3f, seg_start: Vector3f, seg_end: Vector3f, tolerance: float = 1e-6) -> bool:
            """Check if a segment intersects a triangle edge, excluding overlaps, robustly."""
            edge_dir = edge_end - edge_start
            seg_dir = seg_end - seg_start
            cross = edge_dir.cross(seg_dir)
            cross_mag_squared = cross.dot(cross)  # Avoid unnecessary square root for magnitude
            
            # If cross product is very small, lines are parallel or nearly so
            if cross_mag_squared < tolerance**2:
                return False

            # Solve for intersection parameters (t, u) using the cross product
            edge_to_seg_start = seg_start - edge_start
            t_numerator = edge_to_seg_start.cross(seg_dir).dot(cross)
            u_numerator = edge_to_seg_start.cross(edge_dir).dot(cross)

            # Normalize by the magnitude of the cross product squared
            t = t_numerator / cross_mag_squared
            u = u_numerator / cross_mag_squared

            # Check intersection lies strictly between segment and edge endpoints
            return (t > -tolerance and t < 1 + tolerance and 
                    u > -tolerance and u < 1 + tolerance)

        # Extract vertices
        v0, v1, v2 = triangle.vertices

        # Check if either endpoint of the line segment is inside the triangle
        if is_point_in_triangle(line_start, v0, v1, v2) or is_point_in_triangle(line_end, v0, v1, v2):
            return True

        # Check if the line segment intersects any triangle edge
        if (edge_intersects_segment(v0, v1, line_start, line_end) or
            edge_intersects_segment(v1, v2, line_start, line_end) or
            edge_intersects_segment(v2, v0, line_start, line_end)):
            return True

        return False

    def generate_patches(self, patch_resolution):

        self.legal_pixels = []
        self.legal_positions = []
        self.legal_normals = []
        self.legal_incoming_light = []
        self.illegal_pixels = []

        frame_width_ws = self.bounding_box[2]
        frame_height_ws = self.bounding_box[3]
        frame_origin_u_pixels = math.ceil(self.bounding_box[0] * patch_resolution)
        frame_origin_v_pixels = math.ceil(self.bounding_box[1] * patch_resolution)
        self.frame_width_pixels = math.ceil(frame_width_ws * patch_resolution)
        self.frame_height_pixels = math.ceil(frame_height_ws * patch_resolution)

        # The frames position within the uv map in pixels
        frame_origin_u_pixels = math.ceil(self.bounding_box[0] * patch_resolution)
        frame_origin_v_pixels = math.ceil(self.bounding_box[1] * patch_resolution)
        self.frame_u_start = int(frame_origin_u_pixels)
        self.frame_v_start = int(frame_origin_v_pixels)
        self.frame_u_end = self.frame_u_start + int(self.frame_width_pixels)
        self.frame_v_end = self.frame_v_start + int(self.frame_height_pixels)

        half_u_pixel = (1 / self.frame_width_pixels) / 2
        half_v_pixel = (1 / self.frame_height_pixels) / 2

        triangles_uv_space = [triangle for triangle in self.lm_uvs_bbox_space]

        for v_pixel in range(self.frame_height_pixels):
            v = v_pixel / (self.frame_height_pixels)
            for u_pixel in range(self.frame_width_pixels):
                u = u_pixel / (self.frame_width_pixels)

                if geometry.is_point_in_triangles_2d((u, v), triangles_uv_space):
                            
                    worldspace_position = self.interpolate_uv_to_world(
                        self.triangles[0],
                        self.lm_uvs_bbox_space[0],
                        (u + half_u_pixel, v + half_v_pixel)
                    )

                    # Check if patch is covered by triangle on the same plane
                    if geometry.point_is_covered_by_triangle(worldspace_position, self.frame_normal, self.close_triangles, 1e-6):
                        self.illegal_pixels.append((u_pixel, v_pixel))
                        continue
                    
                    # Check if there is a intersectin triangle which covers this patch
                    distance = geometry.distance_to_closest_triangle_facing_away(worldspace_position, self.frame_normal, self.intersection_segments, self.intersection_normals)
                    if distance < 1/patch_resolution *  2:
                        self.illegal_pixels.append((u_pixel, v_pixel))
                        continue

                    self.legal_pixels.append((u_pixel, v_pixel))
                    self.legal_positions.append(worldspace_position)
                    self.legal_normals.append(self.frame_normal)
                    self.legal_incoming_light.append(Vector3f(0, 0, 0))

                else:
                    self.illegal_pixels.append((u_pixel, v_pixel))


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
    
    def calculate_incoming_light(self, lights: List[PointLight], triangles: List[Triangle]):
        for i, patch_position in enumerate(self.legal_positions):
            incoming_light = Vector3f(0, 0, 0)

            for point_light in lights:
                light_dir = point_light.origin - patch_position
                light_distance = light_dir.magnitude()
                light_dir = light_dir.normalize()


                if light_distance > point_light.range:
                    continue

                blocked = False

                for triangle in triangles:
                    if any((patch_position - v).magnitude() < 1e-6 for v in triangle.vertices):
                        continue  # Skip the triangle the patch is on

                    t = triangle.intersect_ray(point_light.origin, -light_dir)
                    if t is not None and t < light_distance - 1e-6:
                        blocked = True
                        break

                if not blocked:
                    attenuation = max(0.0, float(point_light.intensity / (light_distance ** 2)))
                    incoming_light_x = attenuation * max(0.0, light_dir.dot(self.frame_normal)) * point_light.color.x
                    incoming_light_y = attenuation * max(0.0, light_dir.dot(self.frame_normal)) * point_light.color.y
                    incoming_light_z = attenuation * max(0.0, light_dir.dot(self.frame_normal)) * point_light.color.z
                    incoming_light_vec = Vector3f(incoming_light_x, incoming_light_y, incoming_light_z)
                    incoming_light += incoming_light_vec

            self.legal_incoming_light[i] = incoming_light

        

    


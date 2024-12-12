from dataclasses import dataclass
from typing import List, Optional, Tuple
from data_structures.triangle import Triangle
import numpy as np
from scipy.spatial import ConvexHull
from data_structures.vector3f import Vector3f
from data_structures.line_segment import LineSegment
from data_structures.bounding_box import BoundingBox
from util import geometry

class Frame:
    triangles: List[Triangle]
    triangles_indices: List[int]
    patch_ws_size = 0
    projected_triangles: List[Tuple[Tuple[float]]]
    bounding_box: Tuple[float, float, float, float]  # (origin_x, origin_y, width, height)
    bounding_box_3d: BoundingBox
    lm_uvs: List[Tuple[float, float]]
    lm_uvs_bbox_space: List[Tuple[float, float]]
    frame_normal: Vector3f

    # Frame intersections for illegal pixel calculations
    close_triangles: List[Triangle]
    intersection_segments: List[LineSegment] = []
    intersection_normals: List[Vector3f] = []


    def __init__(
        self,
        triangles: List[Triangle],
        triangles_indices: List[int],
        patch_ws_size: float
    ):
        self.triangles = triangles
        self.triangles_indices = triangles_indices
        self.patch_ws_size = patch_ws_size
        self.projected_triangles, self.projection_matrix = self.__project_triangles_to_2d(self.triangles)
        self.bounding_box_unpadded = self._calculate_bounding_box()

        self.frame_normal = self.triangles[0].normal()


        # Check if the height is less than or equal to the width
        width, height = self.bounding_box_unpadded[2], self.bounding_box_unpadded[3]
        if width > height:
            # Rotate the bounding box (flip width and height)
            self.bounding_box_unpadded = (self.bounding_box_unpadded[0], self.bounding_box_unpadded[1], height, width)

            # Rotate the projected triangles (flip x and y)
            self.projected_triangles = [
                [[v[1], v[0]] for v in triangle] for triangle in self.projected_triangles
            ]

            # Flip X and Y axes in the projection matrix by swapping rows 0 and 1
            flipped_projection_matrix = self.projection_matrix.copy()
            flipped_projection_matrix[[0, 1], :] = flipped_projection_matrix[[1, 0], :]
            self.projection_matrix = flipped_projection_matrix
        
        # Adjust projected triangles based on bounding box origin (also consider padding)
        self.projected_triangles = [
            [[v[0] - self.bounding_box_unpadded[0] + self.patch_ws_size, v[1] - self.bounding_box_unpadded[1] + self.patch_ws_size] for v in triangle]
            for triangle in self.projected_triangles
        ]

        # Add padding to the bounding box
        self.bounding_box = (
            self.bounding_box_unpadded[0], 
            self.bounding_box_unpadded[1], 
            self.bounding_box_unpadded[2] + self.patch_ws_size + self.patch_ws_size, 
            self.bounding_box_unpadded[3] + self.patch_ws_size + self.patch_ws_size
        )
        
        # Calculate the 3d bounding box based on the triangles projection
        self.bounding_box_3d = self.__calculate_bounding_box_3d()

        self.tex_uvs = []
        self.lm_uvs = []
        self.lm_uvs_bbox_space = []

    
    def __project_triangles_to_2d(
        self, triangles: List[Triangle]
    ) -> Tuple[
        List[Tuple[Tuple[float, float], Tuple[float, float], Tuple[float, float]]], np.ndarray
    ]:
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

        # Function to project a vertex to 2D
        def project_vertex(vertex: np.ndarray) -> Tuple[float, float]:
            relative_position = vertex - v0  # Translate to origin
            x = np.dot(relative_position, u)  # Projection onto u
            y = np.dot(relative_position, v)  # Projection onto v
            return (x, y)

        # Project all triangles
        projected_triangles = []
        for triangle in triangles:
            projected_triangle = tuple(
                self.project_point_to_plane(vertex.to_array(), projection_matrix) for vertex in triangle.vertices
            )
            projected_triangles.append(projected_triangle)

        return projected_triangles, projection_matrix
    

    def __calculate_bounding_box_3d(self) -> BoundingBox:
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

    def _calculate_bounding_box(self) -> Tuple[float, float, float, float]:
        """Calculate the 2D bounding box that encompasses all the triangles."""
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
    
    def calculate_uv_coordinates(self, point: Tuple[float, float], bbox: Tuple[float, float, float, float]) -> Tuple[float, float]:
        """
        Calculate UV coordinates for a 2D point inside a bounding box.

        Parameters:
        - point: The 2D point (x, y).
        - bbox: The bounding box (minx, miny, width, height).

        Returns:
        - The UV coordinates (u, v), normalized to [0, 1].
        """
        minx, miny, width, height = bbox
        x, y = point

        # Normalize the point inside the bounding box
        u = (x - minx) / width
        v = (y - miny) / height

        return (u, v)

        
    def calculate_intersections(self, all_triangles: List[Triangle]):

        threshold = 0.1

        # Exclude triangles within the current frame
        external_triangles = [tri for tri in all_triangles if tri not in self.triangles]
        # Exclude triangles not intersecting wiht this frames bbox
        self.close_triangles = [tri for tri in external_triangles if self.bounding_box_3d.intersects(tri.bounding_box)]

        intersection_segments = []
        intersection_normals = []

        for tri1 in self.triangles:
            normal = tri1.normal()
            center = tri1.triangle_center()
            for tri2 in self.close_triangles:
                # Bounding box overlap check for faster rejection
                if not tri1.bounding_box.intersects(tri2.bounding_box):
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

                if intersection_segment:
                    intersection_segments.append(intersection_segment)
                    intersection_normals.append(tri2.normal())

        self.intersection_segments = intersection_segments
        self.intersection_normals = intersection_normals

    



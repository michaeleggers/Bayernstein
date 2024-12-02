from dataclasses import dataclass
from typing import List, Optional, Tuple
from data_structures.triangle import Triangle
import numpy as np
from scipy.spatial import ConvexHull
from data_structures.vector3f import Vector3f

@dataclass
class Frame:
    triangles: List[Triangle]
    triangles_indices: List[int]
    patch_ws_size = 0
    projected_triangles: List[Tuple[Tuple[float]]]
    bounding_box: Tuple[float, float, float, float]  # (origin_x, origin_y, width, height)
    lm_uvs: List[Tuple[float, float]]
    lm_uvs_bbox_space: List[Tuple[float, float]]
    frame_ws_min: Vector3f
    frame_ws_max: Vector3f
    frame_normal: Vector3f


    def __init__(
        self,
        triangles: List[Triangle],
        triangles_indices: List[int],
        patch_ws_size: float
    ):
        self.triangles = triangles
        self.triangles_indices = triangles_indices
        self.patch_ws_size = patch_ws_size
        self.projected_triangles = self.__project_triangles_to_2d(self.triangles)
        self.bounding_box = self._calculate_bounding_box()

        # Check if the height is less than or equal to the width
        width, height = self.bounding_box[2], self.bounding_box[3]
        if width > height:
            # Rotate the bounding box (flip width and height)
            self.bounding_box = (self.bounding_box[0], self.bounding_box[1], height, width)

            # Rotate the projected triangles (flip x and y)
            self.projected_triangles = [
                [[v[1], v[0]] for v in triangle] for triangle in self.projected_triangles
            ]
        
        # Adjust projected triangles based on bounding box origin (also consider padding)
        self.projected_triangles = [
            [[v[0] - self.bounding_box[0] + self.patch_ws_size, v[1] - self.bounding_box[1] + self.patch_ws_size] for v in triangle]
            for triangle in self.projected_triangles
        ]

        # Add padding to the bounding box
        self.bounding_box = (
            self.bounding_box[0], 
            self.bounding_box[1], 
            self.bounding_box[2] + self.patch_ws_size + self.patch_ws_size, 
            self.bounding_box[3] + self.patch_ws_size + self.patch_ws_size
        )
        
        # Calculate the 3d bounding box based on the triangles projection
        self.frame_ws_min, self.frame_ws_max, self.frame_normal = self.__calculate_bbox_3d()

        self.tex_uvs = []
        self.lm_uvs = []
        self.lm_uvs_bbox_space = []

    
    def __project_triangles_to_2d(self, triangles: List[Triangle]) -> List[Tuple[Tuple[float, float], Tuple[float, float], Tuple[float, float]]]:
        if not triangles:
            return []

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
                project_vertex(vertex.to_array()) for vertex in triangle.vertices
            )
            projected_triangles.append(projected_triangle)

        return projected_triangles
    

    def __calculate_bbox_3d(self) -> Tuple[Vector3f, Vector3f]:
        """Calculate the 3D min and max points of a 2D bounding box projected back to the original 3D plane."""

        if not self.triangles:
            raise ValueError("Triangles list is empty.")

        # Use the first triangle to compute the plane basis
        v0 = self.triangles[0].vertices[0].to_array()
        v1 = self.triangles[0].vertices[1].to_array()
        v2 = self.triangles[0].vertices[2].to_array()

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

        # Extract 2D bounding box values
        origin_x, origin_y, width, height = self.bounding_box
        min_2d = (origin_x, origin_y)
        max_2d = (origin_x + width, origin_y + height)

        # Function to reproject a 2D point back into 3D space
        def reproject_to_3d(point_2d: Tuple[float, float]) -> np.ndarray:
            x, y = point_2d
            return v0 + x * u + y * v

        # Compute 3D coordinates of the bounding box corners
        min_3d = reproject_to_3d(min_2d)
        max_3d = reproject_to_3d(max_2d)

        return Vector3f(min_3d[0], min_3d[1], min_3d[2]), Vector3f(max_3d[0], max_3d[1], max_3d[2]), Vector3f(normal[0], normal[1], normal[2])

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

        return min_x, min_y, max_x - min_x, max_y - min_y

    



from dataclasses import dataclass
from typing import List, Optional, Tuple
from data_structures.triangle import Triangle
import numpy as np
from scipy.spatial import ConvexHull

@dataclass
class Shape:
    triangles: List[Triangle]
    projected_triangles: List[Tuple[Tuple[float]]]
    bounding_box: Tuple[float, float, float, float]  # (origin_x, origin_y, width, height)
    uvs: List[Tuple[float, float]] 

    def __init__(
        self,
        triangles: List[Triangle],
        projected_triangles: List[Tuple[Tuple[float, float]]] = None,
        bounding_box: Tuple[float, float, float, float] = None
    ):
        self.triangles = triangles
        
        if projected_triangles is None:
            # Generate projected triangles from 3D data
            self.projected_triangles = self.__project_triangles_to_2d(self.triangles)
        else:
            # Use precomputed projections
            self.projected_triangles = projected_triangles

        if bounding_box is None:
            # Calculate bounding box if not provided
            self.bounding_box = self._calculate_bounding_box()
        else:
            # Use precomputed bounding box
            self.bounding_box = bounding_box

        # Adjust projected triangles based on bounding box origin
        self.projected_triangles = [[[v[0] - self.bounding_box[0], v[1] - self.bounding_box[1]] for v in triangle]
                                    for triangle in self.projected_triangles]
        self.uvs = []

    
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

    def __project_to_2d(self, triangle: Triangle):
        """ Projects the 3D triangle onto a 2D plane based on its normal """
        v0, v1, v2 = triangle.vertices
        v0 = v0.to_array()
        v1 = v1.to_array()
        v2 = v2.to_array()
        # Calculate the normal of the triangle
        normal = np.cross(np.subtract(v1, v0), np.subtract(v2, v0))
        normal = normal / np.linalg.norm(normal)  # Normalize the normal

        # Find the dominant axis of the normal to project onto 2D
        abs_normal = np.abs(normal)
        if abs_normal[0] > abs_normal[1] and abs_normal[0] > abs_normal[2]:
            # Dominant axis is X, project onto YZ plane
            projected = [(v.y, v.z) for v in triangle.vertices]
        elif abs_normal[1] > abs_normal[0] and abs_normal[1] > abs_normal[2]:
            # Dominant axis is Y, project onto XZ plane
            projected = [(v.x, v.z) for v in triangle.vertices]
        else:
            # Dominant axis is Z, project onto XY plane
            projected = [(v.x, v.y) for v in triangle.vertices]
        
        return projected

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

    



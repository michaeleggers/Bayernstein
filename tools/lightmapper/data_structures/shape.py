from dataclasses import dataclass
from typing import List, Optional, Tuple
from data_structures.triangle import Triangle
import numpy as np

@dataclass
class Shape:
    triangles: List[Triangle]
    projected_triangles: List[Tuple[Tuple[float]]]
    bounding_box: Tuple[float, float, float, float]  # (min_x, min_y, max_x, max_y)
    uvs: List[Tuple[float, float]] 

    def __init__(self, triangles: List[Triangle]):
        self.triangles = triangles
        self.projected_triangles = [self.__project_to_2d(tri) for tri in triangles]
        self.bounding_box = self._calculate_bounding_box()
        self.uvs = []

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

        return 0, 0, max_x - min_x, max_y - min_y


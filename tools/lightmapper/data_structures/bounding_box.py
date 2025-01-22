from dataclasses import dataclass
from data_structures.vector3f import Vector3f
from typing import List
import numpy as np

@dataclass
class BoundingBox:
    min: Vector3f
    max: Vector3f

    def intersects(self, other: 'BoundingBox', tolerance: float = 0.0) -> bool:
        """
        Check if this bounding box intersects with another bounding box.
        """
        return (
            self.min.x - tolerance <= other.max.x and self.max.x + tolerance >= other.min.x and
            self.min.y - tolerance <= other.max.y and self.max.y + tolerance >= other.min.y and
            self.min.z - tolerance <= other.max.z and self.max.z + tolerance >= other.min.z
        )

    def ray_intersects(self, ray_origin: Vector3f, ray_direction: Vector3f) -> bool:
        """
        Perform a ray-box intersection test.
        
        Args:
            ray_origin (Vector3f): Origin of the ray.
            ray_direction (Vector3f): Direction of the ray.

        Returns:
            bool: True if the ray intersects the bounding box, False otherwise.
        """
        t_min = (np.array([self.min.x, self.min.y, self.min.z]) - np.array([ray_origin.x, ray_origin.y, ray_origin.z])) / \
                (np.array([ray_direction.x, ray_direction.y, ray_direction.z]) + 1e-6)
        t_max = (np.array([self.max.x, self.max.y, self.max.z]) - np.array([ray_origin.x, ray_origin.y, ray_origin.z])) / \
                (np.array([ray_direction.x, ray_direction.y, ray_direction.z]) + 1e-6)
        t1 = np.minimum(t_min, t_max)
        t2 = np.maximum(t_min, t_max)
        t_enter = np.max(t1)
        t_exit = np.min(t2)
        return t_enter <= t_exit and t_exit >= 0



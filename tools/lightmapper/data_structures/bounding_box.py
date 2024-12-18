from dataclasses import dataclass
from data_structures.vector3f import Vector3f

@dataclass
class BoundingBox:
    min: Vector3f
    max: Vector3f

    def intersects(self, other: 'BoundingBox', tolerance: float = 0.0) -> bool:
        return (
            self.min.x - tolerance <= other.max.x and self.max.x + tolerance >= other.min.x and
            self.min.y - tolerance <= other.max.y and self.max.y + tolerance >= other.min.y and
            self.min.z - tolerance <= other.max.z and self.max.z + tolerance >= other.min.z
        )

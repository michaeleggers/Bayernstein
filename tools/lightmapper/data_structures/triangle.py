from dataclasses import dataclass, field
from typing import List, Optional, Tuple
from data_structures.vector3f import Vector3f
from data_structures.bounding_box import BoundingBox
from data_structures.line_segment import LineSegment

@dataclass
class Triangle:
    vertices: Tuple[Vector3f, Vector3f, Vector3f]
    bounding_box: BoundingBox = field(init=False)
    normal: Vector3f = field(init=False)

    def __post_init__(self):
        """Initialize the bounding box for the triangle."""
        xs = [v.x for v in self.vertices]
        ys = [v.y for v in self.vertices]
        zs = [v.z for v in self.vertices]
        self.bounding_box = BoundingBox(
            min=Vector3f(min(xs), min(ys), min(zs)),
            max=Vector3f(max(xs), max(ys), max(zs))
        )

        self.normal = self.calculate_normal()

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, Triangle):
            return False

        # Convert vertices to sets to ensure order-independence
        self_vertices = set(self.vertices)
        other_vertices = set(other.vertices)

        return self_vertices == other_vertices
    
    def calculate_normal(self) -> Vector3f:
        """Compute the normal vector of the triangle."""
        v0, v1, v2 = self.vertices
        normal = (v1 - v0).cross(v2 - v0).normalize()
        return normal
    
    def get_edges(self) -> Tuple[LineSegment, LineSegment, LineSegment]:
        
        return [
            LineSegment(self.vertices[0], self.vertices[1]), 
            LineSegment(self.vertices[1], self.vertices[2]), 
            LineSegment(self.vertices[2], self.vertices[0])
        ]
    
    def triangle_center(self) -> Vector3f:
        v1, v2, v3 = self.vertices
        center_x = (v1.x + v2.x + v3.x) / 3
        center_y = (v1.y + v2.y + v3.y) / 3
        center_z = (v1.z + v2.z + v3.z) / 3
        return Vector3f(center_x, center_y, center_z)
    
    def intersect_ray(self, ray_origin: Vector3f, ray_direction: Vector3f) -> Optional[float]:
        v0, v1, v2 = self.vertices
        edge1 = v1 - v0
        edge2 = v2 - v0
        h = ray_direction.cross(edge2)
        a = edge1.dot(h)

        if abs(a) < 1e-6:  # Ray is parallel to the triangle
            return None

        if a < 0:  # Backface hit
            return None

        f = 1.0 / a
        s = ray_origin - v0
        u = f * s.dot(h)
        if u < 0.0 or u > 1.0:
            return None

        q = s.cross(edge1)
        v = f * ray_direction.dot(q)
        if v < 0.0 or u + v > 1.0:
            return None

        t = f * edge2.dot(q)
        epsilon = 1e-6  # Threshold for valid intersections
        return t if t > epsilon else None
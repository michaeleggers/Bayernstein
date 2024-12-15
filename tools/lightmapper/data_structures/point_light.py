from data_structures.vector3f import Vector3f
from dataclasses import dataclass

@dataclass
class PointLight:
    origin: Vector3f
    intensity: float
    color: Vector3f
    range: float
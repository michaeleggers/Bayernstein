from data_structures.vector3f import Vector3f
from dataclasses import dataclass

@dataclass
class PointLight:
    origin: Vector3f    # "origin" (float float float) in trenchbroom 
    intensity: float    # "light" (float) in trenchbroom, typically between 50 and 500 for indoor lights (optional)
    color: Vector3f     # "color" (float float float) in tranchbroom, make sure to select range 0 to 1 (optional)
    range: float        # "range" (float) in trenchbroom, max range of light, use it for performance reasons (optional)
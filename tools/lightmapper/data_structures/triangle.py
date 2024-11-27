from dataclasses import dataclass
from typing import List, Optional, Tuple
from data_structures.vector3f import Vector3f

@dataclass
class Triangle:
    vertices: Tuple[Vector3f, Vector3f, Vector3f]
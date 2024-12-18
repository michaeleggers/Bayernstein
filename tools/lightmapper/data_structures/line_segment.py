from data_structures.vector3f import Vector3f
from typing import Tuple
from dataclasses import dataclass


@dataclass
class LineSegment:
    start: Vector3f
    end: Vector3f
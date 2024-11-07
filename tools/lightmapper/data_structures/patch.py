from data_structures.vector3f import Vector3f
import numpy as np
import json
from typing import Tuple


class Patch:
    def __init__(
        self, 
        x_tex_coord: int, 
        y_tex_coord: int,
        worldspace_coord: Vector3f,
        normal: Vector3f,
        uv: Tuple[float, float],
        uv_triangle: Tuple[Tuple[float, float], Tuple[float, float], Tuple[float, float]],
        is_emissive: bool
    ) -> None:
        self.x_tex_coord = x_tex_coord
        self.y_tex_coord = y_tex_coord
        self.worldspace_coord = worldspace_coord
        self.normal = normal
        self.uv = uv
        self.uv_triangle = uv_triangle
        self.is_emissive = is_emissive

    def serialize(self) -> dict:
        """
        Serialize the Patch object to a dictionary for JSON serialization.

        Returns:
            dict: A dictionary representation of the Patch object.
        """
        return {
            'x_tex_coord': self.x_tex_coord,
            'y_tex_coord': self.y_tex_coord,
            'worldspace_coord': self.worldspace_coord.to_tuple(),  # Assuming Vector3f has a tolist method
            'normal': self.normal.to_tuple(),  # Assuming Vector3f has a tolist method
            'uv': self.uv,
            'uv_triangle': self.uv_triangle,
            'is_emissive': self.is_emissive
        }

    def __repr__(self) -> str:
        """Return a string representation of the Patch object for debugging.

        Returns:
            str: A string representation of the Patch object.
        """
        return (f"Patch(x_tex_coord={self.x_tex_coord}, y_tex_coord={self.y_tex_coord}, "
                f"worldspace_coord={self.worldspace_coord}, normal={self.normal}, "
                f"uv={self.uv}, uv_triangle={self.uv_triangle}, is_emissive={self.is_emissive})")

    def __str__(self) -> str:
        """Return a user-friendly string representation of the Patch object.

        Returns:
            str: A user-friendly string representation of the Patch object.
        """
        return (f"Patch at texture coordinate ({self.x_tex_coord}, {self.y_tex_coord}): "
                f"Worldspace Coordinate: {self.worldspace_coord}, Normal: {self.normal}, "
                f"UV: {self.uv}, UV Triangle: {self.uv_triangle}, "
                f"Emissive: {'Yes' if self.is_emissive else 'No'}")
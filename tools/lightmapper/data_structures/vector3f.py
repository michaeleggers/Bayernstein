import numpy as np
from dataclasses import dataclass

@dataclass(frozen=True)
class Vector3f:
    """A class representing a 3D vector with float components.

    Attributes:
        x (float): The x-coordinate of the vector.
        y (float): The y-coordinate of the vector.
        z (float): The z-coordinate of the vector.
    """

    x: float
    y: float
    z: float

    def __sub__(self, other: 'Vector3f') -> 'Vector3f':
        return Vector3f(self.x - other.x, self.y - other.y, self.z - other.z)

    def __add__(self, other: 'Vector3f') -> 'Vector3f':
        return Vector3f(self.x + other.x, self.y + other.y, self.z + other.z)
    
    def __mul__(self, scalar: float) -> 'Vector3f':
        """Multiplies the vector by a scalar."""
        return Vector3f(self.x * scalar, self.y * scalar, self.z * scalar)

    def cross(self, other: 'Vector3f') -> 'Vector3f':
        return Vector3f(
            self.y * other.z - self.z * other.y,
            self.z * other.x - self.x * other.z,
            self.x * other.y - self.y * other.x
        )

    def dot(self, other: 'Vector3f') -> float:
        return self.x * other.x + self.y * other.y + self.z * other.z

    def normalize(self) -> 'Vector3f':
        length = np.sqrt(self.x**2 + self.y**2 + self.z**2)
        return Vector3f(self.x / length, self.y / length, self.z / length)
    
    def magnitude(self) -> float:
        return (self.x**2 + self.y**2 + self.z**2)**0.5

    def to_tuple(self) -> tuple[float, float, float]:
        """Convert the vector to a tuple.

        Returns:
            tuple: A tuple representation of the vector (x, y, z).
        """
        return (self.x, self.y, self.z)
    
    def to_array(self) -> np.ndarray:
        """Convert the vector to a NumPy array.

        Returns:
            np.ndarray: A NumPy array representation of the vector with dtype float32.
        """
        return np.array([self.x, self.y, self.z], dtype=np.float32)

    def __repr__(self) -> str:
        """Return a string representation of the vector for debugging.

        Returns:
            str: A string representation of the vector.
        """
        return f"Vector3f([{self.x}, {self.y}, {self.z}])"

    def __str__(self) -> str:
        """Return a user-friendly string representation of the vector.

        Returns:
            str: A user-friendly string representation of the vector.
        """
        return f"[{self.x}, {self.y}, {self.z}]"
    
    def __eq__(self, other: object) -> bool:
        if not isinstance(other, Vector3f):
            return False
        return (
            abs(self.x - other.x) < 1e-6 and
            abs(self.y - other.y) < 1e-6 and
            abs(self.z - other.z) < 1e-6
        )
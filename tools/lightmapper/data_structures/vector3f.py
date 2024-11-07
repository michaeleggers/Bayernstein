import numpy as np

class Vector3f:
    """A class representing a 3D vector with float components.

    Attributes:
        x (float): The x-coordinate of the vector.
        y (float): The y-coordinate of the vector.
        z (float): The z-coordinate of the vector.
    """

    def __init__(self, x: float, y: float, z: float) -> None:
        """Initialize a Vector3f instance.

        Args:
            x (float): The x-coordinate.
            y (float): The y-coordinate.
            z (float): The z-coordinate.
        """
        self.x = float(x)
        self.y = float(y)
        self.z = float(z)

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
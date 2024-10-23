import numpy as np
from pathlib import Path

class Mesh:
    """
    Represents a 3D mesh with vertices and indices.
    
    Attributes:
    -----------
    vertices : np.ndarray
        A numpy array of vertices where each vertex is a 3D coordinate (x, y, z).
    indices : np.ndarray
        A numpy array of face indices where each face is defined by 3 vertex indices.
    """

    def __init__(self) -> None:
        """
        Initializes the Mesh with empty vertices and indices arrays.
        """
        self.vertices: np.ndarray = np.array([], dtype=np.float32)
        self.indices: np.ndarray = np.array([], dtype=np.uint32)
    
    def from_object(self, mesh_path: Path) -> 'Mesh':
        """
        Load a mesh from a .obj file and store vertices and indices as numpy arrays.

        Parameters:
        -----------
        mesh_path : Path
            The file path to the .obj file.

        Returns:
        --------
        Mesh
            The mesh object with loaded vertices and indices.
        """
        vertices = []
        indices = []

        # Read the .obj file line by line
        with open(mesh_path, 'r') as file:
            for line in file:
                parts = line.strip().split()

                if not parts:
                    continue

                # Parse vertex data (lines starting with 'v')
                if parts[0] == 'v':
                    vertex = list(map(float, parts[1:4]))  # Convert to floats
                    vertices.append(vertex)

                # Parse face data (lines starting with 'f') and convert to 0-indexed
                elif parts[0] == 'f':
                    face_indices = [int(i.split('/')[0]) - 1 for i in parts[1:4]]
                    indices.append(face_indices)

        # Convert lists to numpy arrays
        self.vertices = np.array(vertices, dtype=np.float32)
        self.indices = np.array(indices, dtype=np.uint32)

        return self
    
    def from_cube(self) -> 'Mesh':
        """
        Define a cube mesh with predefined vertices and indices.

        Returns:
        --------
        Mesh
            The mesh object representing a cube.
        """
        self.vertices = np.array([
            # Front face
            [-0.5, -0.5,  0.5],  # 0
            [ 0.5, -0.5,  0.5],  # 1
            [ 0.5,  0.5,  0.5],  # 2
            [-0.5,  0.5,  0.5],  # 3

            # Back face
            [-0.5, -0.5, -0.5],  # 4
            [ 0.5, -0.5, -0.5],  # 5
            [ 0.5,  0.5, -0.5],  # 6
            [-0.5,  0.5, -0.5],  # 7
        ], dtype=np.float32)  

        self.indices = np.array([
            # Front face
            [0, 1, 2],  # Triangle 1
            [2, 3, 0],  # Triangle 2

            # Right face
            [1, 5, 6],  # Triangle 3
            [6, 2, 1],  # Triangle 4

            # Back face
            [5, 4, 7],  # Triangle 5
            [7, 6, 5],  # Triangle 6

            # Left face
            [4, 0, 3],  # Triangle 7
            [3, 7, 4],  # Triangle 8

            # Top face
            [3, 2, 6],  # Triangle 9
            [6, 7, 3],  # Triangle 10

            # Bottom face
            [4, 5, 1],  # Triangle 11
            [1, 0, 4],  # Triangle 12
        ], dtype=np.uint32)

        return self


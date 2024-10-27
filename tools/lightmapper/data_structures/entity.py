import numpy as np
import pyrr

from data_structures.mesh import Mesh
from data_structures.color import Color
from data_structures.vector3f import Vector3f

class Entity:
    """
    A basic entity in the world, with a position, rotation, and scale.
    
    Attributes:
        mesh (Mesh): The mesh associated with the entity.
        position (np.ndarray): 3D position of the entity in world space.
        eulers (np.ndarray): 3D rotation of the entity (Euler angles).
        scale (np.ndarray): Scale of the entity in 3D space.
        base_color (Color): Base color of the entity.
        emission (Color): Emission color of the entity.
        vertices (np.ndarray): Transformed vertices of the mesh.
        indices (np.ndarray): Indices of the mesh.
    """

    def __init__(
        self, 
        mesh: Mesh, 
        position: Vector3f, 
        eulers: Vector3f, 
        scale: Vector3f, 
        base_color: Color = Color(1, 1, 1), 
        emission: Color = Color(0, 0, 0)
    ):
        """
        Initialize the entity with position, rotation (Euler angles), scale, 
        base color, and emission color.

        Parameters:
            mesh (Mesh): The mesh associated with the entity.
            position (Vector3f): Position of the entity in world space.
            eulers (Vector3f): Rotation (Euler angles) of the entity.
            scale (Vector3f): Scale of the entity.
            base_color (Color, optional): The base color of the entity. Defaults to white.
            emission (Color, optional): Emission color for light emission. Defaults to black.
        """

        self.mesh = mesh
        self.position = position.to_array()
        self.eulers = eulers.to_array()
        self.scale = scale.to_array()
        self.base_color = base_color
        self.emission = emission

        # Precompute model transformation matrix
        model_transform = self.__get_model_transform()

        # Apply transformation to mesh vertices
        self.vertices = self.__apply_transform(np.array(mesh.vertices), model_transform)
        self.indices = mesh.indices

    def __get_model_transform(self) -> np.ndarray:
        """
        Returns the entity's model-to-world transformation matrix,
        including translation, rotation, and scaling.
        """
        # Start with the identity matrix
        model_transform = pyrr.matrix44.create_identity(dtype=np.float32)

        # Apply scaling first
        scale_matrix = pyrr.matrix44.create_from_scale(
            scale=np.array(self.scale), dtype=np.float32
        )

        # Apply rotation (around the Y-axis in this case)
        rotation_matrix = pyrr.matrix44.create_from_axis_rotation(
            axis=[0, 1, 0],
            theta=np.radians(self.eulers[1]), 
            dtype=np.float32
        )

        # Apply translation last
        translation_matrix = pyrr.matrix44.create_from_translation(
            vec=np.array(self.position), dtype=np.float32
        )

        # Combine matrices in the correct order: translation * rotation * scale
        model_transform = pyrr.matrix44.multiply(scale_matrix, rotation_matrix)
        model_transform = pyrr.matrix44.multiply(model_transform, translation_matrix)

        return model_transform.T

    def __apply_transform(self, vertices, transform):
        """
        Applies a transformation matrix to a list of vertices.

        Args:
        - vertices: List of 3D vertices (shape: Nx3).
        - transform: 4x4 transformation matrix.

        Returns:
        - transformed_vertices: List of transformed 3D vertices (shape: Nx3).
        """
        # Convert vertices to homogeneous coordinates by adding a 1 in the fourth dimension
        homogeneous_vertices = np.hstack([vertices, np.ones((vertices.shape[0], 1))])

        # Apply the transformation matrix (note the correct matrix multiplication order)
        transformed_vertices_homogeneous = np.dot(homogeneous_vertices, transform.T)

        # Convert back to 3D by dividing by the homogeneous coordinate (the fourth column)
        transformed_vertices = transformed_vertices_homogeneous[:, :3] / transformed_vertices_homogeneous[:, 3][:, np.newaxis]

        return transformed_vertices


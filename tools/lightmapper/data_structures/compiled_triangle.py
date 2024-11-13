import struct
from typing import List

from data_structures.compiled_vertex import CompiledVertex

class CompiledTriangle:

    def __init__(self, vertices: List[CompiledVertex], textureName, surfaceFlags, contentFlags):
        self.vertices = vertices  # List of 3 CompiledVertex objects
        self.textureName = textureName
        self.surfaceFlags = surfaceFlags
        self.contentFlags = contentFlags

    def to_binary(self):
        # Serialize the 3 vertices
        vertex_data = b''.join(vertex.to_binary() for vertex in self.vertices)
        
        # Encode the textureName as bytes with a null terminator
        texture_name_bytes = self.textureName.encode('utf-8') + b'\0'

        # Pack the triangle data: vertices, texture name (with EOS), surface and content flags
        return struct.pack(
            f'{len(vertex_data)}s {len(texture_name_bytes)}s 2Q',
            vertex_data,
            texture_name_bytes,
            self.surfaceFlags,
            self.contentFlags
        )
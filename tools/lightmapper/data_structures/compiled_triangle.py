import struct
from typing import List

from data_structures.compiled_vertex import CompiledVertex

class CompiledTriangle:

    def __init__(self, vertices: List[CompiledVertex], textureName, surfaceFlags, contentFlags):
        self.vertices = vertices  # List of 3 CompiledVertex objects
        self.textureName = textureName
        self.surfaceFlags = surfaceFlags
        self.contentFlags = contentFlags

    def to_binary(self) -> bytes:
        # Ensure texture name is exactly 256 bytes, padded with null bytes if necessary
        texture_name_bytes = self.textureName.encode('utf-8')
        texture_name_bytes = texture_name_bytes[:255]  # Trim if too long
        texture_name_bytes = texture_name_bytes.ljust(256, b'\0')  # Pad with null bytes

        # Serialize all fields
        vertex_data = b''.join(vertex.to_binary() for vertex in self.vertices)
        return struct.pack(
            f'{len(vertex_data)}s 256s 2Q',
            vertex_data,
            texture_name_bytes,
            self.surfaceFlags,
            self.contentFlags
        )
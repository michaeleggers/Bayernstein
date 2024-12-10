import struct

class CompiledVertex:
    def __init__(self, pos, normal, uv_texture, uv_lightmap, bc, color):
        self.pos = pos
        self.uv_texture = uv_texture
        self.uv_lightmap = uv_lightmap
        self.bc = bc
        self.normal = normal
        self.color = color 


    def to_binary(self):
        # Pack the vertex data (3 floats for pos, 3 for normal, 2 for each uv)
        return struct.pack(
            '3f 2f 2f 3f 3f 3f', 
            *self.pos, 
            *self.uv_texture, 
            *self.uv_lightmap,
            *self.bc,
            *self.normal,
            *self.color 
        )

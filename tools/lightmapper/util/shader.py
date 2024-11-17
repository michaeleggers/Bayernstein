from OpenGL.GL import *
from OpenGL.GL.shaders import compileProgram,compileShader

def create_shader(vertex_filepath: str, fragment_filepath: str) -> int:
    """
        Compile and link shader modules to make a shader program.

        Parameters:

            vertex_filepath: path to the text file storing the vertex
                            source code
            
            fragment_filepath: path to the text file storing the
                                fragment source code
        
        Returns:

            A handle to the created shader program
    """

    with open(vertex_filepath,'r') as f:
        vertex_src = f.readlines()

    with open(fragment_filepath,'r') as f:
        fragment_src = f.readlines()

    vertex_shader = compileShader(vertex_src, GL_VERTEX_SHADER)
    fragment_shader = compileShader(fragment_src, GL_FRAGMENT_SHADER)

    # Check for errors in the vertex shader compilation
    if not vertex_shader:
        print(glGetShaderInfoLog(vertex_shader))

    # Check for errors in the fragment shader compilation
    if not fragment_shader:
        print(glGetShaderInfoLog(fragment_shader))

    # Check for errors in program linking
    shader_program = compileProgram(vertex_shader, fragment_shader)
    if not shader_program:
        print(glGetProgramInfoLog(shader_program))
    
    return shader_program
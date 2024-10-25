import glfw
from OpenGL.GL import *

# Initialize GLFW
if not glfw.init():
    raise Exception("GLFW initialization failed")

# Request OpenGL version 3.3 and core profile
glfw.window_hint(glfw.CONTEXT_VERSION_MAJOR, 3)
glfw.window_hint(glfw.CONTEXT_VERSION_MINOR, 3)
glfw.window_hint(glfw.OPENGL_PROFILE, glfw.OPENGL_CORE_PROFILE)

# Optionally, disable forward compatibility for Mac
glfw.window_hint(glfw.OPENGL_FORWARD_COMPAT, GL_TRUE)

# Create a windowed mode window and its OpenGL context
window = glfw.create_window(640, 480, "OpenGL 3.3 Core Profile", None, None)
if not window:
    glfw.terminate()
    raise Exception("Window creation failed")

# Make the window's context current
glfw.make_context_current(window)

# Now OpenGL functions can be safely called
version = glGetString(GL_VERSION)
print(f"OpenGL version: {version.decode()}")

glfw.terminate()

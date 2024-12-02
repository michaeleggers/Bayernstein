import sys
from OpenGL.GL import *
from OpenGL.GL.shaders import compileProgram, compileShader
from pathlib import Path
import pyrr
import numpy as np
import glfw

from data_structures.scene import Scene
from data_structures.color import Color
from data_structures.vector3f import Vector3f

import util.shader as shader
import util.geometry as geometry

from PIL import Image


class Renderer:

    def __init__(self, width: int, height: int, scene: Scene, atmosphere_color: Color = Color(0.0, 0.0, 0.0), light_map_path: Path = None, lightmap_mode=False) -> None:
        

        # the parent folder of self
        # Check if the script is running as a bundled executable
        if getattr(sys, 'frozen', False):
            # If running from the bundled executable, use the extracted folder
            self.base_path = Path(sys._MEIPASS)
        else:
            # If running from the source code, use the regular script path
            self.base_path = Path(__file__).resolve().parent
        self.viewport_size = width
        self.scene = scene

        fov = 90 # FOV must be 90 for the lightmap generation in order for the hemisphere to be projected correctly onto the hemicubes

        # This will be the factor by which the lightmaps energy will be multiplied to achieve the final emissive color
        self.emission_strength = 0 if lightmap_mode else 1000

        # Initialize GLFW, OpenGL, and assets
        self._initialize_glfw(width=width, height=height, lightmap_mode=lightmap_mode)
        self._initialize_opengl(clear_color=atmosphere_color)
        self._initialize_assets(scene=scene, light_map_path=light_map_path)
        self._initialize_fbo(width=width, height=height)
        self._initialize_uniforms(fov)
        self._initialize_camera()

    def _initialize_glfw(self, width: int, height: int, lightmap_mode: bool) -> None:
        
        # Initialize GLFW
        if not glfw.init():
            raise Exception("GLFW could not be initialized")
        
        # Set window hints for OpenGL version and core profile
        glfw.window_hint(glfw.CONTEXT_VERSION_MAJOR, 3)
        glfw.window_hint(glfw.CONTEXT_VERSION_MINOR, 3)
        glfw.window_hint(glfw.OPENGL_PROFILE, glfw.OPENGL_CORE_PROFILE)
        glfw.window_hint(glfw.OPENGL_FORWARD_COMPAT, GL_TRUE)  # For Mac compatibility

        if lightmap_mode:
            glfw.window_hint(glfw.RESIZABLE, glfw.FALSE)
            glfw.window_hint(glfw.SCALE_TO_MONITOR, glfw.FALSE)
            # very important as otherwise windows changes the framebuffer size
            glfw.window_hint(glfw.DECORATED, glfw.FALSE)    

        
        # Set the window hints to increase color precision
        glfw.window_hint(glfw.RED_BITS, 16)
        glfw.window_hint(glfw.GREEN_BITS, 16)
        glfw.window_hint(glfw.BLUE_BITS, 16)
        glfw.window_hint(glfw.ALPHA_BITS, 16)
        glfw.window_hint(glfw.DEPTH_BITS, 24)
        glfw.window_hint(glfw.DOUBLEBUFFER, glfw.TRUE)

        # Create the GLFW window with an OpenGL context
        self.window = glfw.create_window(width, height, "OpenGL Renderer", None, None)
        if not self.window:
            glfw.terminate()
            raise Exception("Failed to create GLFW window")

        # Set the current context to this window
        glfw.make_context_current(self.window)

        # Get and print the framebuffer size
        window_size = glfw.get_window_size(self.window)

        # Set the viewport to the framebuffer size
        glViewport(0, 0, window_size[0], window_size[1])

        
        if lightmap_mode == False:
            # Set a callback for resizing the window
            glfw.set_framebuffer_size_callback(self.window, self._framebuffer_size_callback)
            # Enable vertical sync (if desired)
            glfw.swap_interval(1)
        
        # Create a clock similar to pygame's
        self.clock = glfw.get_time

    def _framebuffer_size_callback(self, window, width, height):
        # Adjust the viewport when the window size changes
        glViewport(0, 0, width, height)

    def _initialize_opengl(self, clear_color) -> None:
        glClearColor(clear_color.r, clear_color.g, clear_color.b, 1)
        glEnable(GL_DEPTH_TEST)

    def _initialize_assets(self, scene: Scene, light_map_path: Path) -> None:
        """Initialize OpenGL buffers, textures, and shaders."""
        
        # Vertex Buffer Object (VBO) and Vertex Array Object (VAO)
        self.vao = glGenVertexArrays(1)
        glBindVertexArray(self.vao)

        self.vbo = glGenBuffers(1)
        glBindBuffer(GL_ARRAY_BUFFER, self.vbo)
        glBufferData(GL_ARRAY_BUFFER, scene.vertex_array.nbytes, scene.vertex_array, GL_STATIC_DRAW)

        # Texture Array Setup
        texture_array = self.scene.texture_array
        num_layers, max_height, max_width, _ = texture_array.shape

        # Create and bind the texture array
        self.texture_array_id = glGenTextures(1)
        glActiveTexture(GL_TEXTURE0)  # Ensure GL_TEXTURE0 is active for the texture array
        glBindTexture(GL_TEXTURE_2D_ARRAY, self.texture_array_id)
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, max_width, max_height, num_layers, 0, GL_RGBA, GL_FLOAT, texture_array)

        # Set texture parameters for the texture array
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT)
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT)
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR)
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
        glGenerateMipmap(GL_TEXTURE_2D_ARRAY)

        # Lightmap Setup
        lightmap = self.scene.light_map
        lightmap_width, lightmap_height = lightmap.shape[:2]
        self.lightmap_texture_id = glGenTextures(1)

        # Create and bind the lightmap texture
        glActiveTexture(GL_TEXTURE1)  # Use GL_TEXTURE1 for the lightmap
        glBindTexture(GL_TEXTURE_2D, self.lightmap_texture_id)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, lightmap_width, lightmap_height, 0, GL_RGB, GL_FLOAT, lightmap)

        # Texture parameters for lightmap
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)

        # Compile and link shaders
        self.shader = shader.create_shader(
            vertex_filepath=self.base_path / 'shaders/vertex.txt',
            fragment_filepath=self.base_path / 'shaders/fragment.txt'
        )

        # Now the shader is available, we can set the uniforms
        glUseProgram(self.shader)

        # Set uniform for the diffuse texture array
        glUniform1i(glGetUniformLocation(self.shader, "diffuseTextureArray"), 0)

        # Set uniform for the lightmap texture
        glUniform1i(glGetUniformLocation(self.shader, "lightmapTexture"), 1)

        # Set vertex attributes (Position, UVs, Texture index)
        glEnableVertexAttribArray(0)  # Position (x, y, z)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, ctypes.c_void_p(0))

        glEnableVertexAttribArray(1)  # Diffuse texture UV (u_t, v_t)
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 32, ctypes.c_void_p(12))

        glEnableVertexAttribArray(2)  # Lightmap texture UV (u_l, v_l)
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 32, ctypes.c_void_p(20))

        glEnableVertexAttribArray(3)  # Texture index
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 32, ctypes.c_void_p(28))

    def _initialize_fbo(self, width: int, height: int) -> None:
        """Initialize FBO and texture for rendering."""
        # Create high-precision texture for FBO
        self.fbo_texture = glGenTextures(1)
        glBindTexture(GL_TEXTURE_2D, self.fbo_texture)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, None)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)

        # Create and bind the framebuffer
        self.fbo = glGenFramebuffers(1)
        glBindFramebuffer(GL_FRAMEBUFFER, self.fbo)
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, self.fbo_texture, 0)

        # Ensure the framebuffer is complete
        if glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE:
            raise Exception("Framebuffer not complete")

        # Unbind the framebuffer to avoid interference
        glBindFramebuffer(GL_FRAMEBUFFER, 0)

    def _initialize_uniforms(self, fov=90) -> None:
        """Initialize uniforms for the shader program."""
        
        # Use the shader program
        glUseProgram(self.shader)

        # Set the exposure uniform
        glUniform1f(glGetUniformLocation(self.shader, "exposure"), self.emission_strength)

        # Set up the projection matrix
        width, height = glfw.get_window_size(self.window)
        projection = pyrr.matrix44.create_perspective_projection(
            fovy=fov, aspect=width / height, near=0.1, far=4000.0, dtype=np.float32
        )
        glUniformMatrix4fv(glGetUniformLocation(self.shader, "projection"), 1, GL_FALSE, projection)

        # Store uniform locations for model, view, and exposure
        self.modelMatrixLocation = glGetUniformLocation(self.shader, "model")
        self.viewMatrixLocation = glGetUniformLocation(self.shader, "view")
        self.exposureLocation = glGetUniformLocation(self.shader, "exposure")

        # Bind texture samplers to their respective texture units
        glUniform1i(glGetUniformLocation(self.shader, "diffuseTextureArray"), 0)  # GL_TEXTURE0
        glUniform1i(glGetUniformLocation(self.shader, "lightmapTexture"), 1)      # GL_TEXTURE1


    def _initialize_camera(self) -> None:

        camera_position = np.array([0 ,60 ,0], dtype=np.float32)
        camera_direction =  np.array([1, 0, 0], dtype=np.float32)

        camera_direction = np.array([1, 0, 0])
        x, y, z = camera_position
        self.camera_start_pos = [x, y, z]
        self.camera_pos = camera_position
        self.camera_front = camera_direction / np.linalg.norm(camera_direction)
        self.camera_up = geometry.calculate_camera_up(camera_direction) #np.array([0.0, 1.0, 0.0], dtype=np.float32)
        self.camera_speed = 5.0
        self.yaw = np.degrees(np.arctan2(self.camera_front[2], self.camera_front[0]))
        self.pitch = np.degrees(np.arcsin(self.camera_front[1]))
        self.sensitivity = 1.0

    def update_light_map(self):
        
        #glDeleteTextures(1, (self.lightmap_texture_id))

        lightmap = self.scene.light_map
        lightmap_width, lightmap_height = lightmap.shape[:2]
        self.lightmap_texture_id = glGenTextures(1)
        glBindTexture(GL_TEXTURE_2D, self.lightmap_texture_id)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, lightmap_width, lightmap_height, 0, GL_RGB, GL_FLOAT, lightmap)

        # Initialize the texture
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)

    def render_single_image(self, position: Vector3f, direction: np.ndarray, camera_up: np.ndarray) -> np.ndarray:
        """Render a single image using the pre-initialized FBO."""
        # Update the view matrix
        self.update_view_matrix(position.to_array(), direction, camera_up)

        # Bind the FBO and render the scene
        glBindFramebuffer(GL_FRAMEBUFFER, self.fbo)
        self.render_scene()

        glFlush()
        glFinish()

        # Dimensions of the framebuffer
        width, height = glfw.get_window_size(self.window)

        # Read pixels from the FBO
        glPixelStorei(GL_PACK_ALIGNMENT, 1)
        image_data = glReadPixels(0, 0, width, height, GL_RGB, GL_FLOAT)
        image_array = np.frombuffer(image_data, dtype=np.float32).reshape(height, width, 3)
        image_array = np.flipud(image_array)

        # Unbind the framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0)

        return image_array

    
    def update_view_matrix(self, camera_pos, camera_direction, camera_up) -> None:
        view_matrix = self._get_view_matrix(camera_pos, camera_direction, camera_up)
        glUniformMatrix4fv(self.viewMatrixLocation, 1, GL_FALSE, view_matrix)

    def _get_view_matrix(self, camera_pos, camera_direction, camera_up) -> np.ndarray:
        camera_target = camera_pos + camera_direction
        return pyrr.matrix44.create_look_at(
            eye=camera_pos, 
            target=camera_target, 
            up=camera_up, 
            dtype=np.float32
        )
    
    def render_scene(self) -> None:
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        glUseProgram(self.shader)
        glEnable(GL_CULL_FACE)
        glCullFace(GL_BACK)
        glFrontFace(GL_CCW)

        # Set shader uniforms
        glUniform1f(self.exposureLocation, self.emission_strength)
        glUniformMatrix4fv(self.modelMatrixLocation, 1, GL_FALSE, self.scene.get_model_transform())

        # Bind textures once for the atlas and lightmap
        glActiveTexture(GL_TEXTURE0)
        glBindTexture(GL_TEXTURE_2D_ARRAY, self.texture_array_id)
        glUniform1i(glGetUniformLocation(self.shader, "diffuseTexture"), 0)  # Texture atlas bound to unit 0

        glActiveTexture(GL_TEXTURE1)
        glBindTexture(GL_TEXTURE_2D, self.lightmap_texture_id)
        glUniform1i(glGetUniformLocation(self.shader, "lightmapTexture"), 1)  # Lightmap bound to unit 1

        # Prepare to draw
        self.arm_for_drawing()
        self.draw()

    def arm_for_drawing(self):
        glBindVertexArray(self.vao)

    def draw(self):
        glDrawArrays(GL_TRIANGLES, 0, len(self.scene.vertex_array) // 3)

    def run(self) -> None:
        while not glfw.window_should_close(self.window):
            self._process_input()
            self._update_camera_vectors()
            self.update_view_matrix(self.camera_pos, self.camera_front, self.camera_up)
            self.render_scene()

            glfw.swap_buffers(self.window)
            glfw.poll_events()

    def _process_input(self) -> None:
        if glfw.get_key(self.window, glfw.KEY_W) == glfw.PRESS:
            self.camera_pos += self.camera_speed * self.camera_front
        if glfw.get_key(self.window, glfw.KEY_S) == glfw.PRESS:
            self.camera_pos -= self.camera_speed * self.camera_front
        if glfw.get_key(self.window, glfw.KEY_A) == glfw.PRESS:
            self.camera_pos -= np.cross(self.camera_front, self.camera_up) * self.camera_speed
        if glfw.get_key(self.window, glfw.KEY_D) == glfw.PRESS:
            self.camera_pos += np.cross(self.camera_front, self.camera_up) * self.camera_speed
        
        # Handle rotation (Arrow keys)
        if glfw.get_key(self.window, glfw.KEY_LEFT) == glfw.PRESS:
            self.yaw += self.sensitivity
        if glfw.get_key(self.window, glfw.KEY_RIGHT) == glfw.PRESS:
            self.yaw -= self.sensitivity
        if glfw.get_key(self.window, glfw.KEY_UP) == glfw.PRESS:
            self.pitch += self.sensitivity
            self.pitch = min(self.pitch, 89.0)  # Limit pitch to avoid gimbal lock
        if glfw.get_key(self.window, glfw.KEY_DOWN) == glfw.PRESS:
            self.pitch -= self.sensitivity
            self.pitch = max(self.pitch, -89.0)

        if glfw.get_key(self.window, glfw.KEY_J) == glfw.PRESS:
            self.emission_strength -= 1
        if glfw.get_key(self.window, glfw.KEY_K) == glfw.PRESS:
            self.emission_strength += 1
        self.emission_strength = max(1, min(self.emission_strength, 1000))

    def _update_camera_vectors(self) -> None:
        # Calculate new front vector based on yaw and pitch
        front = np.array([
            np.cos(np.radians(self.yaw)) * np.cos(np.radians(self.pitch)),
            np.sin(np.radians(self.yaw)) * np.cos(np.radians(self.pitch)),
            np.sin(np.radians(self.pitch))
        ], dtype=np.float32)
        self.camera_front = front / np.linalg.norm(front)

    def destroy(self) -> None:
        # Delete Vertex Array Object and Vertex Buffer Object
        glDeleteVertexArrays(1, (self.vao,))
        glDeleteBuffers(1, (self.vbo,))
        glDeleteFramebuffers(1, [self.fbo])

        # Delete textures for the texture atlas and lightmap
        glDeleteTextures(1, (self.texture_array_id,))
        glDeleteTextures(1, (self.lightmap_texture_id,))
        glDeleteTextures(1, (self.fbo_texture))

        # Delete shader program
        glDeleteProgram(self.shader)

        # Destroy GLFW window and terminate GLFW
        glfw.destroy_window(self.window)
        glfw.terminate()

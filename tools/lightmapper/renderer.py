import sys
from OpenGL.GL import *
from OpenGL.GL.shaders import compileProgram, compileShader
from pathlib import Path
from typing import List
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
        self.lightmap_mode = lightmap_mode

        fov = 90 # FOV must be 90 for the lightmap generation in order for the hemisphere to be projected correctly onto the hemicubes

        # This will be the factor by which the lightmaps energy will be multiplied to achieve the final emissive color
        self.emission_strength = 0 if self.lightmap_mode else 1000

        # Initialize GLFW, OpenGL, and assets
        self._initialize_glfw(width=width, height=height, lightmap_mode=lightmap_mode)
        self._initialize_opengl(clear_color=atmosphere_color)
        self._initialize_assets(scene=scene)
        if self.lightmap_mode:
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


    # Initialize assets

    def _initialize_assets(self, scene: Scene) -> None:
        """Initialize OpenGL buffers, textures, and shaders."""
        self._initialize_geometry_buffers(scene)
        #self._initialize_line_buffers(scene)
        self._initialize_texture_array(scene)
        self._initialize_lightmap(scene)
        self._initialize_shaders()

    def _initialize_geometry_buffers(self, scene: Scene) -> None:
        """Initialize buffers for the scene geometry (triangles)."""
        self.vao = glGenVertexArrays(1)
        glBindVertexArray(self.vao)

        self.vbo = glGenBuffers(1)
        glBindBuffer(GL_ARRAY_BUFFER, self.vbo)
        glBufferData(GL_ARRAY_BUFFER, scene.vertex_array.nbytes, scene.vertex_array, GL_STATIC_DRAW)

        """Set vertex attributes for the geometry buffers."""
        glEnableVertexAttribArray(0)  # Position (x, y, z)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, ctypes.c_void_p(0))

        glEnableVertexAttribArray(1)  # Diffuse texture UV (u_t, v_t)
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 32, ctypes.c_void_p(12))

        glEnableVertexAttribArray(2)  # Lightmap texture UV (u_l, v_l)
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 32, ctypes.c_void_p(20))

        glEnableVertexAttribArray(3)  # Texture index
        glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 32, ctypes.c_void_p(28))

    def _initialize_line_buffers(self, scene: Scene) -> None:
        """Initialize buffers for the lines in the scene."""
        # Generate and bind VAO for lines
        self.line_vao = glGenVertexArrays(1)
        glBindVertexArray(self.line_vao)

        # Generate and bind VBO for line data (positions and colors)
        self.line_vbo = glGenBuffers(1)
        glBindBuffer(GL_ARRAY_BUFFER, self.line_vbo)
        glBufferData(GL_ARRAY_BUFFER, scene.line_array.nbytes, scene.line_array, GL_STATIC_DRAW)

        # Set vertex attributes for the line buffers
        # 0: Position (x, y, z) - each line has 2 points
        glEnableVertexAttribArray(0)  # Position
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 24, ctypes.c_void_p(0))  # 3 floats for position

        # 1: Color (r, g, b) - each point has a color
        glEnableVertexAttribArray(1)  # Color
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 24, ctypes.c_void_p(12))  # 3 floats for color (offset by 12 bytes)

        glBindVertexArray(0)  # Unbind VAO

    def _initialize_texture_array(self, scene: Scene) -> None:
        """Initialize the texture array for the scene."""
        texture_array = self.scene.texture_array
        num_layers, max_height, max_width, _ = texture_array.shape

        self.texture_array_id = glGenTextures(1)
        glActiveTexture(GL_TEXTURE0)
        glBindTexture(GL_TEXTURE_2D_ARRAY, self.texture_array_id)
        glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, max_width, max_height, num_layers, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_array)

        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT)
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT)
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR)
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR)
        glGenerateMipmap(GL_TEXTURE_2D_ARRAY)

    def _initialize_lightmap(self, scene: Scene) -> None:
        """Initialize the lightmap texture for the scene."""
        lightmap = self.scene.light_map
        lightmap_width, lightmap_height = lightmap.shape[:2]
        
        self.lightmap_texture_id = glGenTextures(1)

        glActiveTexture(GL_TEXTURE1)
        glBindTexture(GL_TEXTURE_2D, self.lightmap_texture_id)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, lightmap_width, lightmap_height, 0, GL_RGB, GL_FLOAT, lightmap)

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)

    def _initialize_shaders(self) -> None:
        """Compile and link shaders for the scene."""
        # Compile and link the main shader
        self.shader = shader.create_shader(
            vertex_filepath=self.base_path / 'shaders/vertex.txt',
            fragment_filepath=self.base_path / 'shaders/fragment.txt',
        )

        # Compile and link the line shader
        #self.shader_line = shader.create_shader(
        #    vertex_filepath=self.base_path / 'shaders/lineVertex.txt',
        #    fragment_filepath=self.base_path / 'shaders/lineFragment.txt'
        #)

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
        
        # Set up the projection matrix
        width, height = glfw.get_window_size(self.window)
        projection = pyrr.matrix44.create_perspective_projection(
            fovy=fov, aspect=width / height, near=0.1, far=4000.0, dtype=np.float32
        )

        # Use the shader program
        glUseProgram(self.shader)

        # Store uniform locations for model, view, and exposure
        self.modelMatrixLocation = glGetUniformLocation(self.shader, "model")
        self.viewMatrixLocation = glGetUniformLocation(self.shader, "view")
        self.projectionMatrixLocation = glGetUniformLocation(self.shader, "projection")
        self.exposureLocation = glGetUniformLocation(self.shader, "exposure")

        glUniformMatrix4fv(self.projectionMatrixLocation, 1, GL_FALSE, projection)
        glUniform1f(self.exposureLocation, self.emission_strength)
        # Bind texture samplers to their respective texture units
        glUniform1i(glGetUniformLocation(self.shader, "diffuseTextureArray"), 0)  # GL_TEXTURE0
        glUniform1i(glGetUniformLocation(self.shader, "lightmapTexture"), 1)      # GL_TEXTURE1



        #glUseProgram(self.shader_line)
        #self.viewMatrixLocation_line = glGetUniformLocation(self.shader_line, "view")
        #self.projectionMatrixLocation_line = glGetUniformLocation(self.shader_line, "projection")

        #glUniformMatrix4fv(self.projectionMatrixLocation_line, 1, GL_FALSE, projection)


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
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)

    def render_batch_images(self, positions: List[Vector3f], directions: List[np.ndarray], camera_ups: List[np.ndarray]) -> List[np.ndarray]:
        """Render multiple images in one pass if possible."""
        images = []
        glBindFramebuffer(GL_FRAMEBUFFER, self.fbo)
        
        for position, direction, camera_up in zip(positions, directions, camera_ups):
            # Update only the view matrix for each frame
            self.update_view_matrix(position.to_array(), direction, camera_up)
            self.render_scene()

            # Read pixels
            width, height = glfw.get_window_size(self.window)
            image_array = np.empty((height, width, 3), dtype=np.float32)
            glReadPixels(0, 0, width, height, GL_RGB, GL_FLOAT, image_array)
            images.append(np.flipud(image_array))  # Flip once per frame
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0)
        return images

    
    def update_view_matrix(self, camera_pos, camera_direction, camera_up) -> None:
        
        view_matrix = self._get_view_matrix(camera_pos, camera_direction, camera_up)

        glUseProgram(self.shader)
        glUniformMatrix4fv(self.viewMatrixLocation, 1, GL_FALSE, view_matrix)

        #glUseProgram(self.shader_line)
        #glUniformMatrix4fv(self.viewMatrixLocation_line, 1, GL_FALSE, view_matrix)

        #glUseProgram(self.shader)

    def _get_view_matrix(self, camera_pos, camera_direction, camera_up) -> np.ndarray:
        camera_target = camera_pos + camera_direction
        return pyrr.matrix44.create_look_at(
            eye=camera_pos, 
            target=camera_target, 
            up=camera_up, 
            dtype=np.float32
        )
    
    def render_scene(self) -> None:
        """Render the entire scene, including geometry and lines."""
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        
        # Render geometry (triangles)
        self._render_geometry()

        # Render lines
        #self._render_lines()

        # Reset shader
        #glUseProgram(self.shader)

    def _render_geometry(self) -> None:
        """Render the scene geometry using the main shader."""
        glUseProgram(self.shader)
        glEnable(GL_CULL_FACE)
        glCullFace(GL_BACK)
        glFrontFace(GL_CCW)

        # Set shader uniforms for geometry
        glUniform1f(self.exposureLocation, self.emission_strength)
        glUniformMatrix4fv(self.modelMatrixLocation, 1, GL_FALSE, self.scene.get_model_transform())

        # Bind the texture array and lightmap texture for geometry
        glActiveTexture(GL_TEXTURE0)
        glBindTexture(GL_TEXTURE_2D_ARRAY, self.texture_array_id)
        glUniform1i(glGetUniformLocation(self.shader, "diffuseTexture"), 0)  # Texture atlas bound to unit 0

        glActiveTexture(GL_TEXTURE1)
        glBindTexture(GL_TEXTURE_2D, self.lightmap_texture_id)
        glUniform1i(glGetUniformLocation(self.shader, "lightmapTexture"), 1)  # Lightmap bound to unit 1

        # Prepare to draw the geometry
        glBindVertexArray(self.vao)
        glDrawArrays(GL_TRIANGLES, 0, len(self.scene.vertex_array) // 3)

    def _render_lines(self) -> None:
        """Render the scene lines using the line shader."""
        glUseProgram(self.shader_line)  # Use the line shader

        # Set any line-specific uniforms here (e.g., line color)
        glUniform3f(glGetUniformLocation(self.shader_line, "lineColor"), 1.0, 0.0, 0.0)  # Red 

        # Bind the line VAO and prepare for line rendering
        glBindVertexArray(self.line_vao)
        
        # Draw the lines with GL_LINES
        glDrawArrays(GL_LINES, 0, self.scene.line_count * 2)

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
        if self.lightmap_mode:
            glDeleteFramebuffers(1, [self.fbo])

        # Delete textures for the texture atlas and lightmap
        glDeleteTextures(1, (self.texture_array_id,))
        glDeleteTextures(1, (self.lightmap_texture_id,))
        if self.lightmap_mode:
            glDeleteTextures(1, (self.fbo_texture,))

        # Delete shader program
        glDeleteProgram(self.shader)

        # Destroy GLFW window and terminate GLFW
        glfw.destroy_window(self.window)
        glfw.terminate()

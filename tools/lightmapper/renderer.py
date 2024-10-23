import pygame as pg
from OpenGL.GL import *
import random
from OpenGL.GL.shaders import compileProgram, compileShader
from pathlib import Path
import pyrr
import numpy as np
from pygame.locals import K_w, K_a, K_s, K_d, K_k, K_j, K_LEFT, K_RIGHT, K_UP, K_DOWN
import cv2


from data_structures.scene import Scene
from data_structures.color import Color
from data_structures.material import Material
import util.shader as shader
import util.geometry as geometry

from data_structures.vector3f import Vector3f

class Renderer:

    def __init__(self, width: int, height: int, fov: int, scene: Scene, atmosphere_color: Color = Color(0.0, 0.0, 0.0), light_map_path: Path = None, patch = None, lightmap_mode=False) -> None:
        
        # just for debugging
        self.random_patch = patch
        self.viewport_size = width

        self.emission_strength = 1 if lightmap_mode else 500

        self.scene = scene
        self._initialize_pygame(width=width, height=height, lightmap_mode=lightmap_mode)
        self._initialize_opengl(clear_color=atmosphere_color)
        self._initialize_assets(scene=scene, lightmap_mode=lightmap_mode, light_map_path=light_map_path)
        self._initialize_uniforms(fov)
        self._initialize_camera()

    def _initialize_pygame(self, width: int, height: int, lightmap_mode: bool) -> None:
        pg.init()
        pg.display.gl_set_attribute(pg.GL_RED_SIZE, 16)
        pg.display.gl_set_attribute(pg.GL_GREEN_SIZE, 16)
        pg.display.gl_set_attribute(pg.GL_BLUE_SIZE, 16)
        pg.display.gl_set_attribute(pg.GL_ALPHA_SIZE, 16)
        pg.display.gl_set_attribute(pg.GL_DEPTH_SIZE, 24)
        pg.display.gl_set_attribute(pg.GL_DOUBLEBUFFER, 1)
        pg.display.set_mode((width, height), pg.OPENGL | pg.DOUBLEBUF)

        self.clock = pg.time.Clock()

    def _initialize_opengl(self, clear_color: Color) -> None:
        glClearColor(clear_color.r, clear_color.g, clear_color.b, 1)
        glEnable(GL_DEPTH_TEST)

    def _initialize_assets(self, scene: Scene, lightmap_mode: bool, light_map_path: Path) -> None:

        self.vao = glGenVertexArrays(1)
        glBindVertexArray(self.vao)
        self.vbo = glGenBuffers(1)
        glBindBuffer(GL_ARRAY_BUFFER, self.vbo)
        glBufferData(GL_ARRAY_BUFFER, scene.vertex_array.nbytes, scene.vertex_array, GL_STATIC_DRAW)

        # vertices
        glEnableVertexAttribArray(0)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, ctypes.c_void_p(0))
        # color
        glEnableVertexAttribArray(1)
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 32, ctypes.c_void_p(12))
        # uv
        glEnableVertexAttribArray(2)
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 32, ctypes.c_void_p(24))

        # DEBUG
        if self.random_patch:
            scene.light_map[self.random_patch[0], self.random_patch[1]] = [255, 255, 255]

        if light_map_path:
                # Load the HDR texture into a NumPy array with floating-point precision
                light_map = cv2.imread(str(light_map_path), cv2.IMREAD_UNCHANGED)

                # Convert from BGR to RGB if needed
                light_map = cv2.cvtColor(light_map, cv2.COLOR_BGR2RGB)

                # Assign the light map to the scene
                self.scene.light_map = light_map

                # Assign the material
                self.light_map_material = Material(filepath=light_map_path)

        else:
            light_map_path = Path('temp/lightmap.hdr')
            scene.generate_light_map(light_map_path)
            self.light_map_material = Material(filepath=light_map_path)

        self.shader = shader.create_shader(
            vertex_filepath="shaders/vertex.txt", 
            fragment_filepath="shaders/fragment.txt"
        )

    def _initialize_uniforms(self, fov=90) -> None:
        glUseProgram(self.shader)
        glUniform1i(glGetUniformLocation(self.shader, "imageTexture"), 0)
        width, height = pg.display.get_surface().get_size()
        projection = pyrr.matrix44.create_perspective_projection(
            fovy=fov, aspect=width/height, near=0.1, far=2000, dtype=np.float32
        )
        glUniformMatrix4fv(glGetUniformLocation(self.shader, "projection"), 1, GL_FALSE, projection)
        self.modelMatrixLocation = glGetUniformLocation(self.shader, "model")
        self.viewMatrixLocation = glGetUniformLocation(self.shader, "view")

        # Set the logStrength uniform
        self.logStrengthLocation = glGetUniformLocation(self.shader, "logStrength")
        glUniform1f(self.logStrengthLocation, self.emission_strength)

    def _initialize_camera(self) -> None:

        if self.random_patch:
            random_patch = self.random_patch
            print('random_patch:', random_patch)

            camera_position = np.array([random_patch[2], random_patch[3], random_patch[4]], dtype=np.float32)
            camera_direction =  np.array([random_patch[5], random_patch[6], random_patch[7]], dtype=np.float32)
        else:
            camera_position = np.array([0 ,0 ,0], dtype=np.float32)
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

    def update_ligth_map(self, light_map_path: Path):
        #glDeleteTextures(1, (self.light_map_material.texture,))

        # Load the HDR texture into a NumPy array with floating-point precision
        light_map = cv2.imread(str(light_map_path), cv2.IMREAD_UNCHANGED)

        # Convert from BGR to RGB if needed
        light_map = cv2.cvtColor(light_map, cv2.COLOR_BGR2RGB)

        # Assign the light map to the scene
        self.scene.light_map = light_map

        # Assign the material
        self.light_map_material = Material(filepath=light_map_path)

    def render_single_image(self, position: Vector3f, direction: np.ndarray, camera_up: np.ndarray) -> None:

        # Set the camera position, direction, and up vector
        self.update_view_matrix(
            position.to_array(), 
            direction,
            camera_up
        )

        # Render the scene once
        self.render_scene()
        
        # Ensure all OpenGL commands are completed
        glFlush()
        glFinish()

        # Capture the rendered frame using OpenGL's glReadPixels
        width, height = pg.display.get_surface().get_size()
        glPixelStorei(GL_PACK_ALIGNMENT, 1)
        image_data = glReadPixels(0, 0, width, height, GL_RGB, GL_FLOAT)

        # Convert the image to a format suitable for saving
        image_array = np.frombuffer(image_data, dtype=np.float32).reshape(height, width, 3)

        # Since OpenGL coordinates are bottom-left, we flip the image vertically
        image_array = np.flipud(image_array)

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
        # Clear screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)
        glUseProgram(self.shader)

        # Enable face culling
        glEnable(GL_CULL_FACE)
        glCullFace(GL_BACK)  # Cull back faces, meaning only front faces are rendered
        glFrontFace(GL_CCW)  # Specify that front faces are counterclockwise (default)

        glUniform1f(self.logStrengthLocation, self.emission_strength)

        # Render geometry
        # TODO: handle multiple entities in scene
        glUniformMatrix4fv(self.modelMatrixLocation, 1, GL_FALSE, self.scene.get_model_transform())
        self.light_map_material.use()
        self.arm_for_drawing()
        self.draw()

    def arm_for_drawing(self):
        glBindVertexArray(self.vao)

    def draw(self):
        glDrawArrays(GL_TRIANGLES, 0, self.scene.vertex_count)

    def run(self) -> None:
    
        running = True
        while running:

            self._process_input()
            self._update_camera_vectors()
            self.update_view_matrix(self.camera_pos, self.camera_front, self.camera_up)
            self.render_scene()



            #position = np.array([0.0, 0.0, 100.0], dtype=np.float32)
            #direction = np.array([0.0, 0.0, -1.0], dtype=np.float32)
            #filepath = "rendered_image.png"  # Output image file path
            #my_app.render_single_image(position, direction, filepath)

            pg.display.flip()
            self.clock.tick(60)

    def _process_input(self) -> None:

        keys = pg.key.get_pressed()

        # Handle movement (WASD)
        if keys[K_w]:
            self.camera_pos += self.camera_speed * self.camera_front
        if keys[K_s]:
            self.camera_pos -= self.camera_speed * self.camera_front
        if keys[K_a]:
            self.camera_pos -= np.cross(self.camera_front, self.camera_up) * self.camera_speed
        if keys[K_d]:
            self.camera_pos += np.cross(self.camera_front, self.camera_up) * self.camera_speed

        # Handle rotation (Arrow keys)
        if keys[K_LEFT]:
            self.yaw -= self.sensitivity
        if keys[K_RIGHT]:
            self.yaw += self.sensitivity
        if keys[K_UP]:
            self.pitch += self.sensitivity
            self.pitch = min(self.pitch, 89.0)  # Limit pitch to avoid gimbal lock
        if keys[K_DOWN]:
            self.pitch -= self.sensitivity
            self.pitch = max(self.pitch, -89.0)

        # Handle log strength
        if keys[K_j]:
            self.emission_strength -= 10
        if keys[K_k]:
            self.emission_strength += 10

        # Clamp the value between 0 and 1
        self.emission_strength = max(1.0, min(self.emission_strength, 10000))

        for event in pg.event.get():
            if event.type == pg.QUIT:
                pg.quit()
                exit()

    def _update_camera_vectors(self) -> None:
        # Calculate new front vector based on yaw and pitch
        front = np.array([
            np.cos(np.radians(self.yaw)) * np.cos(np.radians(self.pitch)),
            np.sin(np.radians(self.pitch)),
            np.sin(np.radians(self.yaw)) * np.cos(np.radians(self.pitch))
        ], dtype=np.float32)
        self.camera_front = front / np.linalg.norm(front)

    def destroy(self) -> None:
        
        glDeleteVertexArrays(1,(self.vao,))
        glDeleteBuffers(1,(self.vbo,))
        glDeleteTextures(1, (self.light_map_material.texture,))
        glDeleteProgram(self.shader)
        pg.quit()

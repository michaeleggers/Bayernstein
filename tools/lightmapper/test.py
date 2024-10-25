from pathlib import Path
from renderer import Renderer

from data_structures.scene import Scene
from data_structures.color import Color

from OpenGL.GL import *


print(OpenGL.__version__)

scene = Scene().load(Path('tools/lightmapper/lightmaps/test_scene_test.json'))
renderer = Renderer(width=2160, height=1080, fov=90, scene=scene, 
                    atmosphere_color=Color(0.0, 0.0, 0.0), lightmap_mode=False, 
                    light_map_path=Path(f'tools/lightmapper/lightmaps/test_scene_test.hdr'))
renderer.run()
renderer.destroy()
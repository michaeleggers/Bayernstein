from pathlib import Path
from renderer import Renderer

from data_structures.scene import Scene
from data_structures.color import Color

path = 'test_scene'

scene = Scene().load(Path('tools/lightmapper/lightmaps/test_scene.json'))
renderer = Renderer(width=2160, height=1080, fov=90, scene=scene, atmosphere_color=Color(0.0, 0.0, 0.0), lightmap_mode=False, light_map_path=Path(f'lightmaps/test_scene.hdr'))
renderer.run()
renderer.destroy()
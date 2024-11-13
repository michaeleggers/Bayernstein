import argparse
import os
import subprocess
import json
from renderer import Renderer
from lightmapper import Lightmapper
from data_structures.scene import Scene
from data_structures.color import Color
from pathlib import Path


def compile_map(
        assets_path: Path, 
        map_path: Path, 
        output_path: Path, 
        iterations: int, 
        patches_resolution: float, 
        atmospheric_color=Color(0, 0, 0)
    ):

    temp_output_path = soup_map(assets_path=assets_path, map_path=map_path)
    
    generate_lightmaps(temp_output_path, assets_path, map_path.stem, output_path, iterations, patches_resolution, atmospheric_color)

def soup_map(assets_path: Path, map_path: Path) -> Path:

    """
    loads in a .map file, calculates triangles and saves them as a json in a temporary location
    """
    
    # the parent folder of self
    base_path = Path(__file__).resolve().parent
    souper_path = 'souper' # NOTE: Has to be discovarable by OS.
    temp_output_file = base_path / 'temp/temp.json'

    command = [str(souper_path), str(assets_path), str(map_path), str(temp_output_file)]
    subprocess.run(command, check=True)

    return temp_output_file


def generate_lightmaps(
        temp_map_path: Path, 
        assets_path: Path, 
        map_name: str, 
        output_path: Path, 
        iterations: int,
        patches_resolution: int, 
        atmospheric_color: Color
        ):
 
    # Create the scene
    scene = Scene(temp_map_path, assets_path)
    scene.create_patches(patches_resolution=patches_resolution)
    scene.generate_vertex_array()
    #scene.save_to_json(assets_path / f'compiled/{map_name}/{map_name}.json', assets_path)
    scene.save_to_json(output_path / f'{map_name}/{map_name}.json', assets_path)
    scene.save_to_binary(output_path / f'{map_name}/{map_name}.ply')

    # Create the renderer
    lightmap_renderer = Renderer(
        width=64, 
        height=64, 
        scene=scene, 
        atmosphere_color=atmospheric_color, 
        lightmap_mode=True
    )

    # Initialize the lightmapper
    lightmapper = Lightmapper(scene=scene, renderer=lightmap_renderer)

    # Generate the lightmaps
    lightmap_path = output_path / f'{map_name}/{map_name}.hdr'
    lightmapper.generate_lightmap(lightmap_path=lightmap_path, iterations=iterations)

    # Clean up
    #lightmap_renderer.destroy()

if __name__ == "__main__":

    parser = argparse.ArgumentParser(description="Compile a map for lightmapping.")
    parser.add_argument("assets_path", help="Path to the assets folder.")
    parser.add_argument("map_path", help="Path to the map file.")
    parser.add_argument("output_path", help="Path to the output folder.")
    parser.add_argument("iterations", type=int, help="Number of iterations for lightmap generation.")
    parser.add_argument("patches_resolution", type=float, help="Resolution of the patches.")
    parser.add_argument("--atmospheric_color", type=lambda s: Color(*map(float, s.split(','))), 
                        default=Color(0, 0, 0), 
                        help="Atmospheric color in format 'R,G,B'.")

    args = parser.parse_args()

    compile_map(
        assets_path=Path(args.assets_path),
        map_path=Path(args.map_path),
        output_path=Path(args.output_path),
        iterations=args.iterations,
        patches_resolution=args.patches_resolution,
        atmospheric_color=args.atmospheric_color
    )

    # EXAMPLE USAGE:
    #
    # C:/Users/Fabian/AppData/Local/Programs/Python/Python312/python.exe d:/data/Informatik/GamesEngineering/Bayernstein/tools/lightmapper/map_compiler.py D:\data\Informatik\GamesEngineering\Bayernstein\assets\ D:\data\Informatik\GamesEngineering\Bayernstein\assets\maps\test_map_open.map D:\data\Informatik\GamesEngineering\Bayernstein\assets\compiled 2 0.0625 --atmospheric_color=0.01,0.01,0.01

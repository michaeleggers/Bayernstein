# Lightmapper for TrenchBroom Scenes

This lightmapper, built primarily with PyOpenGL and GLFW, allows you to generate and visualize lightmaps for 3D scenes created in TrenchBroom. 


## Getting Started
### Prerequisites

It is recomended to use a virtual enviroment:

1. Install Conda (if not already installed)
2. You may need to initialize conda for Use in the PowerShell with:
    `conda init powershell`
3. Now you can create an enviroment with:
    `conda create --name games_engineering python=3.12`
4. Activate the enviroment with:
    `conda activate games_engineering`
5. Now install the required packages. Within this folder there is a requirements.txt with all the dependencies. You can automatically install them with `pip install -r tools/lightmapper/requirements.txt`

Before running the lightmapper, ensure the following dependencies are installed:

    - Python (3.x)
    - ipykernel (for the notebook)
    - NumPy (for array manipulation)
    - PyOpenGL (for OpenGL rendering)
    - GLFW (for window management)
    - matplotlib
    - pathlib
    - pillow
    - tqdm
    - opencv-python
    - pyrr
    - scipy

Install the dependencies with:

`pip install -r requirements.txt`


## Running the Notebook

The primary interface for the lightmapper is the lightmapper.ipynb Jupyter notebook. Open the notebook and follow these steps:

1. Configure Lightmapping Parameters

    In the lightmapper section, define the scene and lightmap settings:

    - map_filename: Name of the .obj file for the TrenchBroom map (expected in ./maps).
    - scene_filenames: Name under which the scene with its lightmap is saved in ./lightmaps.
    - patches_resolution: Defines patches per world-space unit, affecting the lightmap's resolution.
    - iterations: Specifies the number of light bounces to simulate.
    - atmosphere_color: Adds a base indirect atmospheric emission.
    - hemicube_resolution: Sets the resolution of the hemicube’s front face; affects light diffusion.

2. Generate the Lightmap 

    Run each cell in the notebook’s lightmapper section. The lightmapping process will:

    - UV-map the scenes geometry.
    - Create patches from the scene’s geometry.
    - Save the lightmap with the scene under the specified scene_filenames name.

3. Visualize the Lightmap 

    Use the lightmap visualizer section to display the generated lightmap.

    Ensure that scene_filenames matches the saved lightmapped scene you intend to view. Run the cells to visualize the scene and its lighting.


## Key Components

### Lightmapper

The lightmapper requires:

- A Renderer: Powered by PyOpenGL and GLFW to handle the lighting calculations and display.
- A Scene: Composed of entities, each representing an object or geometry within the world. Entities can be created from custom meshes, and all transformations can be applied in world space.


### Lightmap Visualizer

The visualizer reads the generated lightmap and displays it, offering insight into how the lighting will look in the final scene.


### Creating an executable

pyinstaller --onefile --upx-dir="D:\data\Informatik\GamesEngineering\Bayernstein\tools\lightmapper\upx-4.2.4-win64" --exclude-module=ipykernel --add-binary "D:\data\Informatik\GamesEngineering\Bayernstein\tools\lightmapper\glfw3.dll;." --add-binary "D:\data\Informatik\GamesEngineering\Bayernstein\tools\lightmapper\souper\bin\Debug\souper.exe;souper/bin/Debug" --add-data "D:\data\Informatik\GamesEngineering\Bayernstein\tools\lightmapper\shaders;shaders" map_compiler.py


### Running the executable

.\map_compiler.exe D:\data\Informatik\GamesEngineering\Bayernstein\assets\ D:\data\Informatik\GamesEngineering\Bayernstein\assets\maps\test_map_open.map D:\data\Informatik\GamesEngineering\Bayernstein\assets\compiled 2 0.0625 --atmospheric_color=0.01,0.01,0.01
# Map Compiler for TrenchBroom Scenes

The Map Compiler is a tool built in Python that includes a lightmapper for generating and visualizing lightmaps for 3D scenes created in TrenchBroom. It takes a `.map` file as input and outputs a `.ply` binary file along with a `.hdr` lightmap.

The Map Compiler can be run as a standard Python script. Additionally, for Windows systems, a precompiled executable is available at `tools/lightmapper/dist/mapcompiler-0.1-win.exe`. If you're using this version, you can skip the Python-specific setup and directly refer to the section Running the MapCompiler.

## Prerequisits

The Map Compiler depends on `souper` for parsing .map files. Precompiled binaries for Windows and Linux are included. If you’re using either of these operating systems, no further setup is required.

### macOS Users:

If you’re working on macOS (not recommended), you must manually rebuild the souper executable. To do this:

1. Remove the existing executable located at tools/lightmapper/souper/bin/souper.
2. Follow the guide in tools/lightmapper/souper/readme.md to rebuild it.
3. You’ll also need the SDL libraries. You can install them using brew


## How to Run the Map Compiler

The Map Compiler and its included lightmapper are written in Python. You can compile a map by running the `map_compiler.py` script like any other Python script. However, you will need to install some dependencies first. These dependencies are listed in the `requirements.txt` file.

### Setting Up the Environment

If you’re familiar with Python and have the necessary packages installed, you can skip to the **Running the MapCompiler** section. For all others, here’s how to create and configure the environment:

It’s recommended to use a virtual environment, and the the Map Compiler has been tested with Anaconda, so that's the preferred method (If you are on Linux, reade the Note below).

Important: On Linux you may encounter a segmentation fault when running the script inside a Conda environment. This issue appears to be isolated to Conda. If this happens, you can resolve it by reversing the Conda initialization:

```bash
conda init --reverse
```
Once done, proceed with installing the required packages using:
```bash
conda pip install -r requirements.txt 
```


#### 1. Install Anaconda

You can download Anaconda from here: `https://www.anaconda.com/download/success`. 

After installation, verify it by running:
```bash
conda
```

Then run:
```bash
conda init
```
Finally, restart your terminal.

If you get a `command not found` error, you'll need to add Conda to your PATH.

On Windows:
- Open System Properties → Environment Variables → Edit the Path variable.
- Add the path to the Scripts and bin folders:
`C:\Users\<YourUsername>\Anaconda3\Scripts`
`C:\Users\<YourUsername>\Anaconda3\bin`.

On Linux:
```bash
export PATH=~/anaconda3/bin:$PATH
```

#### 2. Creating the Enviroment

Create a Conda environment for the project:
```bash
conda create --name games_engineering python=3.10
```
Activate the environment:
```bash
conda activate games_engineering
```

#### 3. Installing all the packages.

To ensure we are using the correct version of pip and other packages we will use the `conda run`. This has the huge disadvantage that console outputs are hidden during runtime. This might not be necessary depending on you indivdual installations. You can try all the commands without the conda run prefix and fall back to it if you run into problems.

All the required packages can be installed with this command:

```bash
conda run pip install -r requirements.txt 
```

#### 4. Some additional stuff

Depending on your system and installed drivers, you might need to install additional packages. If you encounter the error:

`Exception: Failed to create GLFW window`

You can resolve it by installing the following drivers:

```bash
conda install -c conda-forge libgl libglu mesa
```


## Running the Map Compiler

Once everything is set up, you can run the Map Compiler using the following command:

```bash
python map_compiler.py <path/to/assets/folder> <path/to/map/file> <path/to/output/folder> <iterations> <lightmap_resolution> --atmospheric_color=<r>,<g>,<b>
```

### Example Command:

```bash
python map_compiler.py /home/devbox/Bayernstein/assets /home/devbox/Bayernstein/assets/maps/room.map /home/devbox/Bayernstein/assets/compiled 2 0.0125 --atmospheric_color=0.01,0.01,0.01
```

## Visualizer   

There is also a notebook, `lightmapper.ipynb`, which was previously used to visualize the scene with the generated lightmaps. Although this notebook is no longer maintained, you can still use the last two cells for visualization purposes.


---

---




## Some extra stuff...

### Creating an executable

pyinstaller --onefile --upx-dir="D:\data\Informatik\GamesEngineering\Bayernstein\tools\lightmapper\upx-4.2.4-win64" --exclude-module=ipykernel --add-binary "D:\data\Informatik\GamesEngineering\Bayernstein\tools\lightmapper\glfw3.dll;." --add-binary "D:\data\Informatik\GamesEngineering\Bayernstein\tools\lightmapper\souper\bin\Debug\souper.exe;souper/bin/Debug" --add-data "D:\data\Informatik\GamesEngineering\Bayernstein\tools\lightmapper\shaders;shaders" map_compiler.py


### Running the executable

.\map_compiler.exe D:\data\Informatik\GamesEngineering\Bayernstein\assets\ D:\data\Informatik\GamesEngineering\Bayernstein\assets\maps\test_map_open.map D:\data\Informatik\GamesEngineering\Bayernstein\assets\compiled 2 0.0625 --atmospheric_color=0.01,0.01,0.01


### Install the Static SDL2 Library (Unix)

Make sure you have cmake installed, you can install it with: 
```bash
sudo apt install cmake
```

1. Download and extract the SDL2 source:
```bash
wget https://github.com/libsdl-org/SDL/releases/download/release-2.30.8/SDL2-2.30.8.tar.gz
tar -xvzf SDL2-2.30.8.tar.gz
cd SDL2-2.30.8
```
2. Configure and build with static library support:
```bash
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DSDL_STATIC=ON
make
sudo make install
```
3. Locate the newly installed static library:
The static library should now be available in /usr/local/lib (or /opt/homebrew/lib on Apple Silicon).

### Building Souper (Unix)
```bash
cd tools/lightmapper/souper
mkdir build
cd build

cmake -DSDL_INCLUDE_DIR="/usr/local/include/SDL2" -DSDL_LIBRARIES_DIR="/usr/local/lib" ..
make
```

### Building Souper (Unix/Linux):

#### Install SDL2 devel (Debian):
```bash
sudo apt-get install libsdl2-dev
```
#### Install SDL2 devel (Fedora):
```bash
sudo dnf install SDL2-devel
```

```bash
cd tools/lightmapper/souper
mkdir build
cd build

cmake -DSDL_INCLUDE_DIR="/usr/include/SDL2" -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ .. # Fedora
cmake -DSDL_INCLUDE_DIR="/usr/local/include/SDL2" -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ .. # Ubuntu
make
```

### Building Souper (Windows):
Download the SDL2 development libs: https://github.com/libsdl-org/SDL/releases/download/release-2.30.10/SDL2-devel-2.30.10-VC.zip and extract its contents to a folder called `SDL2` (or anything that you can remember). The folder structure should look like this:
souper
...
build
...
└───SDL2
    ├───cmake
    ├───docs
    ├───include
    └───lib
        ├───x64
        └───x86

In the souper directory make a `build` folder and `cd` into it.
Then run CMake:
```bash
cmake -DSDL_INCLUDE_DIR=SDL2\include -DSDL_LIBS_DIR=SDL2\lib\x64 ..
```
It will create a Visual Studio Solution that you can use to build souper with.
The generated exe will be in `souper/bin/Release` or `souper/bin/Debug`. Make sure
to copy the `SDL2.dll` from `SDL2/lib/x64/` alongside the `souper.exe`.

### Running Python File on Unix
if: Failed to create GLFW window"
then do:
sudo apt install --reinstall libgl1-mesa-glx libgl1-mesa-dri libglu1-mesa mesa-utils
conda install -c conda-forge libgl libglu mesa

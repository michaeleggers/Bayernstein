from cx_Freeze import setup, Executable

setup(
    name="map_compiler",
    version="1.0",
    description="Reads .map files, compiles them into .ply binaries and precalculates ligthing.",
    executables=[Executable("map_compiler.py")]
)
#!/usr/bin/bash

# Copies .PLY and .PNG files into the game-assets'
# maps/ and textures/ directories.
#
# example usage:
# ./cp_lightmaps.sh compiled_maps lightmap_test ~/repos/Bayernstein/assets
# 
# Assuming the following folder structure in the dir where this script is located.
# 
#compiled_maps/
# ├── lightmap_test
# │   ├── lightmap_test.hdr
# │   ├── lightmap_test.json
# │   ├── lightmap_test.ply
# │   └── lightmap_test.png
# └── room
#     ├── room.hdr
#     ├── room.json
#     ├── room.ply
#     └── room.png
# ...
#

MAPFOLDER=$1
MAPNAME=$2
GAME_ASSETS_FOLDER=$3

$(cp ${MAPFOLDER}/${MAPNAME}/${MAPNAME}.ply ${GAME_ASSETS_FOLDER}/maps)
$(cp ${MAPFOLDER}/${MAPNAME}/${MAPNAME}.png ${GAME_ASSETS_FOLDER}/textures)


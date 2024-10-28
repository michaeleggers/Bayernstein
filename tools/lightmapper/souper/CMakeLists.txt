cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(SOUPER VERSION 1.0)

macro(ADD_SRC_HEADER_FILES)
    file(GLOB_RECURSE SRC_FILES LIST_DIRECTORIES false RELATIVE
            ${CMAKE_CURRENT_SOURCE_DIR} *.c??)
    file(GLOB_RECURSE HEADER_FILES LIST_DIRECTORIES false RELATIVE
            ${CMAKE_CURRENT_SOURCE_DIR} *.h)
endmacro()

# ADD_SRC_HEADER_FILES()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")

# set(CMAKE_CXX_FLAGS "-fms-extensions")
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message("############## Using Clang compiler ##############")
    add_compile_options(-fms-extensions)
endif ()

set(CMAKE_VERBOSE_MAKEFILE ON)


if (MSVC)
    add_compile_options(/W4)
endif ()

add_executable(souper
    main.cpp
    ../../../map_parser.h
    ../../../polysoup.h
    ../../../polysoup.cpp
    ../../../platform.h
    ../../../platform.cpp
    ../../../image.h
    ../../../image.cpp
)

target_include_directories(souper
    PUBLIC ../../../
    PUBLIC ../../../dependencies/
    PUBLIC ../../../dependencies/SDL2.28.4/include/
)
message("SDL2 libs dir is set to: ${CMAKE_SOURCE_DIR}/${SDL_LIBRARIES_DIR}" )

target_link_directories(souper PUBLIC
    ${CMAKE_SOURCE_DIR}/${SDL_LIBRARIES_DIR}
)

target_link_libraries(souper PUBLIC
    SDL2
    SDL2main
)





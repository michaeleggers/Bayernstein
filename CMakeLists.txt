cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(HKD VERSION 1.0)

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

if (WIN32)
    # res/app.rc
    SET(RESOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/res/app.rc)
endif()

if (MSVC)
    add_compile_options(/W4)
endif ()

add_subdirectory(dependencies/SDL2.28.4)


# add_executable(Game
#   ${SRC_FILES}
#   ${HEADER_FILES}
# ) 


add_executable(Game

    main.cpp
    hkd_interface.h
    platform.h
    platform.cpp
    input.h
    input.cpp
    iqm_loader.h
    iqm_loader.cpp
    irender.h
    r_itexture.h
    r_gl.h
    r_gl.cpp
    r_model.h
    r_model.cpp
    r_common.h
    r_common.cpp
    r_font.h
    r_font.cpp

    r_gl_batch.h
    r_gl_batch.cpp
    r_gl_shader.h
    r_gl_shader.cpp
    r_gl_texture.h
    r_gl_texture.cpp
    r_gl_texture_mgr.h
    r_gl_texture_mgr.cpp

    r_gl_fbo.h
    r_gl_fbo.cpp
    r_gl_rendertexture.h
    r_gl_rendertexture.cpp
    
    camera.h
    camera.cpp
    physics.h
    physics.cpp
    utils.h
    utils.cpp
    polysoup.h
    polysoup.cpp
        # map_parser.h
    game.h
    game.cpp    

    dependencies/stb_image.h
    dependencies/stb_image_write.h
    dependencies/stb_truetype.h

    dependencies/imgui/imgui.h
    dependencies/imgui/imconfig.h
    dependencies/imgui/imgui_internal.h
    dependencies/imgui/imstb_rectpack.h
    dependencies/imgui/imstb_textedit.h
    dependencies/imgui/imstb_truetype.h

    dependencies/imgui/misc/cpp/imgui_stdlib.h
    dependencies/imgui/misc/cpp/imgui_stdlib.cpp

    dependencies/imgui/imgui.cpp
    dependencies/imgui/imgui_draw.cpp
    dependencies/imgui/imgui_tables.cpp
    dependencies/imgui/imgui_widgets.cpp
    dependencies/imgui/imgui_demo.cpp

    dependencies/imgui/backends/imgui_impl_opengl3.cpp
    dependencies/imgui/backends/imgui_impl_opengl3.h
    dependencies/imgui/backends/imgui_impl_sdl2.cpp
    dependencies/imgui/backends/imgui_impl_sdl2.h

    dependencies/glad/src/glad.c

    TestClass.cpp
    TestClass.h
    Shape.cpp
    Shape.h
    ShapeSphere.cpp
    ShapeSphere.h
    Body.cpp
    Body.h
    Intersections.cpp
    Intersections.h
    CircularBuffer.cpp
    CircularBuffer.h
    Console/Console.cpp
    Console/Console.h
    Console/CommandManager.cpp
    Console/CommandManager.h
    Console/VariableManager.cpp
    Console/VariableManager.h
    CWorld.cpp
    CWorld.h
    collision.cpp
    collision.h
    Clock/clock.cpp
    Clock/clock.h
    Message/message_dispatcher.cpp
    Message/message_dispatcher.h
    Message/telegram.h
    Entity/base_game_entity.cpp
    Entity/base_game_entity.h
    FSM/state_machine.h
    FSM/istate.h
    Entity/entity_manager.cpp
    Entity/entity_manager.h
    Entity/Player/g_player_states.cpp
    Entity/Player/g_player_states.h
    Entity/Player/g_player.cpp
    Entity/Player/g_player.h
    Entity/Enemy/g_enemy_states.cpp
    Entity/Enemy/g_enemy_states.h
    Entity/Enemy/g_enemy.cpp
    Entity/Enemy/g_enemy.h
    Entity/Door/g_door.h
    Entity/Door/g_door.cpp
    Entity/Door/g_door_states.h
    Entity/Door/g_door_states.cpp
    
    ${RESOURCE_FILES}
)

target_include_directories(Game
    PUBLIC dependencies
    PUBLIC dependencies/glad/include
    PUBLIC dependencies/imgui
    PUBLIC dependencies/SDL2.28.4/include
)


target_link_libraries(Game PUBLIC
    SDL2
    SDL2main
)




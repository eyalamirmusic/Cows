# Reads the meshes of OBJ files (as Blender exports them) into indexed
# triangles, one group per material. Knows nothing of the GPU or the game.
add_library(cowsinlove_ObjLoader STATIC
        ObjLoader/ObjFile.cpp)

target_include_directories(cowsinlove_ObjLoader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(cowsinlove_ObjLoader PUBLIC eacp-core)

set_default_target_setting(cowsinlove_ObjLoader)

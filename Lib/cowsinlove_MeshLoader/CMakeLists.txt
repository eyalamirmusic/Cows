# Reads the mesh blobs the Blender scripts write (tools/blender/README.md):
# indexed triangles, one group per material. Knows nothing of the GPU.
add_library(cowsinlove_MeshLoader STATIC
        MeshLoader/MeshBlob.cpp)

target_include_directories(cowsinlove_MeshLoader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(cowsinlove_MeshLoader PUBLIC eacp-core)

set_default_target_setting(cowsinlove_MeshLoader)

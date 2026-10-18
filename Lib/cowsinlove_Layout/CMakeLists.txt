# Flexbox-lite layout: trees of boxes resolved to rects, shared columns and
# fit-to-size. Knows nothing of fonts or the GPU, so eacp could take it.
add_library(cowsinlove_Layout STATIC
        Layout/FitToSize.cpp
        Layout/Tree.cpp)

target_include_directories(cowsinlove_Layout PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(cowsinlove_Layout PUBLIC eacp-graphics)

set_default_target_setting(cowsinlove_Layout)

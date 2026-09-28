add_library(cowsinlove_World STATIC
        Level.cpp
        Levels/Meadow.cpp
        Terrain/Grass.cpp
        Terrain/TerrainShaders.cpp)

target_include_directories(cowsinlove_World PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(cowsinlove_World PUBLIC cowsinlove_Actors)

set_default_target_setting(cowsinlove_World)

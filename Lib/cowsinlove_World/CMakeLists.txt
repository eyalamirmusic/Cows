add_library(cowsinlove_World STATIC
        Level.cpp
        Levels/Biome.cpp
        Levels/Layout.cpp
        Levels/LevelGenerator.cpp
        Levels/Region.cpp
        Levels/Segments/JumpLineSegment.cpp
        Levels/Segments/MeadowSegment.cpp
        Levels/Segments/MoverSpawner.cpp
        Levels/Segments/RavineSegment.cpp
        Terrain/Grass.cpp
        Terrain/Ground.cpp
        Terrain/TerrainShaders.cpp)

target_include_directories(cowsinlove_World PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(cowsinlove_World PUBLIC cowsinlove_Actors)

set_default_target_setting(cowsinlove_World)

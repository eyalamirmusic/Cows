add_library(cowsinlove_Game STATIC
        Ending.cpp
        Game.cpp
        Input.cpp
        Title/TitleFont.cpp
        Title/TitleShader.cpp)

target_include_directories(cowsinlove_Game PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(cowsinlove_Game PUBLIC cowsinlove_World)

set_default_target_setting(cowsinlove_Game)

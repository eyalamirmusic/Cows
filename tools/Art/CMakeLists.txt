# Renders the store art (app icon, key art, logo) and the store screenshots
# from the game itself. A macOS tool; never shipped.
add_executable(CowsArt
        Main.cpp
        LogoView.cpp
        Screenshots.cpp
        ${CMAKE_SOURCE_DIR}/Apps/CowsInLove/Source/Stages.cpp
        ${CMAKE_SOURCE_DIR}/Apps/CowsInLove/Source/Templates.cpp
        ${CMAKE_SOURCE_DIR}/Apps/CowsInLove/Source/Scene/CowsView.cpp)

target_include_directories(CowsArt PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/Apps/CowsInLove/Source)
target_link_libraries(CowsArt PRIVATE cowsinlove_Game)
target_compile_definitions(CowsArt PRIVATE
        COWS_RENDERS_DIR="${CMAKE_SOURCE_DIR}/Deploy/Art/Renders")

set_default_target_setting(CowsArt)

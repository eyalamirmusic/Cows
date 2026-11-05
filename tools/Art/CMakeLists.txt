# Renders the store art (app icon, key art, logo) and the store screenshots
# from the game itself. A macOS tool; never shipped.
add_executable(CowsArt
        Main.cpp
        LogoView.cpp
        Screenshots.cpp
        ${CMAKE_SOURCE_DIR}/Apps/CowsInLove/Source/Stages.cpp
        ${CMAKE_SOURCE_DIR}/Apps/CowsInLove/Source/Templates.cpp
        ${CMAKE_SOURCE_DIR}/Apps/CowsInLove/Source/Scene/CowsView.cpp
        ${CMAKE_SOURCE_DIR}/Apps/CowsInLove/Source/Scene/Quality.cpp)

target_include_directories(CowsArt PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/Apps/CowsInLove/Source)
target_link_libraries(CowsArt PRIVATE cowsinlove_Game)
target_compile_definitions(CowsArt PRIVATE
        COWS_RENDERS_DIR="${CMAKE_SOURCE_DIR}/Deploy/Art/Renders"
        COWS_APP_VERSION="${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}"
        COWS_APP_BUILD="${PROJECT_VERSION_TWEAK}")

set_default_target_setting(CowsArt)

# The cow redesign pitch's mockups (docs/design/cow-redesign.md), drawn through
# the tests' SnapshotView. A prototype; nothing in Lib uses it.
if (COWS_BUILD_TESTS)
    add_executable(CowPitch
            CowPitch/Main.cpp
            CowPitch/Designs.cpp
            CowPitch/Refinements.cpp)
    target_link_libraries(CowPitch PRIVATE cowsinlove_Actors cowsinlove_TestSupport)
    set_default_target_setting(CowPitch)
endif ()

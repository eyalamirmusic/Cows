nano_add_executable(CowsInLoveTests
        NO_MAIN
        SOURCES
            StagesTests.cpp
            ${CMAKE_SOURCE_DIR}/Apps/CowsInLove/Source/Stages.cpp
            ${CMAKE_SOURCE_DIR}/Apps/CowsInLove/Source/Templates.cpp
            ../Support/TestMain.cpp
        TARGETS cowsinlove_Game)

target_include_directories(CowsInLoveTests PRIVATE ${CMAKE_SOURCE_DIR}/Apps/CowsInLove/Source)
set_default_target_setting(CowsInLoveTests)

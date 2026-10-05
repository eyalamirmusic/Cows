nano_add_executable(CowsInLoveTests
        NO_MAIN
        SOURCES
            QualityTests.cpp
            StagesTests.cpp
            ${CMAKE_SOURCE_DIR}/Apps/CowsInLove/Source/Scene/Quality.cpp
            ${CMAKE_SOURCE_DIR}/Apps/CowsInLove/Source/Stages.cpp
            ${CMAKE_SOURCE_DIR}/Apps/CowsInLove/Source/Templates.cpp
            ../Support/TestMain.cpp
        TARGETS cowsinlove_Game)

target_include_directories(CowsInLoveTests PRIVATE ${CMAKE_SOURCE_DIR}/Apps/CowsInLove/Source)
set_default_target_setting(CowsInLoveTests)

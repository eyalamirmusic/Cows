add_library(cowsinlove_TestSupport STATIC Snapshot.cpp)

target_include_directories(cowsinlove_TestSupport PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(cowsinlove_TestSupport PUBLIC cowsinlove_Engine)
target_compile_definitions(cowsinlove_TestSupport PUBLIC
        COWS_SHOTS_DIR="${PROJECT_SOURCE_DIR}/docs/shots/tests")

set_default_target_setting(cowsinlove_TestSupport)

add_library(cowsinlove_TestFixtures STATIC Fixtures.cpp)

target_include_directories(cowsinlove_TestFixtures PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(cowsinlove_TestFixtures PUBLIC cowsinlove_World)

set_default_target_setting(cowsinlove_TestFixtures)

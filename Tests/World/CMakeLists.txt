nano_add_executable(WorldTests
        NO_MAIN
        SOURCES
            GrassTests.cpp
            LevelTests.cpp
            MeadowTests.cpp
            ../Support/TestMain.cpp
        TARGETS cowsinlove_World cowsinlove_TestSupport)

set_default_target_setting(WorldTests)

nano_add_executable(WorldTests
        NO_MAIN
        SOURCES
            BiomeTests.cpp
            GrassTests.cpp
            GroundTests.cpp
            LayoutTests.cpp
            LevelGeneratorTests.cpp
            LevelTests.cpp
            MeadowTests.cpp
            RavineSegmentTests.cpp
            ../Support/TestMain.cpp
        TARGETS cowsinlove_World cowsinlove_TestSupport cowsinlove_TestFixtures)

set_default_target_setting(WorldTests)

nano_add_executable(GameTests
        NO_MAIN
        SOURCES
            EndingTests.cpp
            BarnJumpTests.cpp
            GameTests.cpp
            RespawnTests.cpp
            InputTests.cpp
            PadTests.cpp
            PlayTouchTests.cpp
            TitleFontTests.cpp
            ../Support/TestMain.cpp
        TARGETS cowsinlove_Game cowsinlove_TestSupport cowsinlove_TestFixtures
                cowsinlove_TestCow)

set_default_target_setting(GameTests)

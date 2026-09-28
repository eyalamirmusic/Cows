nano_add_executable(GameTests
        NO_MAIN
        SOURCES
            EndingTests.cpp
            GameTests.cpp
            InputTests.cpp
            TitleFontTests.cpp
            ../Support/TestMain.cpp
        TARGETS cowsinlove_Game cowsinlove_TestSupport)

set_default_target_setting(GameTests)

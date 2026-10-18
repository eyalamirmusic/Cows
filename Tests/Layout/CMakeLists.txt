nano_add_executable(LayoutTests
        NO_MAIN
        SOURCES
            FitToSizeTests.cpp
            TreeTests.cpp
            ../Support/TestMain.cpp
        TARGETS cowsinlove_Layout)

set_default_target_setting(LayoutTests)

nano_add_executable(ObjLoaderTests
        NO_MAIN
        SOURCES
            ObjFileTests.cpp
            ../Support/TestMain.cpp
        TARGETS cowsinlove_ObjLoader)

set_default_target_setting(ObjLoaderTests)

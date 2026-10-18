nano_add_executable(MeshLoaderTests
        NO_MAIN
        SOURCES
            MeshBlobTests.cpp
            ../Support/TestMain.cpp
        TARGETS cowsinlove_MeshLoader)

target_compile_definitions(MeshLoaderTests PRIVATE
        COWS_RESOURCES_DIR="${PROJECT_SOURCE_DIR}/Lib/cowsinlove_Actors/Resources")

set_default_target_setting(MeshLoaderTests)

nano_add_executable(EngineTests
        NO_MAIN
        SOURCES
            HudSnapshotTests.cpp
            InstancesTests.cpp
            MeshTests.cpp
            OrbitCameraTests.cpp
            PaletteTests.cpp
            RenderSnapshotTests.cpp
            TouchControlsTests.cpp
            ../Support/TestMain.cpp
        TARGETS cowsinlove_Engine cowsinlove_TestSupport)

set_default_target_setting(EngineTests)

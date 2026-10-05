nano_add_executable(EngineTests
        NO_MAIN
        SOURCES
            EditorSnapshotTests.cpp
            FrustumTests.cpp
            HudSnapshotTests.cpp
            InstancesTests.cpp
            MenuSnapshotTests.cpp
            MenuTouchTests.cpp
            RootKeyTests.cpp
            MeshTests.cpp
            NoiseLatticeTests.cpp
            OrbitCameraTests.cpp
            PaletteTests.cpp
            RenderSnapshotTests.cpp
            TouchControlsTests.cpp
            ../Support/TestMain.cpp
        TARGETS cowsinlove_Engine cowsinlove_TestSupport)

set_default_target_setting(EngineTests)

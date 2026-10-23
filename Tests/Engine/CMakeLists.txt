nano_add_executable(EngineTests
        NO_MAIN
        SOURCES
            EditorSnapshotTests.cpp
            FontCacheTests.cpp
            FrustumTests.cpp
            HudSnapshotTests.cpp
            InstancesTests.cpp
            MenuSnapshotTests.cpp
            MenuTouchTests.cpp
            RootKeyTests.cpp
            ScreenFrameTests.cpp
            ScreenWalkTests.cpp
            MeshTests.cpp
            NoiseLatticeTests.cpp
            OrbitCameraTests.cpp
            PaletteTests.cpp
            QualityGovernorTests.cpp
            RenderSnapshotTests.cpp
            TouchControlsTests.cpp
            ../Support/TestMain.cpp
        TARGETS cowsinlove_Engine cowsinlove_TestSupport)

set_default_target_setting(EngineTests)

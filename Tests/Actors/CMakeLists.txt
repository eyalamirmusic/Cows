nano_add_executable(ActorsTests
        NO_MAIN
        SOURCES
            ChoreographyTests.cpp
            CollisionTests.cpp
            CowTests.cpp
            KissHeartsTests.cpp
            MooTests.cpp
            PropsTests.cpp
            SkyTests.cpp
            ../Support/TestMain.cpp
        TARGETS cowsinlove_Actors cowsinlove_TestSupport)

set_default_target_setting(ActorsTests)

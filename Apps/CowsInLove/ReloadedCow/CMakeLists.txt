# The cow's parts, jump and animation, reached only through makeCowModel. The
# app gets it through Plugins::Reloadable, and CowsArt and the tests compile in
# its objects ($<TARGET_OBJECTS:ReloadedCow>) and do the same.
eacp_add_hot_reload(Cows ReloadedCow
        SOURCES
            CowAnimation.cpp
            CowJump.cpp
            CowModelFactory.cpp
            CowParts.cpp
        HOT_RELOAD ${COWS_HOT_RELOAD_COW})
target_link_libraries(ReloadedCow PRIVATE cowsinlove_Actors)

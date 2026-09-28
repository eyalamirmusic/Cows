add_library(cowsinlove_Actors STATIC
        Animation/Choreography.cpp
        Cow/Cow.cpp
        Cow/HeartMesh.cpp
        Cow/KissHearts.cpp
        Cow/Moo.cpp
        Props/Bale.cpp
        Props/Barn.cpp
        Props/Bridge.cpp
        Props/Collision.cpp
        Props/Crate.cpp
        Props/Fence.cpp
        Props/Hedge.cpp
        Props/Log.cpp
        Props/Mover.cpp
        Props/Props.cpp
        Props/Rock.cpp
        Props/Tree.cpp
        Sky/SkyDecor.cpp)

target_include_directories(cowsinlove_Actors PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(cowsinlove_Actors PUBLIC cowsinlove_Engine cowsinlove_AudioEngine)

set_default_target_setting(cowsinlove_Actors)

res_embed_add(cowsinlove_Actors
        FILES Resources/moo.f32
        NAMESPACE Sounds
        CATEGORY Sounds)

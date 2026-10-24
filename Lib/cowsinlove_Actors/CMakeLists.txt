
add_library(cowsinlove_Actors STATIC
        Animation/Choreography.cpp
        Cow/CowSkin.cpp
        Cow/HeartMesh.cpp
        Cow/KissHearts.cpp
        Cow/Moo.cpp
        Cow/Wardrobe.cpp
        Props/Bale.cpp
        Props/Barn.cpp
        Props/Bridge.cpp
        Props/Collision.cpp
        Props/Crate.cpp
        Props/Fence.cpp
        Props/Hedge.cpp
        Props/Log.cpp
        Props/Meshes.cpp
        Props/Mover.cpp
        Props/PropMeshes.cpp
        Props/Props.cpp
        Props/Rock.cpp
        Props/Tree.cpp
        Sky/SkyDecor.cpp)

target_compile_definitions(cowsinlove_Actors PUBLIC COWS_COW_MODEL_SOURCE_DIRECTORY="${CMAKE_CURRENT_LIST_DIR}/Cow")

target_include_directories(cowsinlove_Actors PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(cowsinlove_Actors PUBLIC cowsinlove_Engine cowsinlove_AudioEngine cowsinlove_MeshLoader Miro)

set_default_target_setting(cowsinlove_Actors)

res_embed_add(cowsinlove_Actors
        FILES Resources/moo.f32
        NAMESPACE Sounds
        CATEGORY Sounds)

res_embed_add(cowsinlove_Actors
        FILES Resources/barn.mesh Resources/bale.mesh
        NAMESPACE Models
        CATEGORY Models)

eacp_add_hot_reload(cowsinlove_Actors MODULE ReloadedCows
        SOURCES Cow/Cow.cpp HOT_RELOAD ${COWS_HOT_RELOAD_COW})

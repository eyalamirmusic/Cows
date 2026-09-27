add_executable(Cows
        Source/Main.cpp
        Source/CowsApp.cpp
        Source/Scene/Choreography.cpp
        Source/Scene/Cow.cpp
        Source/Scene/CowsView.cpp
        Source/Scene/Game.cpp
        Source/Scene/Grass.cpp
        Source/Scene/HeartMesh.cpp
        Source/Scene/Instances.cpp
        Source/Scene/KissHearts.cpp
        Source/Scene/Mesh.cpp
        Source/Scene/Moo.cpp
        Source/Scene/Obstacles.cpp
        Source/Scene/OrbitCamera.cpp
        Source/Scene/Overlay.cpp
        Source/Scene/Shaders.cpp
        Source/Scene/Shading.cpp
        Source/Scene/ShadowMap.cpp
        Source/Scene/SkyDecor.cpp
        Source/Scene/TitleFont.cpp
        Source/Scene/TouchControls.cpp)

if (IOS)
    set(COWS_PLATFORM Source/iOS)
    target_sources(Cows PRIVATE
            Source/iOS/Platform.cpp
            Source/iOS/TouchSurface.mm)
    target_link_libraries(Cows PRIVATE "-framework AVFoundation")
else ()
    set(COWS_PLATFORM Source/macOS)
endif ()

target_include_directories(Cows PRIVATE Source ${COWS_PLATFORM})
target_link_libraries(Cows PRIVATE eacp-gpu "-framework AudioToolbox")

set(BUNDLE_ID "com.tamber.cows")

set_target_properties(Cows PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "Cows"
        MACOSX_BUNDLE_GUI_IDENTIFIER ${BUNDLE_ID}
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${BUNDLE_ID})

eacp_set_gui_subsystem(Cows)
set_default_target_setting(Cows)

res_embed_add(Cows
        FILES Resources/moo.f32
        NAMESPACE Sounds
        CATEGORY Sounds)

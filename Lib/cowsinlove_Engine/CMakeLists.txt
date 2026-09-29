add_library(cowsinlove_Engine STATIC
        Camera/OrbitCamera.cpp
        Render/FrameProfile.cpp
        Render/Instances.cpp
        Render/Mesh.cpp
        Render/Shaders.cpp
        Render/Shading.cpp
        Render/ShadowMap.cpp
        UI/Hud.cpp
        UI/Overlay.cpp
        UI/TouchControls.cpp)

# The browser answers at run time what the native platforms know at compile time.
if (EMSCRIPTEN)
    target_sources(cowsinlove_Engine PRIVATE Platform/Device-Web.cpp)
else ()
    target_sources(cowsinlove_Engine PRIVATE Platform/Device.cpp)
endif ()

target_include_directories(cowsinlove_Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(cowsinlove_Engine PUBLIC eacp-gpu eacp-ui)

set_default_target_setting(cowsinlove_Engine)

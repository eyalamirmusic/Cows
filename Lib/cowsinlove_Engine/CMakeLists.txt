add_library(cowsinlove_Engine STATIC
        Camera/OrbitCamera.cpp
        Render/FrameProfile.cpp
        Render/Instances.cpp
        Render/Mesh.cpp
        Render/Shaders.cpp
        Render/Shading.cpp
        Render/ShadowMap.cpp
        Render/ShapeMeshes.cpp
        UI/Hud.cpp
        UI/Menu.cpp
        UI/MenuStyle.cpp
        UI/Editor.cpp
        UI/Overlay.cpp
        UI/TouchControls.cpp)

target_include_directories(cowsinlove_Engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(cowsinlove_Engine PUBLIC eacp-gpu eacp-ui)

set_default_target_setting(cowsinlove_Engine)

res_embed_add(cowsinlove_Engine
        FILES Resources/ComicNeue-Bold.ttf
        NAMESPACE Fonts
        CATEGORY Fonts)

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

if (IOS)
    set(COWS_PLATFORM Platform/iOS)
    target_sources(cowsinlove_Engine PRIVATE
            Platform/iOS/Platform.cpp
            Platform/iOS/TouchSurface.mm)
    target_link_libraries(cowsinlove_Engine PUBLIC "-framework AVFoundation")
elseif (ANDROID)
    set(COWS_PLATFORM Platform/Android)
    target_sources(cowsinlove_Engine PRIVATE
            Platform/Android/Platform.cpp
            Platform/Android/TouchSurface.cpp)
else ()
    set(COWS_PLATFORM Platform/macOS)
endif ()

target_include_directories(cowsinlove_Engine PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/${COWS_PLATFORM})
target_link_libraries(cowsinlove_Engine PUBLIC eacp-gpu eacp-ui)

set_default_target_setting(cowsinlove_Engine)

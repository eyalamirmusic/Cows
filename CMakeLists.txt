cmake_minimum_required(VERSION 3.31)
project(Cows VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (APPLE)
    enable_language(OBJCXX)
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_LINK_OBJC_RUNTIME NO)
endif ()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

# Neither has ALSA, which MakeASound's RtMidi asks for on every UNIX.
if (ANDROID OR EMSCRIPTEN)
    list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake/NoALSA")
endif ()

# eacp supplies both the GPU module and the CMake helpers every target here is
# configured with (set_default_target_setting, eacp_set_gui_subsystem), so it
# is fetched before those helpers are included.
find_package(eacp REQUIRED)

list(APPEND CMAKE_MODULE_PATH "${eacp_SOURCE_DIR}/CMake")
include(TargetSetup)

# eacp's web flags (wasm exceptions, which every object in the link must share,
# and the page and link options eacp_web_app reads) are set in eacp's own
# directory, so they are set again here, ahead of every target and of
# MakeASound.
if (EMSCRIPTEN)
    include(WebSetup)
endif ()

# eacp only runs this for itself when it is the top-level project, so a consumer
# has to call it or the Apple deployment target is never set. It points the
# bundle plist templates at this project's CMake directory, where the store
# copies live (display name, category, launch screen, portrait-only iOS). eacp
# also forces its own signing team; COWS_IOS_TEAM replaces it.
if (PROJECT_IS_TOP_LEVEL)
    eacp_default_setup()

    if (IOS)
        set(COWS_IOS_TEAM "" CACHE STRING "Apple team that signs the iOS app")
        set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${COWS_IOS_TEAM}"
                CACHE STRING "" FORCE)
        set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Apple Development"
                CACHE STRING "" FORCE)
        set(CMAKE_OSX_DEPLOYMENT_TARGET "15.0" CACHE STRING "" FORCE)
    endif ()
endif ()

option(COWS_BUILD_TESTS "Build the test suites" ON)

add_subdirectory(Lib)
add_subdirectory(Apps)

if (APPLE AND NOT IOS)
    add_subdirectory(tools/Art)
endif ()

if (COWS_BUILD_TESTS AND NOT IOS AND NOT EMSCRIPTEN)
    enable_testing()
    add_subdirectory(Tests)
endif ()

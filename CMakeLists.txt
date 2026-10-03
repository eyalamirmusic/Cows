cmake_minimum_required(VERSION 3.31)

if (NOT CMAKE_OSX_DEPLOYMENT_TARGET)
    if (CMAKE_SYSTEM_NAME STREQUAL "iOS")
        set(CMAKE_OSX_DEPLOYMENT_TARGET "15.0" CACHE STRING "Minimum iOS version")
    elseif (CMAKE_HOST_APPLE AND NOT CMAKE_SYSTEM_NAME AND NOT CMAKE_TOOLCHAIN_FILE)
        set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version")
    endif ()
endif ()

project(Cows VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

if (APPLE)
    enable_language(OBJCXX)
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_LINK_OBJC_RUNTIME NO)
endif ()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

if (ANDROID)
    list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake/Android")
endif ()

# eacp supplies both the GPU module and the CMake helpers every target here is
# configured with (set_default_target_setting, eacp_set_gui_subsystem), so it
# is fetched before those helpers are included.
find_package(eacp REQUIRED)

list(APPEND CMAKE_MODULE_PATH "${eacp_SOURCE_DIR}/CMake")
include(TargetSetup)

if (IOS)
    set(COWS_IOS_TEAM "" CACHE STRING "Apple team that signs the iOS app")
    set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${COWS_IOS_TEAM}")
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Apple Development" CACHE STRING "")
endif ()

option(COWS_BUILD_TESTS "Build the test suites" ON)

add_subdirectory(Lib)
add_subdirectory(Apps)

if (APPLE AND NOT IOS)
    add_subdirectory(tools/Art)
endif ()

if (COWS_BUILD_TESTS AND NOT IOS AND NOT ANDROID)
    enable_testing()
    add_subdirectory(Tests)
endif ()

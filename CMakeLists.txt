cmake_minimum_required(VERSION 3.31)
project(Cows VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (APPLE)
    enable_language(OBJCXX)
    set(CMAKE_XCODE_ATTRIBUTE_CLANG_LINK_OBJC_RUNTIME NO)
endif ()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake")

# eacp supplies both the GPU module and the CMake helpers every target here is
# configured with (set_default_target_setting, eacp_set_gui_subsystem), so it
# is fetched before those helpers are included.
find_package(eacp REQUIRED)

list(APPEND CMAKE_MODULE_PATH "${eacp_SOURCE_DIR}/CMake")
include(TargetSetup)

# eacp only runs this for itself when it is the top-level project, so a consumer
# has to call it or the Apple deployment target is never set. It points the
# bundle plist template at this project's CMake directory, so the template is
# pointed back at eacp's own copy straight after rather than vendored here.
if (PROJECT_IS_TOP_LEVEL)
    eacp_default_setup()

    if (APPLE AND NOT IOS)
        set(EACP_MACOS_PLIST
                "${eacp_SOURCE_DIR}/CMake/macOSBundleInfo.plist.in"
                CACHE INTERNAL "")
    endif ()
endif ()

add_subdirectory(App)

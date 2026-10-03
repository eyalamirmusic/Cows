# The android preset's toolchain: the NDK eacp pins (EACP_ANDROID_NDK_VERSION in
# eacp's CMake/AndroidVersions.cmake), from $ANDROID_HOME, else
# $ANDROID_SDK_ROOT, else where Android Studio installs the SDK on this host.
# eacp's own CMake/AndroidToolchain.cmake does the same, but only for a
# top-level eacp: Cows fetches eacp after project(), too late for a toolchain.

set(COWS_NDK 30.0.16248370)

set(cows_sdk "$ENV{ANDROID_HOME}")

if (NOT cows_sdk)
    set(cows_sdk "$ENV{ANDROID_SDK_ROOT}")
endif ()

if (NOT cows_sdk)
    if (CMAKE_HOST_WIN32)
        set(cows_sdk "$ENV{LOCALAPPDATA}/Android/Sdk")
    else ()
        set(cows_sdk "$ENV{HOME}/Library/Android/sdk")
    endif ()
endif ()

file(TO_CMAKE_PATH "${cows_sdk}" cows_sdk)
set(cows_ndk_toolchain "${cows_sdk}/ndk/${COWS_NDK}/build/cmake/android.toolchain.cmake")

if (NOT EXISTS "${cows_ndk_toolchain}")
    message(FATAL_ERROR "No NDK ${COWS_NDK} in ${cows_sdk}: install it with "
            "Android Studio's SDK Manager, or set ANDROID_HOME to an SDK that has it.")
endif ()

include("${cows_ndk_toolchain}")

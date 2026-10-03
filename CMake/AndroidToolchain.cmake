# The android preset's toolchain: the NDK COWS_NDK locks to, else the one
# $ANDROID_NDK_HOME, $ANDROID_NDK_ROOT or $ANDROID_NDK names, else the newest
# NDK in the SDK ($ANDROID_HOME, else $ANDROID_SDK_ROOT, else where Android
# Studio installs it on this host). eacp's own CMake/AndroidToolchain.cmake does
# the same, but only for a top-level eacp: Cows fetches eacp after project(),
# too late for a toolchain.

set(COWS_NDK "" CACHE STRING "An NDK version to build with, else the newest installed")

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

set(cows_ndk "$ENV{ANDROID_NDK_HOME}")

if (NOT cows_ndk)
    set(cows_ndk "$ENV{ANDROID_NDK_ROOT}")
endif ()

if (NOT cows_ndk)
    set(cows_ndk "$ENV{ANDROID_NDK}")
endif ()

if (COWS_NDK)
    set(cows_ndk "${cows_sdk}/ndk/${COWS_NDK}")
    set(cows_ndk_missing "No NDK ${COWS_NDK} in ${cows_sdk}")
elseif (cows_ndk)
    set(cows_ndk_missing "No NDK at ${cows_ndk}")
else ()
    set(cows_ndk_missing "No NDK in ${cows_sdk}")
    file(GLOB cows_ndks LIST_DIRECTORIES true "${cows_sdk}/ndk/*")
    list(SORT cows_ndks COMPARE NATURAL)

    foreach (ndk IN LISTS cows_ndks)
        if (EXISTS "${ndk}/build/cmake/android.toolchain.cmake")
            set(cows_ndk "${ndk}")
        endif ()
    endforeach ()
endif ()

file(TO_CMAKE_PATH "${cows_ndk}" cows_ndk)
set(cows_ndk_toolchain "${cows_ndk}/build/cmake/android.toolchain.cmake")

if (NOT EXISTS "${cows_ndk_toolchain}")
    message(FATAL_ERROR "${cows_ndk_missing}: install one with Android Studio's SDK "
            "Manager, set ANDROID_HOME to an SDK that has one, or ANDROID_NDK_HOME to an NDK.")
endif ()

include("${cows_ndk_toolchain}")
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES COWS_NDK)

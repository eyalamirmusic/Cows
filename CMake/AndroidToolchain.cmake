# The NDK eacp builds with (eacp's CMake/AndroidVersions.cmake), from the SDK at
# $ANDROID_HOME, else the one eacp's android-setup.cmake makes, else Android
# Studio's.
set(cows_ndk_version 30.0.16248370)

set(cows_sdks "$ENV{ANDROID_HOME}" "$ENV{HOME}/.eacp/android/sdk"
        "$ENV{HOME}/Library/Android/sdk" "$ENV{LOCALAPPDATA}/Android/Sdk")

foreach (sdk IN LISTS cows_sdks)
    file(TO_CMAKE_PATH "${sdk}" sdk)
    set(toolchain "${sdk}/ndk/${cows_ndk_version}/build/cmake/android.toolchain.cmake")

    if (sdk AND EXISTS "${toolchain}")
        include("${toolchain}")
        return()
    endif ()
endforeach ()

message(FATAL_ERROR "No NDK ${cows_ndk_version} in $ANDROID_HOME, "
        "~/.eacp/android/sdk or ~/Library/Android/sdk: install it with Android "
        "Studio's SDK Manager, or eacp's cmake -P Scripts/android-setup.cmake.")

# The NDK eacp builds with (eacp's CMake/AndroidVersions.cmake), from the SDK at
# $ANDROID_HOME, else Android Studio's, which tools/android.sh uses too and
# which Android Studio otherwise asks to switch the project to, else the one
# eacp's android-setup.cmake makes.
set(cows_ndk_version 30.0.16248370)

set(cows_sdks "$ENV{ANDROID_HOME}" "$ENV{HOME}/Library/Android/sdk"
        "$ENV{LOCALAPPDATA}/Android/Sdk" "$ENV{HOME}/.eacp/android/sdk")

foreach (sdk IN LISTS cows_sdks)
    file(TO_CMAKE_PATH "${sdk}" sdk)
    set(toolchain "${sdk}/ndk/${cows_ndk_version}/build/cmake/android.toolchain.cmake")

    if (sdk AND EXISTS "${toolchain}")
        include("${toolchain}")
        return()
    endif ()
endforeach ()

message(FATAL_ERROR "No NDK ${cows_ndk_version} in $ANDROID_HOME, "
        "~/Library/Android/sdk or ~/.eacp/android/sdk: install it with Android "
        "Studio's SDK Manager, or eacp's cmake -P Scripts/android-setup.cmake.")

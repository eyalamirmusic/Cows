cmake_minimum_required(VERSION 3.31)

if (NOT CMAKE_OSX_DEPLOYMENT_TARGET)
    if (CMAKE_SYSTEM_NAME STREQUAL "iOS")
        set(CMAKE_OSX_DEPLOYMENT_TARGET "15.0" CACHE STRING "Minimum iOS version")
    elseif (CMAKE_HOST_APPLE AND NOT CMAKE_SYSTEM_NAME AND NOT CMAKE_TOOLCHAIN_FILE)
        set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version")
    endif ()
endif ()

# The fourth number is the build (versionCode, CFBundleVersion): raise it every upload.
project(Cows VERSION 1.0.2.14 LANGUAGES C CXX)

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

    # Play requires API 36; compileSdk matches so an older local SDK cannot slip in.
    set(COWS_ANDROID_TARGET_SDK 36 CACHE STRING "Android target/compile SDK")
    set(COWS_ANDROID_MIN_SDK 30 CACHE STRING "Oldest supported Android SDK")
    set(EACP_ANDROID_TARGET_SDK "${COWS_ANDROID_TARGET_SDK}" CACHE STRING
            "targetSdk selected by Cows" FORCE)
    set(EACP_ANDROID_COMPILE_SDK "${COWS_ANDROID_TARGET_SDK}" CACHE STRING
            "compileSdk selected by Cows" FORCE)
    set(EACP_ANDROID_MIN_SDK "${COWS_ANDROID_MIN_SDK}" CACHE STRING
            "minSdk selected by Cows" FORCE)
endif ()

find_package(eacp REQUIRED)

list(APPEND CMAKE_MODULE_PATH "${eacp_SOURCE_DIR}/CMake")
include(TargetSetup)

if (IOS)
    set(COWS_IOS_TEAM "" CACHE STRING "Apple team that signs the iOS app")
    set(CMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM "${COWS_IOS_TEAM}")
    set(CMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Apple Development" CACHE STRING "")
endif ()

option(COWS_BUILD_TESTS "Build the test suites" ON)
option(COWS_HOT_RELOAD_COW
        "Dev only: hot reload Cow.cpp. The app compiles it, so tests and tools/Art, which link the library without it, are not built" OFF)

if (COWS_HOT_RELOAD_COW)
    set(COWS_BUILD_TESTS OFF)
endif ()

# A seed must make the same level everywhere; FMA changes the low bits placement hinges on.
add_compile_options($<$<CXX_COMPILER_ID:Clang,AppleClang,GNU>:-ffp-contract=off>)

add_subdirectory(Lib)
add_subdirectory(Apps)

# Signing secrets come from the environment so they never land in CMake's cache.
if (ANDROID AND EACP_ANDROID_STUDIO)
    set(cows_gradle "${CMAKE_BINARY_DIR}/AndroidStudio/Cows/build.gradle.kts")
    if (EXISTS "${cows_gradle}")
        file(READ "${cows_gradle}" gradle)
        string(REGEX REPLACE
                "    compileSdk \\{\n        version = release\\(([0-9]+)\\) \\{\n            minorApiLevel = 0\n        \\}\n    \\}"
                "    compileSdk = \\1" fixed_gradle "${gradle}")

        set(cows_signing_preamble [=[
val cowsUploadKeystore = providers.environmentVariable("COWS_ANDROID_KEYSTORE")
val cowsUploadKeystorePassword = providers.environmentVariable("COWS_ANDROID_KEYSTORE_PASSWORD")
val cowsUploadKeyAlias = providers.environmentVariable("COWS_ANDROID_KEY_ALIAS")
val cowsUploadKeyPassword = providers.environmentVariable("COWS_ANDROID_KEY_PASSWORD")
val cowsUploadSigningValues = listOf(
    cowsUploadKeystore,
    cowsUploadKeystorePassword,
    cowsUploadKeyAlias,
    cowsUploadKeyPassword,
)
val cowsHasUploadSigning = cowsUploadSigningValues.all { it.isPresent }
val cowsHasAnyUploadSigning = cowsUploadSigningValues.any { it.isPresent }
check(!cowsHasAnyUploadSigning || cowsHasUploadSigning) {
    "Play signing requires all four COWS_ANDROID_* environment variables"
}
]=])
        string(REPLACE "android {\n"
                "${cows_signing_preamble}\nandroid {\n"
                fixed_gradle "${fixed_gradle}")

        set(cows_signing_config [=[
    signingConfigs {
        if (cowsHasUploadSigning) {
            create("upload") {
                storeFile = file(cowsUploadKeystore.get())
                storePassword = cowsUploadKeystorePassword.get()
                keyAlias = cowsUploadKeyAlias.get()
                keyPassword = cowsUploadKeyPassword.get()
            }
        }
    }

]=])
        string(REPLACE "    buildTypes {\n"
                "${cows_signing_config}    buildTypes {\n"
                fixed_gradle "${fixed_gradle}")
        string(REPLACE
                "            // The debug key, so a release build installs as is. A Play upload\n            // needs an upload key in a signingConfig of its own here.\n            signingConfig = signingConfigs.getByName(\"debug\")"
                "            // Upload builds are signed only while the release wrapper supplies credentials.\n            signingConfig = signingConfigs.getByName(if (cowsHasUploadSigning) \"upload\" else \"debug\")"
                fixed_gradle "${fixed_gradle}")

        string(FIND "${fixed_gradle}" "cowsHasUploadSigning" cows_signing_patch)
        if (cows_signing_patch EQUAL -1)
            message(FATAL_ERROR "Could not add Play signing to eacp's generated Gradle project")
        endif ()
        if (NOT fixed_gradle STREQUAL gradle)
            file(WRITE "${cows_gradle}" "${fixed_gradle}")
        endif ()
    endif ()
endif ()

if (APPLE AND NOT IOS AND NOT COWS_HOT_RELOAD_COW)
    add_subdirectory(tools/Art)
endif ()

if (COWS_BUILD_TESTS AND NOT IOS AND NOT ANDROID)
    enable_testing()
    add_subdirectory(Tests)
endif ()

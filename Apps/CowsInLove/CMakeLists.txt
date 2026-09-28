set(COWS_SOURCES
        Source/Main.cpp
        Source/CowsApp.cpp
        Source/Stages.cpp
        Source/Templates.cpp
        Source/Scene/CowsView.cpp)

# On Android the app is the library NativeActivity loads.
if (ANDROID)
    add_library(Cows SHARED ${COWS_SOURCES})
else ()
    add_executable(Cows ${COWS_SOURCES})
endif ()

target_include_directories(Cows PRIVATE Source)
target_link_libraries(Cows PRIVATE cowsinlove_Game)

set(COWS_BUNDLE_ID "com.cowsinlove.cows" CACHE STRING
        "Bundle identifier: set to the one registered on the publishing account")
set(COWS_COPYRIGHT "© 2026 Cows In Love" CACHE STRING
        "The copyright line shown in Finder and the About box")
set(COWS_BUILD_NUMBER "1" CACHE STRING
        "CFBundleVersion: raise it for every upload of the same version")

set_target_properties(Cows PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "Cows In Love"
        MACOSX_BUNDLE_GUI_IDENTIFIER ${COWS_BUNDLE_ID}
        MACOSX_BUNDLE_COPYRIGHT "${COWS_COPYRIGHT}"
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${COWS_BUNDLE_ID})

eacp_set_gui_subsystem(Cows)
set_default_target_setting(Cows)

set_target_properties(Cows PROPERTIES
        MACOSX_BUNDLE_BUNDLE_VERSION ${COWS_BUILD_NUMBER}
        XCODE_ATTRIBUTE_CURRENT_PROJECT_VERSION ${COWS_BUILD_NUMBER}
        XCODE_ATTRIBUTE_MARKETING_VERSION ${PROJECT_VERSION})

if (ANDROID)
    set(EACP_ANDROID_MANIFEST_TEMPLATE
            "${CMAKE_CURRENT_SOURCE_DIR}/Android/AndroidManifest.xml.in")
    eacp_add_android_apk(Cows
            PACKAGE ${COWS_BUNDLE_ID}
            LABEL "Cows In Love"
            ORIENTATION portrait
            VERSION_CODE ${COWS_BUILD_NUMBER}
            VERSION_NAME ${PROJECT_VERSION}
            RES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Android/res"
            ICON "@mipmap/ic_launcher")
elseif (IOS)
    set(assets "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Assets.xcassets")
    set(privacy "${CMAKE_CURRENT_SOURCE_DIR}/Resources/PrivacyInfo.xcprivacy")
    target_sources(Cows PRIVATE "${assets}" "${privacy}")
    set_source_files_properties("${assets}" "${privacy}" PROPERTIES
            MACOSX_PACKAGE_LOCATION Resources)
    set_target_properties(Cows PROPERTIES
            XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME AppIcon
            XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1")
else ()
    eacp_set_app_icon(Cows IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/Resources/AppIcon-Desktop.png")
endif ()

# An Xcode archive keeps only what installs: the app, never the libraries it
# links (tools/release-ios.sh and release-mas.sh configure SKIP_INSTALL=YES).
if (APPLE)
    set(COWS_MAC_ENTITLEMENTS "" CACHE FILEPATH
            "Entitlements for the Mac App Store build (tools/release-mas.sh)")
    set_target_properties(Cows PROPERTIES
            XCODE_ATTRIBUTE_SKIP_INSTALL NO
            XCODE_ATTRIBUTE_INSTALL_PATH "$(LOCAL_APPS_DIR)")

    if (COWS_MAC_ENTITLEMENTS)
        set_target_properties(Cows PROPERTIES
                XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${COWS_MAC_ENTITLEMENTS}")
    endif ()
endif ()

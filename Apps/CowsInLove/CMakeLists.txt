set(COWS_BUNDLE_ID "com.cowsinlove.play" CACHE STRING
        "Bundle identifier: set to the one registered on the publishing account")
set(COWS_COPYRIGHT "© 2026 Cows In Love" CACHE STRING
        "The copyright line shown in Finder and the About box")
set(COWS_MAC_ENTITLEMENTS "" CACHE FILEPATH
        "Entitlements for the Mac App Store build (tools/release-mas.sh)")

set(COWS_DISPLAY_NAME "Cows In Love")
set(COWS_VERSION
        "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}")

eacp_add_app(Cows
        Source/Main.cpp
        Source/Reloadable.cpp
        Source/CowsApp.cpp
        Source/Settings.cpp
        Source/Stages.cpp
        Source/Templates.cpp
        Source/Scene/CowsView.cpp
        Source/Scene/FrameRate.cpp
        Source/Scene/Quality.cpp
        BUNDLE_ID ${COWS_BUNDLE_ID}
        DISPLAY_NAME "${COWS_DISPLAY_NAME}"
        VERSION ${COWS_VERSION}
        VERSION_CODE ${PROJECT_VERSION_TWEAK}
        ICON Resources/AppIcon-Desktop.png
        ORIENTATION portrait)

# Android/res and the store manifest entries wait for eacp to take an app's own again.

eacp_add_plist_entries(Cows
        CFBundleDisplayName "${COWS_DISPLAY_NAME}"
        ITSAppUsesNonExemptEncryption FALSE
        LSApplicationCategoryType public.app-category.games)

if (IOS)
    eacp_add_plist_entries(Cows
            LSRequiresIPhoneOS TRUE
            UIRequiresFullScreen TRUE
            UIStatusBarHidden TRUE)

    set(ios_resources Resources/Assets.xcassets Resources/PrivacyInfo.xcprivacy)
    target_sources(Cows PRIVATE ${ios_resources})
    set_source_files_properties(${ios_resources} PROPERTIES
            MACOSX_PACKAGE_LOCATION Resources)
    set_target_properties(Cows PROPERTIES
            XCODE_ATTRIBUTE_ASSETCATALOG_COMPILER_APPICON_NAME AppIcon)
endif ()

target_include_directories(Cows PRIVATE Source)
target_compile_definitions(Cows PRIVATE
        COWS_APP_VERSION="${COWS_VERSION}"
        COWS_APP_BUILD="${PROJECT_VERSION_TWEAK}")
target_link_libraries(Cows PRIVATE cowsinlove_Game)

# An Xcode archive keeps only the app, never the libraries it links.
set_target_properties(Cows PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_COPYRIGHT "${COWS_COPYRIGHT}"
        XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1"
        XCODE_ATTRIBUTE_SKIP_INSTALL NO
        XCODE_ATTRIBUTE_INSTALL_PATH "$(LOCAL_APPS_DIR)"
        XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${COWS_MAC_ENTITLEMENTS}")

# Finder and Launchpad show a Mac app's file name.
if (APPLE AND NOT IOS)
    set_target_properties(Cows PROPERTIES OUTPUT_NAME "${COWS_DISPLAY_NAME}")
endif ()

set(cow_model_directory ${CMAKE_SOURCE_DIR}/Lib/cowsinlove_Actors/Cow)
target_compile_definitions(Cows PRIVATE COWS_COW_MODEL_SOURCE_DIRECTORY="${cow_model_directory}")

if (COWS_HOT_RELOAD_COW)
    eacp_add_hot_reload(Cows MODULE CowsCowModel
            SOURCES ${cow_model_directory}/Cow.cpp HOT_RELOAD ON)
endif ()

eacp_set_gui_subsystem(Cows)
set_default_target_setting(Cows)

set(COWS_BUNDLE_ID "com.cowsinlove.cows" CACHE STRING
        "Bundle identifier: set to the one registered on the publishing account")
set(COWS_COPYRIGHT "© 2026 Cows In Love" CACHE STRING
        "The copyright line shown in Finder and the About box")
set(COWS_BUILD_NUMBER "1" CACHE STRING
        "Build number (CFBundleVersion, Play versionCode): raise it for every upload")
set(COWS_MAC_ENTITLEMENTS "" CACHE FILEPATH
        "Entitlements for the Mac App Store build (tools/release-mas.sh)")

set(COWS_DISPLAY_NAME "Cows In Love")

eacp_add_app(Cows
        Source/Main.cpp
        Source/CowsApp.cpp
        Source/Stages.cpp
        Source/Templates.cpp
        Source/Scene/CowsView.cpp
        BUNDLE_ID ${COWS_BUNDLE_ID}
        DISPLAY_NAME "${COWS_DISPLAY_NAME}"
        VERSION_CODE ${COWS_BUILD_NUMBER}
        ICON Resources/AppIcon-Desktop.png
        ORIENTATION portrait)

# Android/res (the adaptive icon) and the store manifest entries (touchscreen
# and portrait required, isGame, appCategory, no backup) wait for eacp to take
# an app's own res directory and manifest elements again.

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
target_link_libraries(Cows PRIVATE cowsinlove_Game)

# An Xcode archive keeps only what installs: the app, never the libraries it
# links (tools/release-ios.sh and release-mas.sh configure SKIP_INSTALL=YES).
set_target_properties(Cows PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_COPYRIGHT "${COWS_COPYRIGHT}"
        XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1"
        XCODE_ATTRIBUTE_SKIP_INSTALL NO
        XCODE_ATTRIBUTE_INSTALL_PATH "$(LOCAL_APPS_DIR)"
        XCODE_ATTRIBUTE_CODE_SIGN_ENTITLEMENTS "${COWS_MAC_ENTITLEMENTS}")

eacp_set_gui_subsystem(Cows)
set_default_target_setting(Cows)

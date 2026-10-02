set(COWS_BUNDLE_ID "com.cowsinlove.cows" CACHE STRING
        "Bundle identifier: set to the one registered on the publishing account")
set(COWS_COPYRIGHT "© 2026 Cows In Love" CACHE STRING
        "The copyright line shown in Finder and the About box")
set(COWS_BUILD_NUMBER "1" CACHE STRING
        "Build number (CFBundleVersion, Play versionCode): raise it for every upload")
set(COWS_MAC_ENTITLEMENTS "" CACHE FILEPATH
        "Entitlements for the Mac App Store build (tools/release-mas.sh)")

eacp_add_app(Cows
        Source/Main.cpp
        Source/CowsApp.cpp
        Source/Stages.cpp
        Source/Templates.cpp
        Source/Scene/CowsView.cpp
        BUNDLE_ID ${COWS_BUNDLE_ID}
        DISPLAY_NAME "Cows In Love"
        VERSION_CODE ${COWS_BUILD_NUMBER}
        ICON Resources/AppIcon-Desktop.png
        IOS_RESOURCES Resources/Assets.xcassets Resources/PrivacyInfo.xcprivacy
        ORIENTATION portrait
        RES_DIR Android/res
        MANIFEST_ELEMENTS [[
    <uses-feature android:name="android.hardware.touchscreen" android:required="true" />
    <uses-feature android:name="android.hardware.screen.portrait" android:required="true" />
    <supports-screens android:smallScreens="true" android:normalScreens="true"
            android:largeScreens="true" android:xlargeScreens="true" />]]
        APPLICATION_ATTRIBUTES [[android:isGame="true" android:appCategory="game"
            android:allowBackup="false" android:hasFragileUserData="false"]])

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

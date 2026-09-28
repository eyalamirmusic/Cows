add_executable(Cows
        Source/Main.cpp
        Source/CowsApp.cpp
        Source/Stages.cpp
        Source/Scene/CowsView.cpp)

target_include_directories(Cows PRIVATE Source)
target_link_libraries(Cows PRIVATE cowsinlove_Game)

set(BUNDLE_ID "com.tamber.cows")

set_target_properties(Cows PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "Cows"
        MACOSX_BUNDLE_GUI_IDENTIFIER ${BUNDLE_ID}
        XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER ${BUNDLE_ID})

eacp_set_gui_subsystem(Cows)
set_default_target_setting(Cows)

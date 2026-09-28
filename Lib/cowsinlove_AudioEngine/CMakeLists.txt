add_library(cowsinlove_AudioEngine STATIC SamplePlayer.cpp)

target_include_directories(cowsinlove_AudioEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(cowsinlove_AudioEngine PUBLIC eacp-core)

if (APPLE)
    target_link_libraries(cowsinlove_AudioEngine PUBLIC "-framework AudioToolbox")
endif ()

set_default_target_setting(cowsinlove_AudioEngine)

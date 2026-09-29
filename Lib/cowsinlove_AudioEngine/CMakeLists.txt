find_package(MakeASound REQUIRED)

# See CMake/NoALSA/FindALSA.cmake: no MIDI on Android or the web, so RtMidi's
# dummy API.
if (ANDROID OR EMSCRIPTEN)
    set_property(TARGET rtmidi PROPERTY COMPILE_DEFINITIONS __RTMIDI_DUMMY__)
endif ()

add_library(cowsinlove_AudioEngine STATIC SamplePlayer.cpp)

target_include_directories(cowsinlove_AudioEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(cowsinlove_AudioEngine PUBLIC eacp-core MakeASound)

set_default_target_setting(cowsinlove_AudioEngine)

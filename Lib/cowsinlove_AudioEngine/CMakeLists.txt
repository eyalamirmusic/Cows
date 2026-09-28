find_package(MakeASound REQUIRED)

add_library(cowsinlove_AudioEngine STATIC SamplePlayer.cpp)

target_include_directories(cowsinlove_AudioEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(cowsinlove_AudioEngine PUBLIC eacp-core MakeASound)

set_default_target_setting(cowsinlove_AudioEngine)

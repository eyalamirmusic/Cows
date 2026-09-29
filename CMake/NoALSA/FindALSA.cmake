# Android and Emscripten have no ALSA, but MakeASound's RtMidi find module asks
# for it on every UNIX. This answers with an empty target, and
# cowsinlove_AudioEngine takes RtMidi's ALSA backend back out (RtMidi then
# builds its dummy API). The real fix is an Android and Emscripten branch in
# MakeASound's CMake/FindRTMidi.cmake.
if (NOT TARGET ALSA::ALSA)
    add_library(ALSA::ALSA INTERFACE IMPORTED)
endif ()

set(ALSA_FOUND TRUE)

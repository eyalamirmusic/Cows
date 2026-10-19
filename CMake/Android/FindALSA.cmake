# Workaround: MakeASound's RtMidi asks for ALSA on every UNIX, Android included.
if (NOT TARGET ALSA::ALSA)
    add_library(ALSA::ALSA INTERFACE IMPORTED)
endif ()

set(ALSA_FOUND TRUE)

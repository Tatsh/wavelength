set(WAVELENGTH_SYNTH_SRCS
    bankfileio.cpp
    callbackxferbdtoiop.cpp
    fxmidi.cpp
    lazyfilestream.cpp
    lfo.cpp
    sfxbuilder.cpp
    songspeed.cpp
    streamplayer.cpp
    synth.cpp
    syntheffects.cpp
    synthps2.cpp)

add_library(wavelength_synth STATIC ${WAVELENGTH_SYNTH_SRCS})
target_link_libraries(wavelength_synth PRIVATE wavelength_common)

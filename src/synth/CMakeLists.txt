set(WAVELENGTH_SYNTH_SRCS
    callbackxferbdtoiop.cpp
    songspeed.cpp
    synth.cpp)

add_library(wavelength_synth STATIC ${WAVELENGTH_SYNTH_SRCS})
target_link_libraries(wavelength_synth PRIVATE wavelength_common)

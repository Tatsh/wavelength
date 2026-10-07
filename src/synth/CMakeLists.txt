set(WAVELENGTH_SYNTH_SRCS
    callbackxferbdtoiop.cpp
    callbackxferhdtoiop.cpp
    songspeed.cpp
    source.cpp
    synth.cpp
    synthsustainer.cpp)

add_library(wavelength_synth STATIC ${WAVELENGTH_SYNTH_SRCS})
target_link_libraries(wavelength_synth PRIVATE wavelength_common)

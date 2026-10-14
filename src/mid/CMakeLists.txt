set(WAVELENGTH_MID_SRCS
    mbt.cpp
    midivalidator.cpp
    midireader.cpp
    trackbuilder.cpp)

add_library(wavelength_mid STATIC ${WAVELENGTH_MID_SRCS})
target_link_libraries(wavelength_mid PRIVATE wavelength_common)

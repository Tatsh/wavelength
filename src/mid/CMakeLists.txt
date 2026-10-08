set(WAVELENGTH_MID_SRCS
    mbt.cpp
    midireader.cpp
    tick.cpp
    trackbuilder.cpp)

add_library(wavelength_mid STATIC ${WAVELENGTH_MID_SRCS})
target_link_libraries(wavelength_mid PRIVATE wavelength_common)

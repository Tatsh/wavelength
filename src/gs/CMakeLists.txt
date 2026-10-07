set(WAVELENGTH_GS_SRCS multimuse.cpp)

add_library(wavelength_gs STATIC ${WAVELENGTH_GS_SRCS})
target_link_libraries(wavelength_gs PRIVATE wavelength_common)

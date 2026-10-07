set(WAVELENGTH_PROFILE_SRCS
    profiler.cpp)

add_library(wavelength_profile STATIC ${WAVELENGTH_PROFILE_SRCS})
target_link_libraries(wavelength_profile PRIVATE wavelength_common)

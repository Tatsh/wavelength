set(WAVELENGTH_SCRIPT_SRCS cmdpostscript.cpp scripttemplatemap.cpp)

add_library(wavelength_script STATIC ${WAVELENGTH_SCRIPT_SRCS})
target_link_libraries(wavelength_script PRIVATE wavelength_common)

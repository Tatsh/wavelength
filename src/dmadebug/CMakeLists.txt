set(WAVELENGTH_DMADEBUG_SRCS debug.c dma.c gif.c vif.c)

add_library(wavelength_dmadebug STATIC ${WAVELENGTH_DMADEBUG_SRCS})
target_link_libraries(wavelength_dmadebug PRIVATE wavelength_common)

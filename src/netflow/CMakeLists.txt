set(WAVELENGTH_NETFLOW_SRCS netflow.c)

add_library(wavelength_netflow STATIC ${WAVELENGTH_NETFLOW_SRCS})
target_link_libraries(wavelength_netflow PRIVATE wavelength_common)

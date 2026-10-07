set(WAVELENGTH_SCH_SRCS
    cmdid.cpp
    command.cpp
    tempomap.cpp
    tickclock.cpp
    time.cpp
    timedcommand.cpp)

add_library(wavelength_sch STATIC ${WAVELENGTH_SCH_SRCS})
target_link_libraries(wavelength_sch PRIVATE wavelength_common)

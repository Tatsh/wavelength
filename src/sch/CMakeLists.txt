set(WAVELENGTH_SCH_SRCS
    barsequencer.cpp
    cmdid.cpp
    command.cpp
    sequencercmd.cpp
    tempomap.cpp
    tickclock.cpp
    time.cpp
    timedcommand.cpp)

add_library(wavelength_sch STATIC ${WAVELENGTH_SCH_SRCS})
target_link_libraries(wavelength_sch PRIVATE wavelength_common)

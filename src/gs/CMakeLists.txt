set(WAVELENGTH_GS_SRCS
    axecontour.cpp
    axeharmony.cpp
    axer.cpp
    midichannelstate.cpp
    multimuse.cpp
    musefactory.cpp
    muselooper.cpp
    mutablenotemuse.cpp
    notemuse.cpp
    notemusebase.cpp
    scratcher.cpp
    stdmidimuse.cpp)

add_library(wavelength_gs STATIC ${WAVELENGTH_GS_SRCS})
target_link_libraries(wavelength_gs PRIVATE wavelength_common)

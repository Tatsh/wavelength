set(WAVELENGTH_GS_SRCS
    effector.cpp
    ghostnoteseffector.cpp
    midionoffeffector.cpp
    multimuse.cpp
    multimuseplayer.cpp
    museplayer.cpp
    musesynth.cpp
    noteplayer.cpp
    stuttereffector.cpp
    volumeeffector.cpp
    waheffector.cpp)

add_library(wavelength_gs STATIC ${WAVELENGTH_GS_SRCS})
target_link_libraries(wavelength_gs PRIVATE wavelength_common)

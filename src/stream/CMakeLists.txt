set(WAVELENGTH_STREAM_SRCS
    hxchunkheader.cpp
    hxchunkname.cpp
    hxidatachunk.cpp
    hxilistchunk.cpp
    hxmemstream.cpp
    hxstream.cpp
    hxvarlennumber.cpp
    ibfilestream.cpp
    ibstream.cpp
    iobmemstream.cpp
    iobpreallocmemstream.cpp
    obfilestream.cpp
    obstream.cpp)

add_library(wavelength_stream STATIC ${WAVELENGTH_STREAM_SRCS})
target_link_libraries(wavelength_stream PRIVATE wavelength_common)

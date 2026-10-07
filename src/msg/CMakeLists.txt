set(WAVELENGTH_MSG_SRCS
    arbiterpacket.cpp
    bumperpacket.cpp
    capturepacket.cpp
    editgempacket.cpp
    finalscorepacket.cpp
    fxpacket.cpp
    lobbyconnectionlostmsg.cpp
    message.cpp
    mutevocalspacket.cpp
    packetfactories.cpp
    playerupdatepacket.cpp
    previewpacket.cpp
    remixerupdatepacket.cpp
    sectionchangepacket.cpp)

add_library(wavelength_msg STATIC ${WAVELENGTH_MSG_SRCS})
target_link_libraries(wavelength_msg PRIVATE wavelength_common)

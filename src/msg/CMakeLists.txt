set(WAVELENGTH_MSG_SRCS
    arbiterpacket.cpp
    chatmsg.cpp
    bumperpacket.cpp
    capturepacket.cpp
    chatmsg.cpp
    cripplerpacket.cpp
    editgempacket.cpp
    erasebarpacket.cpp
    erasesectionpacket.cpp
    finalscorepacket.cpp
    freestylebumppacket.cpp
    freestylepacket.cpp
    fxpacket.cpp
    joypadanalogstickmsg.cpp
    joypadconnectionmsg.cpp
    joypadinputmsg.cpp
    keyboardkeymsg.cpp
    lobbyconnectionlostmsg.cpp
    message.cpp
    messagefactory.cpp
    mutevocalspacket.cpp
    packetfactories.cpp
    playerupdatepacket.cpp
    previewpacket.cpp
    remixerupdatepacket.cpp
    sectionchangepacket.cpp)

add_library(wavelength_msg STATIC ${WAVELENGTH_MSG_SRCS})
target_link_libraries(wavelength_msg PRIVATE wavelength_common)
# ChatMsg::Send() copies the sender's name into the request with strncpy() to the full size of the
# field, as retail does.
set_source_files_properties(chatmsg.cpp PROPERTIES COMPILE_OPTIONS -Wno-stringop-truncation)

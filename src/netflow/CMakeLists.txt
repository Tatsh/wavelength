set(WAVELENGTH_NETFLOW_SRCS
    accountdeletetask.cpp
    accountgetidtask.cpp
    accountlogintask.cpp
    accountregistrationtask.cpp
    accountupdatepasswordtask.cpp
    accountupdatestatstask.cpp
    allocatebuffertask.cpp
    anonymouslogintask.cpp
    banplayertask.cpp
    channelinfotask.cpp
    checkinettask.cpp
    cleargamelistfiltertask.cpp
    clocksynctask.cpp
    closemgcltask.cpp
    closerttask.cpp
    createchanneltask.cpp
    dnastask.cpp
    dodnstask.cpp
    fileclosetask.cpp
    filecreatetask.cpp
    filedeletetask.cpp
    filedownloadtask.cpp
    filegetattributestask.cpp
    filelisttask.cpp
    fileuploadtask.cpp
    findplayertask.cpp
    finishupdategameparamstask.cpp
    getannouncementstask.cpp
    getchannelstask.cpp
    getgameplayerstask.cpp
    getgamestask.cpp
    getlobbyplayerstask.cpp
    getpolicytask.cpp
    hostpadrt.cpp
    inetimpl.cpp
    initmgcltask.cpp
    initrttask.cpp
    joinchanneltask.cpp
    joinpadrt.cpp
    killlaunchpadtask.cpp
    ladderlisttask.cpp
    ladderposition2task.cpp
    ladderpositionfasttask.cpp
    ladderpositiontask.cpp
    launchpadrt.cpp
    loadgametask.cpp
    lobbyrt.cpp
    mediusconnecttask.cpp
    mediusdisconnecttask.cpp
    mediusjoingametask.cpp
    mediussendplayerreporttask.cpp
    mediussendworldreporttask.cpp
    mgclauthenticationtask.cpp
    mgclconnecttask.cpp
    mgclcreategametask.cpp
    mgcldisconnecttask.cpp
    mgclsendserverreporttask.cpp
    mgclsessionbegintask.cpp
    mgclsessionendtask.cpp
    net.cpp
    netconnecttask.cpp
    netcrypt.cpp
    netdisconnecttask.cpp
    netgameparams.cpp
    nethosttask.cpp
    netmessages.cpp
    nettask.cpp
    playerinfotask.cpp
    ratingstat.cpp
    sessionbegintask.cpp
    sessionendtask.cpp
    setgamelistfiltertask.cpp
    setuplocalplayertask.cpp
    textfiltertask.cpp
    transportrt.cpp
    updateclientstatetask.cpp
    versionservertask.cpp
    waitforkeytask.cpp
    waitforsharedonetask.cpp
    waitforwelcometask.cpp)

add_library(wavelength_netflow STATIC ${WAVELENGTH_NETFLOW_SRCS})
target_link_libraries(wavelength_netflow PRIVATE wavelength_common)
# The requests of the network library take their text fields with strncpy() to the full size of
# each field, as retail does.
target_compile_options(wavelength_netflow PRIVATE -Wno-stringop-truncation)

# The cipher comes from the vendored LibTomCrypt, built with Blowfish and CBC mode only.
set(_tomcrypt "${CMAKE_SOURCE_DIR}/3rdparty/libtomcrypt-0.75")
add_library(wavelength_tomcrypt STATIC "${_tomcrypt}/blowfish.c" "${_tomcrypt}/cbc.c"
                                       "${_tomcrypt}/crypt.c" "${_tomcrypt}/mem.c")
target_link_libraries(wavelength_tomcrypt PRIVATE wavelength_common)
# The game's build settings string lists MD5, BASE64, HMAC, SMALL_PRIME_TAB, and little-endian
# 64-bit words beside Blowfish and CBC. The compiler does not define the R5900 macros upstream
# detects. The game has no argument checks (ARGTYPE 2).
target_compile_definitions(
  wavelength_tomcrypt
  PRIVATE ARGTYPE=2
          BASE64
          BLOWFISH
          CBC
          ENDIAN_64BITWORD
          ENDIAN_LITTLE
          HMAC
          MD5
          SMALL_PRIME_TAB)
target_include_directories(wavelength_tomcrypt PRIVATE "${_tomcrypt}")
# These warnings concern upstream style.
target_compile_options(wavelength_tomcrypt PRIVATE -Wno-unused-parameter)
set_source_files_properties(
  netcrypt.cpp PROPERTIES INCLUDE_DIRECTORIES "${_tomcrypt}"
                          COMPILE_DEFINITIONS "BLOWFISH;CBC")

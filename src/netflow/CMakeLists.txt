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
    netflow.c
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
set_source_files_properties(
  netcrypt.cpp PROPERTIES INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/3rdparty/libtomcrypt-0.75"
                          COMPILE_DEFINITIONS "BLOWFISH;CBC")

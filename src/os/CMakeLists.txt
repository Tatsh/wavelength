set(WAVELENGTH_OS_SRCS
    acircbuff.cpp
    archive.cpp
    arkfile.cpp
    arkhash.cpp
    assert.cpp
    asynccallback.cpp
    asyncfile.cpp
    asyncstream.cpp
    asynctask.cpp
    backtrace.cpp
    binstream.cpp
    blockmgr.cpp
    breugpad.cpp
    bufstream.cpp
    cheatsmanager.cpp
    chunkstream.cpp
    commandid.cpp
    cycles.cpp
    datetime.cpp
    dbg.cpp
    debug.cpp
    devconsole.cpp
    file.cpp
    filelog.cpp
    fileutil.cpp
    formatmessage.cpp
    genpath.cpp
    hostmode.cpp
    hxstr.cpp
    inflate.c
    iop.cpp
    irx.cpp
    joypad.cpp
    keyboard.cpp
    loadfile.cpp
    log.cpp
    mem.cpp
    openarkobject.cpp
    ramp.cpp
    random.cpp
    scheduler.cpp
    schedulerclock.cpp
    seccache.cpp
    spewregister.cpp
    spewtable.cpp
    string.cpp
    system.cpp
    timer.cpp
    zone.cpp)

add_library(wavelength_os STATIC ${WAVELENGTH_OS_SRCS})
target_link_libraries(wavelength_os PRIVATE wavelength_common)

# inflate.c includes the vendored gzip 1.2.4 inflate.c. The sign-compare warning concerns upstream
# style.
set_source_files_properties(inflate.c PROPERTIES INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/3rdparty"
                                                 COMPILE_OPTIONS -Wno-sign-compare)

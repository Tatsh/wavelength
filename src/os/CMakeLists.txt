set(WAVELENGTH_OS_SRCS
    allocinfo.cpp
    archive.cpp
    arkfile.cpp
    arkhash.cpp
    asyncfile.cpp
    asyncstream.cpp
    asynctask.cpp
    backtrace.cpp
    base64.cpp
    binstream.cpp
    bitstream.cpp
    blockmgr.cpp
    blockpool.cpp
    booloption.cpp
    blockstattable.cpp
    breugpad.cpp
    bufstream.cpp
    cheatsmanager.cpp
    chunkstream.cpp
    command.cpp
    commandid.cpp
    cycles.cpp
    datetime.cpp
    debug.cpp
    file.cpp
    filepath.cpp
    filestream.cpp
    filestreambuffer.cpp
    fileutil.cpp
    fixedsizealloc.cpp
    freestore.cpp
    heap.cpp
    heaptracker.cpp
    inflate.c
    irx.cpp
    joypad.cpp
    keyboard.cpp
    locale.cpp
    memhandle.cpp
    memmgr.cpp
    memstream.cpp
    memtrack.cpp
    namedobjectregistry.cpp
    option.cpp
    optionprocessor.cpp
    prnstream.cpp
    ramp.cpp
    scheduler.cpp
    schedulerclock.cpp
    serialtasks.cpp
    streambuffer.cpp
    stringoption.cpp
    string.cpp
    system.cpp
    task.cpp
    timer.cpp)

# The modules of the vendored gzip 1.2.4 the game links beside inflate.c, edited to match the game.
set(WAVELENGTH_GZIP_SRCS
    ${CMAKE_SOURCE_DIR}/3rdparty/gzip-1.2.4/bits.c
    ${CMAKE_SOURCE_DIR}/3rdparty/gzip-1.2.4/deflate.c
    ${CMAKE_SOURCE_DIR}/3rdparty/gzip-1.2.4/gzip.c
    ${CMAKE_SOURCE_DIR}/3rdparty/gzip-1.2.4/trees.c
    ${CMAKE_SOURCE_DIR}/3rdparty/gzip-1.2.4/unzip.c
    ${CMAKE_SOURCE_DIR}/3rdparty/gzip-1.2.4/util.c
    ${CMAKE_SOURCE_DIR}/3rdparty/gzip-1.2.4/zip.c)

add_library(wavelength_os STATIC ${WAVELENGTH_OS_SRCS} ${WAVELENGTH_GZIP_SRCS})
target_link_libraries(wavelength_os PRIVATE wavelength_common)

# inflate.c includes the vendored gzip 1.2.4 inflate.c. The sign-compare warning concerns upstream
# style.
set_source_files_properties(inflate.c PROPERTIES INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/3rdparty"
                                                 COMPILE_OPTIONS -Wno-sign-compare)

# The warnings concern upstream style.
set_source_files_properties(${WAVELENGTH_GZIP_SRCS} PROPERTIES COMPILE_OPTIONS
                                                          "-Wno-sign-compare;-Wno-unused-parameter")

set(WAVELENGTH_EZMPEG_SRCS sample/audiodec.c sample/ipudma.c sample/ldimage.c)

add_library(wavelength_ezmpeg STATIC ${WAVELENGTH_EZMPEG_SRCS})
target_link_libraries(wavelength_ezmpeg PRIVATE wavelength_common)
target_include_directories(wavelength_ezmpeg PRIVATE "${CMAKE_SOURCE_DIR}/sce/ee/include")

# The clients of the eznetcnf and eznetctl servers from Sony's network samples.
set(WAVELENGTH_EZNET_SRCS sample/libezctl.c sample/libeznetcnf.c)

add_library(wavelength_eznet STATIC ${WAVELENGTH_EZNET_SRCS})
target_link_libraries(wavelength_eznet PRIVATE wavelength_common)
target_include_directories(wavelength_eznet PUBLIC "${CMAKE_SOURCE_DIR}/sce/ee/include")

# The Sony library reconstructions. Each unit lists its sources explicitly, as the subsystem
# libraries do. libdma is absent until its routines are recovered.
set(WAVELENGTH_SONYLIB_SRCS
    src/alarm.c
    src/exit.c
    src/fileio.c
    src/initsys.c
    src/iopcontrol.c
    src/iopheap.c
    src/ipuinit.c
    src/libcdvd.c
    src/libdev.c
    src/libdma.c
    src/libgifpk.c
    src/libgraph.c
    src/libipu.c
    src/libkernel.c
    src/libkernel_alarm.s
    src/libkernel_exec.s
    src/libkernel_syscalls.s
    src/libkernel_tlb.s
    src/libmc.c
    src/libmpeg.c
    src/libmpeg_mc.s
    src/libmtap.c
    src/libosd.c
    src/libpad.c
    src/libscf.c
    src/libsdr.c
    src/libusbkb.c
    src/libvifpk.c
    src/libvu0.c
    src/loadfile.c
    src/msifrpc.c
    src/msin.c
    src/sifcmd.c
    src/sifdma.c
    src/sifrpc.c
    src/tlbfunc.c
    src/tty.c)

# The SCE-RT network library.
set(WAVELENGTH_RT_SRCS
    src/rt/libnet.c
    src/rt/mgcl.c
    src/rt/rtchannel.c
    src/rt/rtchannelapi.c
    src/rt/rtclock.c
    src/rt/rtcodec.c
    src/rt/rtdmeapi.c
    src/rt/rtdmedevice.c
    src/rt/rtdmemain.c
    src/rt/rtdmemessage.c
    src/rt/rtdmeobjects.c
    src/rt/rtdmevoice.c
    src/rt/rtgsm.c
    src/rt/rtimage.c
    src/rt/rtinet.c
    src/rt/rtlargeint.c
    src/rt/rtlargeintcore.c
    src/rt/rtlpc10.c
    src/rt/rtmedius.c
    src/rt/rtmediushandlers.c
    src/rt/rtmemory.c
    src/rt/rtmutex.c
    src/rt/rtpitch.c
    src/rt/rtrandom.c
    src/rt/rtrc4.c
    src/rt/rtreassembler.c
    src/rt/rtrecord.c
    src/rt/rtringbuffer.c
    src/rt/rtrsa.c
    src/rt/rtsecurity.c
    src/rt/rtsha1.c
    src/rt/rtsocket.c
    src/rt/rtstats.c
    src/rt/rtstream.c
    src/rt/rttime.c
    src/rt/rttitle.c
    src/rt/rtupnp.c
    src/rt/rtupnpxml.c
    src/rt/rtvlc.c)

add_library(wavelength_sonylibs STATIC ${WAVELENGTH_SONYLIB_SRCS} ${WAVELENGTH_RT_SRCS})
target_link_libraries(wavelength_sonylibs PRIVATE wavelength_common)
# The original libraries assert in the shipped build. No build type may compile the assertions out.
target_compile_options(wavelength_sonylibs PRIVATE -UNDEBUG)
# The game server client copies its fixed texts with the full size of their fields, as retail does.
set_source_files_properties(src/rt/mgcl.c PROPERTIES COMPILE_OPTIONS -Wno-stringop-truncation)
# The lobby client copies the texts of the connection with the length of the source, as retail does.
set_source_files_properties(src/rt/rtmedius.c PROPERTIES COMPILE_OPTIONS -Wno-stringop-truncation)
# Most receivers of the lobby messages only report the size of the record.
set_source_files_properties(src/rt/rtmediushandlers.c PROPERTIES COMPILE_OPTIONS -Wno-unused-parameter)
# The device copies the address of the router with the full size of its field, as retail does.
set_source_files_properties(src/rt/rtdmedevice.c PROPERTIES COMPILE_OPTIONS -Wno-stringop-truncation)
target_include_directories(wavelength_sonylibs PRIVATE "${CMAKE_SOURCE_DIR}/sce/ee/include"
                                                      "${CMAKE_SOURCE_DIR}/sce/ee/src"
                                                      "${CMAKE_SOURCE_DIR}/3rdparty/expat/xmlparse")

# The system layer the compiler's C and C++ runtime needs, over the reconstructed kernel.
add_library(wavelength_runtime STATIC runtime/newlib.c runtime/pthread.c runtime/softfloat.c)
target_link_libraries(wavelength_runtime PRIVATE wavelength_common)

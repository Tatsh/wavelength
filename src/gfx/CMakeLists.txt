set(WAVELENGTH_GFX_SRCS
    beam.cpp
    beamparams.cpp
    camslide.cpp
    constructo.cpp
    freestylus.cpp
    gemerasefx.cpp
    gfxarena.cpp
    gfxconfig.cpp
    gfxmanager.cpp
    gfxtunnel.cpp
    gfxtunnelpanel.cpp
    gfxutil.cpp
    gsdoublebuffer.cpp
    meshgroup.cpp
    meshrotation.cpp
    particlearm.cpp
    particlearms.cpp
    particlearmsassembly.cpp
    playercamfx.cpp
    polynomial.cpp
    quadraticapproach.cpp
    ship.cpp
    shipflyin.cpp
    spotlightfx.cpp
    spritegroup.cpp
    substepparticles.cpp
    tnlactivator.cpp
    tnlblurs.cpp
    tnlcatcher.cpp
    tnlconnectorbeam.cpp
    tnlcripfx.cpp
    tnlcursor.cpp
    tnldivider.cpp
    tnlfire.cpp
    tnlflashfx.cpp
    tnlfreestyle.cpp
    tnlgem.cpp
    tnlgemcapfx.cpp
    tnlgems.cpp
    tnlgeom.cpp
    tnlgrid.cpp
    tnlpath.cpp
    tnlplayer.cpp
    tnlsabretrail.cpp
    tnlseeker.cpp
    tnlslowdownfx.cpp
    tnlstreakgemfx.cpp
    tnltrackfx.cpp
    tnltrackrange.cpp
    tnlwave.cpp
    tnlwinshell.cpp
    vortex.cpp)

# The VU microprograms and the DMA chains that upload them. The DVP assembler builds them. It ships
# with the SDK beside the Emotion Engine compiler.
find_program(DVP_AS dvp-as REQUIRED HINTS "$ENV{PS2DEV}/dvp/bin")
set(WAVELENGTH_GFX_VU_SRCS vu0microcode.vsm vu1microcode.vsm)
set(WAVELENGTH_GFX_VU_OBJS)
foreach(VSM ${WAVELENGTH_GFX_VU_SRCS})
  get_filename_component(VSM_NAME ${VSM} NAME_WE)
  set(VSM_OBJ "${CMAKE_CURRENT_BINARY_DIR}/${VSM_NAME}.vsm.o")
  add_custom_command(
    OUTPUT ${VSM_OBJ}
    COMMAND ${DVP_AS} -o ${VSM_OBJ} ${CMAKE_CURRENT_SOURCE_DIR}/${VSM}
    DEPENDS ${VSM}
    COMMENT "Assembling ${VSM}"
    VERBATIM)
  list(APPEND WAVELENGTH_GFX_VU_OBJS ${VSM_OBJ})
endforeach()
set_source_files_properties(${WAVELENGTH_GFX_VU_OBJS} PROPERTIES EXTERNAL_OBJECT TRUE GENERATED TRUE)

add_library(wavelength_gfx STATIC ${WAVELENGTH_GFX_SRCS} ${WAVELENGTH_GFX_VU_OBJS})
target_link_libraries(wavelength_gfx PRIVATE wavelength_common)

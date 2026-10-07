# Defines the image target, which writes a bootable Amplitude DVD image as an ISO, with the built
# executable and IOP modules in place of the originals.
#
# The build-image tool rebuilds the image from an original ISO image or the disc root directory.
# The target exists only once WAVELENGTH_DISC_IMAGE specifies one of them.

# WAVELENGTH_DISC_IMAGE is defined in disc-region.cmake. disc-region.cmake reads it before the
# compile definitions are set.
set(WAVELENGTH_DISC_SYSTEM_AREA
    ""
    CACHE FILEPATH "System area of the original disc, its first 16 sectors (32768 bytes) with the \
boot logo, for a disc root directory. The sectors are zero when unset.")
set(WAVELENGTH_IMAGE_OUTPUT
    "${CMAKE_BINARY_DIR}/wavelength.iso"
    CACHE FILEPATH "ISO image the image target writes.")
# The console loads only the program segment, so the debug information is dead weight on the disc.
# The stripped executable fits the original's extent and is written in place.
option(WAVELENGTH_IMAGE_STRIP "Put an executable without debug information on the image." ON)

if(NOT WAVELENGTH_DISC_IMAGE)
  message(STATUS "Set WAVELENGTH_DISC_IMAGE to an original disc image to enable the image target.")
  return()
endif()

# A disc root is rebuilt whenever one of its files changes.
if(IS_DIRECTORY "${WAVELENGTH_DISC_IMAGE}")
  file(GLOB_RECURSE _wavelength_disc_inputs CONFIGURE_DEPENDS LIST_DIRECTORIES false
       "${WAVELENGTH_DISC_IMAGE}/*")
else()
  set(_wavelength_disc_inputs "${WAVELENGTH_DISC_IMAGE}")
endif()
set(_wavelength_system_area_args)
if(VIDEO_STANDARD STREQUAL "PAL")
  list(APPEND _wavelength_system_area_args --pal)
endif()
if(WAVELENGTH_DISC_SYSTEM_AREA)
  list(APPEND _wavelength_system_area_args --system-area "${WAVELENGTH_DISC_SYSTEM_AREA}")
  list(APPEND _wavelength_disc_inputs "${WAVELENGTH_DISC_SYSTEM_AREA}")
endif()

set(_wavelength_executable "$<TARGET_FILE:${CMAKE_PROJECT_NAME}>")
if(WAVELENGTH_IMAGE_STRIP)
  set(_wavelength_executable "${CMAKE_BINARY_DIR}/WAVELENGTH.ELF")
  add_custom_command(
    OUTPUT "${_wavelength_executable}"
    COMMAND "${CMAKE_OBJCOPY}" --strip-debug $<TARGET_FILE:${CMAKE_PROJECT_NAME}>
            "${_wavelength_executable}"
    DEPENDS ${CMAKE_PROJECT_NAME}
    COMMENT "Writing ${_wavelength_executable}"
    VERBATIM)
endif()
# Each IOP module that irx.cmake registered replaces the module of the same name on the disc.
get_property(_wavelength_irx_files GLOBAL PROPERTY WAVELENGTH_IRX_FILES)
get_property(_wavelength_irx_targets GLOBAL PROPERTY WAVELENGTH_IRX_TARGETS)
set(_wavelength_irx_args)
foreach(_irx IN LISTS _wavelength_irx_files)
  list(APPEND _wavelength_irx_args --iop-module "${_irx}")
endforeach()
add_custom_command(
  OUTPUT "${WAVELENGTH_IMAGE_OUTPUT}"
  COMMAND
    "${WAVELENGTH_TOOL_BUILD_IMAGE}" "${WAVELENGTH_DISC_IMAGE}" "${WAVELENGTH_IMAGE_OUTPUT}"
    --overwrite --wavelength-bin "${_wavelength_executable}" ${_wavelength_irx_args}
    ${_wavelength_system_area_args}
  DEPENDS ${CMAKE_PROJECT_NAME} "${_wavelength_executable}" ${_wavelength_irx_targets}
          ${_wavelength_irx_files} ${_wavelength_disc_inputs} wavelength_tools
          "${WAVELENGTH_TOOL_BUILD_IMAGE}"
  COMMENT "Writing ${WAVELENGTH_IMAGE_OUTPUT}"
  VERBATIM)
add_custom_target(image DEPENDS "${WAVELENGTH_IMAGE_OUTPUT}")

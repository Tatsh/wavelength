# Defines the image target, which writes a bootable Amplitude CD image, a raw MODE2/2352 bin and
# its cue sheet, with the built executable in place of the original.
#
# The build-image tool rebuilds the image from an original disc image. The target exists only once
# WAVELENGTH_DISC_IMAGE specifies an original disc image (cue, bin, or ISO) or the disc root
# directory.

# WAVELENGTH_DISC_IMAGE is defined in disc-region.cmake. disc-region.cmake reads it before the
# compile definitions are set.
set(WAVELENGTH_DISC_SYSTEM_AREA
    ""
    CACHE FILEPATH "First 12 sectors (the boot logo) of the original disc, for a disc root \
directory. The sectors are zero when unset.")
set(WAVELENGTH_IMAGE_OUTPUT
    "${CMAKE_BINARY_DIR}/wavelength.cue"
    CACHE FILEPATH "Cue sheet the image target writes. The bin is written beside it.")
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

get_filename_component(_wavelength_image_dir "${WAVELENGTH_IMAGE_OUTPUT}" DIRECTORY)
get_filename_component(_wavelength_image_stem "${WAVELENGTH_IMAGE_OUTPUT}" NAME_WLE)
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
add_custom_command(
  OUTPUT "${WAVELENGTH_IMAGE_OUTPUT}" "${_wavelength_image_dir}/${_wavelength_image_stem}.bin"
  COMMAND
    "${WAVELENGTH_TOOL_BUILD_IMAGE}" "${WAVELENGTH_DISC_IMAGE}" "${WAVELENGTH_IMAGE_OUTPUT}"
    --overwrite --wavelength-bin "${_wavelength_executable}" ${_wavelength_system_area_args}
  DEPENDS ${CMAKE_PROJECT_NAME} "${_wavelength_executable}" ${_wavelength_disc_inputs}
          wavelength_tools "${WAVELENGTH_TOOL_BUILD_IMAGE}"
  COMMENT "Writing ${WAVELENGTH_IMAGE_OUTPUT}"
  VERBATIM)
add_custom_target(image DEPENDS "${WAVELENGTH_IMAGE_OUTPUT}")

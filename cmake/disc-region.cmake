# Checks that the original disc matches VIDEO_STANDARD. A PAL build requires the European disc
# and an NTSC build the North American one.

set(WAVELENGTH_DISC_IMAGE
    ""
    CACHE PATH "Original Amplitude ISO image, or the disc root directory, \
that the image target rebuilds. It must be the release VIDEO_STANDARD selects.")

if(WAVELENGTH_DISC_IMAGE AND NOT BUILD_DOCS_ONLY)
  execute_process(
    COMMAND "${WAVELENGTH_TOOL_BUILD_IMAGE}" --identify "${WAVELENGTH_DISC_IMAGE}"
    OUTPUT_VARIABLE _wavelength_disc_executable
    ERROR_VARIABLE _wavelength_disc_error
    RESULT_VARIABLE _wavelength_disc_result
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(NOT _wavelength_disc_result EQUAL 0)
    message(FATAL_ERROR "Cannot identify ${WAVELENGTH_DISC_IMAGE}:\n${_wavelength_disc_error}")
  endif()
  if(_wavelength_disc_executable STREQUAL "SCES_517.06")
    set(_wavelength_disc_standard PAL)
  else()
    set(_wavelength_disc_standard NTSC)
  endif()
  if(NOT _wavelength_disc_standard STREQUAL VIDEO_STANDARD)
    message(FATAL_ERROR "VIDEO_STANDARD is ${VIDEO_STANDARD}, and the original boots "
                        "${_wavelength_disc_executable}, the ${_wavelength_disc_standard} release.")
  endif()
endif()

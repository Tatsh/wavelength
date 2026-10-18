# Writes a disc root with the P.O.D. demo's exclusive song added, for the image target. Run with
# cmake -P and these variables:
#
#   DISC_IMAGE      The original ISO image or disc root.
#   DEMO_IMAGE      The P.O.D. demo ISO image or disc root.
#   SPACE_EDITION   The Amplitude-Space-Edition checkout.
#   STAGING_DIR     The directory to write. Its disc subdirectory becomes the disc root.
#   PAL             ON for the European release.
#   EXTRACT_IMAGE, ARK, DEMO_SONG   The host tools.
#   STAMP           The file touched once the disc root is complete.

function(_wavelength_run)
  execute_process(COMMAND ${ARGN} COMMAND_ECHO STDOUT RESULT_VARIABLE _result)
  if(NOT _result EQUAL 0)
    message(FATAL_ERROR "The command failed (${_result}).")
  endif()
endfunction()

set(_disc "${STAGING_DIR}/disc")
set(_ark "${STAGING_DIR}/ark")
set(_order "${STAGING_DIR}/order.txt")
file(REMOVE_RECURSE "${_disc}" "${_ark}" "${STAMP}")

if(IS_DIRECTORY "${DISC_IMAGE}")
  file(COPY "${DISC_IMAGE}/" DESTINATION "${_disc}")
else()
  _wavelength_run("${EXTRACT_IMAGE}" "${DISC_IMAGE}" "${_disc}")
endif()

# Disc file names keep the case of the original, which differs between releases and copies.
file(GLOB_RECURSE _files LIST_DIRECTORIES false RELATIVE "${_disc}" "${_disc}/*")
set(_main_ark "")
foreach(_file IN LISTS _files)
  string(TOLOWER "${_file}" _lower)
  if(_lower STREQUAL "gen/main.ark")
    set(_main_ark "${_disc}/${_file}")
  endif()
endforeach()
if(NOT _main_ark)
  message(FATAL_ERROR "${DISC_IMAGE} has no gen/main.ark.")
endif()

set(_pal_args)
if(PAL)
  list(APPEND _pal_args --pal)
endif()
_wavelength_run("${ARK}" extract "${_main_ark}" "${_ark}" --order "${_order}")
_wavelength_run("${DEMO_SONG}" "${_ark}" "${_disc}" --demo-image "${DEMO_IMAGE}" --space-edition
                "${SPACE_EDITION}" ${_pal_args})
_wavelength_run("${ARK}" create "${_ark}" "${_main_ark}" --order "${_order}")
file(REMOVE_RECURSE "${_ark}")
file(TOUCH "${STAMP}")

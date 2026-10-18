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
# A Debug build puts it on the disc anyway, for a debugger reading the executable from the image.
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(_wavelength_image_strip_default OFF)
else()
  set(_wavelength_image_strip_default ON)
endif()
option(WAVELENGTH_IMAGE_STRIP "Put an executable without debug information on the image."
       ${_wavelength_image_strip_default})

option(ADD_USA_DEMO_SONG "Add \"Space\" by P.O.D., the exclusive song of the P.O.D. demo disc, to \
the image the way Amplitude-Space-Edition does. Requires USA_DEMO_IMAGE, and \
USA_DEMO_SPACE_EDITION or USA_DEMO_SPACE_EDITION_DOWNLOAD." OFF)
set(USA_DEMO_IMAGE
    ""
    CACHE PATH "The ISO image of the P.O.D. demo, Amplitude (USA) (Demo) from the album Payable \
on Death, or its disc root, for ADD_USA_DEMO_SONG.")
set(USA_DEMO_SPACE_EDITION
    ""
    CACHE PATH "Checkout of https://github.com/Squididly/Amplitude-Space-Edition, the recorded \
streams and the song entry, for ADD_USA_DEMO_SONG.")
option(USA_DEMO_SPACE_EDITION_DOWNLOAD "Clone Amplitude-Space-Edition with Git into the build \
directory when USA_DEMO_SPACE_EDITION is unset, for ADD_USA_DEMO_SONG." ON)

if(ADD_USA_DEMO_SONG
   AND NOT USA_DEMO_SPACE_EDITION
   AND USA_DEMO_SPACE_EDITION_DOWNLOAD)
  include(FetchContent)
  FetchContent_Declare(
    amplitude_space_edition
    GIT_REPOSITORY https://github.com/Squididly/Amplitude-Space-Edition.git
    GIT_TAG f395bcb2bc307cb7bc5ddb0c4e74f9481b313a41
    SOURCE_DIR "${CMAKE_BINARY_DIR}/_deps/amplitude-space-edition")
  # The repository has no CMakeLists.txt, so the checkout is not added to the build.
  FetchContent_MakeAvailable(amplitude_space_edition)
  set(USA_DEMO_SPACE_EDITION "${amplitude_space_edition_SOURCE_DIR}")
endif()

if(NOT WAVELENGTH_DISC_IMAGE)
  message(STATUS "Set WAVELENGTH_DISC_IMAGE to an original disc image to enable the image target.")
  return()
endif()

# With the song, the image is built from a disc root that demo-song-stage.cmake writes.
set(_wavelength_image_input "${WAVELENGTH_DISC_IMAGE}")
if(ADD_USA_DEMO_SONG)
  foreach(_variable USA_DEMO_IMAGE USA_DEMO_SPACE_EDITION)
    if(NOT EXISTS "${${_variable}}")
      message(FATAL_ERROR "ADD_USA_DEMO_SONG requires ${_variable}.")
    endif()
  endforeach()
  file(GLOB_RECURSE _wavelength_demo_inputs CONFIGURE_DEPENDS LIST_DIRECTORIES false
       "${USA_DEMO_SPACE_EDITION}/ark/*" "${USA_DEMO_SPACE_EDITION}/disc/*")
  foreach(_image IN ITEMS "${WAVELENGTH_DISC_IMAGE}" "${USA_DEMO_IMAGE}")
    if(IS_DIRECTORY "${_image}")
      file(GLOB_RECURSE _image_files CONFIGURE_DEPENDS LIST_DIRECTORIES false "${_image}/*")
      list(APPEND _wavelength_demo_inputs ${_image_files})
    else()
      list(APPEND _wavelength_demo_inputs "${_image}")
    endif()
  endforeach()
  set(_wavelength_demo_staging "${CMAKE_BINARY_DIR}/demo-song")
  set(_wavelength_demo_stamp "${_wavelength_demo_staging}/disc.stamp")
  if(VIDEO_STANDARD STREQUAL "PAL")
    set(_wavelength_demo_pal ON)
  else()
    set(_wavelength_demo_pal OFF)
  endif()
  add_custom_command(
    OUTPUT "${_wavelength_demo_stamp}"
    COMMAND
      "${CMAKE_COMMAND}" "-DDISC_IMAGE=${WAVELENGTH_DISC_IMAGE}" "-DDEMO_IMAGE=${USA_DEMO_IMAGE}"
      "-DSPACE_EDITION=${USA_DEMO_SPACE_EDITION}" "-DSTAGING_DIR=${_wavelength_demo_staging}"
      "-DPAL=${_wavelength_demo_pal}" "-DEXTRACT_IMAGE=${WAVELENGTH_TOOL_EXTRACT_IMAGE}"
      "-DARK=${WAVELENGTH_TOOL_ARK}" "-DDEMO_SONG=${WAVELENGTH_TOOL_DEMO_SONG}"
      "-DSTAMP=${_wavelength_demo_stamp}" -P "${CMAKE_SOURCE_DIR}/cmake/demo-song-stage.cmake"
    DEPENDS "${CMAKE_SOURCE_DIR}/cmake/demo-song-stage.cmake" ${_wavelength_demo_inputs}
            wavelength_tools "${WAVELENGTH_TOOL_ARK}" "${WAVELENGTH_TOOL_DEMO_SONG}"
            "${WAVELENGTH_TOOL_EXTRACT_IMAGE}"
    COMMENT "Adding the P.O.D. demo song to a copy of the original disc"
    VERBATIM)
  set(_wavelength_image_input "${_wavelength_demo_staging}/disc")
endif()

# A disc root is rebuilt whenever one of its files changes.
if(IS_DIRECTORY "${WAVELENGTH_DISC_IMAGE}")
  file(GLOB_RECURSE _wavelength_disc_inputs CONFIGURE_DEPENDS LIST_DIRECTORIES false
       "${WAVELENGTH_DISC_IMAGE}/*")
else()
  set(_wavelength_disc_inputs "${WAVELENGTH_DISC_IMAGE}")
endif()
if(ADD_USA_DEMO_SONG)
  list(APPEND _wavelength_disc_inputs "${_wavelength_demo_stamp}")
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
    "${WAVELENGTH_TOOL_BUILD_IMAGE}" "${_wavelength_image_input}" "${WAVELENGTH_IMAGE_OUTPUT}"
    --overwrite --wavelength-bin "${_wavelength_executable}" ${_wavelength_irx_args}
    ${_wavelength_system_area_args}
  DEPENDS ${CMAKE_PROJECT_NAME} "${_wavelength_executable}" ${_wavelength_irx_targets}
          ${_wavelength_irx_files} ${_wavelength_disc_inputs} wavelength_tools
          "${WAVELENGTH_TOOL_BUILD_IMAGE}"
  COMMENT "Writing ${WAVELENGTH_IMAGE_OUTPUT}"
  VERBATIM)
add_custom_target(image DEPENDS "${WAVELENGTH_IMAGE_OUTPUT}")

set(WAVELENGTH_MET_SRCS
    freqconfirmscreen.cpp
    freqpanel.cpp
    freqscreen.cpp
    gizmo.cpp
    introscreen.cpp
    metaarenascreen.cpp
    metagame.cpp
    metamainscreen.cpp
    metaskillscreen.cpp
    metasongscreen.cpp
    metastartscreen.cpp
    modescreen.cpp
    songpicpanel.cpp
    transitionscreen.cpp)

add_library(wavelength_met STATIC ${WAVELENGTH_MET_SRCS})
target_link_libraries(wavelength_met PRIVATE wavelength_common)
# Patched builds show the commit and the build time on the title screen. The header is regenerated
# on every build.
if(ENABLE_PATCHES)
  set(_build_info "${CMAKE_BINARY_DIR}/generated/buildinfo.h")
  add_custom_target(
    wavelength_build_info
    COMMAND "${CMAKE_COMMAND}" "-DSOURCE_DIR=${CMAKE_SOURCE_DIR}" "-DOUTPUT=${_build_info}" -P
            "${CMAKE_SOURCE_DIR}/cmake/build-info.cmake"
    BYPRODUCTS "${_build_info}"
    VERBATIM)
  add_dependencies(wavelength_met wavelength_build_info)
  target_include_directories(wavelength_met PRIVATE "${CMAKE_BINARY_DIR}/generated")

  # The credits of patched builds open with this entry. The avatar is fetched once, when the
  # header is first generated. When it cannot be fetched, the entry shows the text alone.
  set(WAVELENGTH_CREDITS_GITHUB_USER
      "Tatsh"
      CACHE STRING "GitHub user whose avatar the first credit shows.")
  set(WAVELENGTH_CREDITS_TEXT
      "Reconstruction:\\nAndrew Udvare\\ngithub.com/Tatsh"
      CACHE STRING "Text of the first credit. \\n breaks a line.")
  find_program(WAVELENGTH_MAGICK magick)
  set(_credits_magick "${WAVELENGTH_MAGICK}")
  if(NOT WAVELENGTH_MAGICK)
    message(STATUS "ImageMagick was not found. The first credit has no avatar.")
    set(_credits_magick "")
  endif()
  set(_credits_avatar "${CMAKE_BINARY_DIR}/generated/creditsavatar.h")
  add_custom_command(
    OUTPUT "${_credits_avatar}"
    COMMAND
      "${WAVELENGTH_TOOL_CREDITS_AVATAR}" "${_credits_avatar}" --magick "${_credits_magick}" --user
      "${WAVELENGTH_CREDITS_GITHUB_USER}" --text "${WAVELENGTH_CREDITS_TEXT}"
    DEPENDS wavelength_tools "${WAVELENGTH_TOOL_CREDITS_AVATAR}"
    COMMENT "Writing ${_credits_avatar}"
    VERBATIM)
  add_custom_target(wavelength_credits_avatar DEPENDS "${_credits_avatar}")
  add_dependencies(wavelength_met wavelength_credits_avatar)
endif()

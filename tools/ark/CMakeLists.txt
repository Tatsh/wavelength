# The archive and DataArray code is shared with demo-song. Rand and HashString are compiled from the
# game's sources, so the cipher and the name hash are the game's own.
add_library(
  wavelength_tools_ark STATIC
  arkreader.cpp
  arkwriter.cpp
  datafile.cpp
  datanode.cpp
  datatext.cpp
  "${WAVELENGTH_GAME_SOURCE_DIR}/math/hash.cpp"
  "${WAVELENGTH_GAME_SOURCE_DIR}/math/rand.cpp")
target_include_directories(wavelength_tools_ark PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
                                                       "${WAVELENGTH_GAME_SOURCE_DIR}")
target_link_libraries(wavelength_tools_ark PUBLIC wavelength_tools_common)

add_executable(ark main.cpp)
target_link_libraries(ark PRIVATE argparse::argparse wavelength_tools_ark wavelength_tools_common
                                  spdlog::spdlog)

add_executable(
  build-image
  directoryvolume.cpp
  githubclient.cpp
  imagebuilder.cpp
  main.cpp
  sourceimage.cpp
  volume.cpp
  ziparchive.cpp)
target_link_libraries(
  build-image PRIVATE argparse::argparse nlohmann_json::nlohmann_json wavelength_tools_common
                      wavelength_tools_https spdlog::spdlog ZLIB::ZLIB)

add_executable(demo-song demosong.cpp main.cpp)
target_link_libraries(
  demo-song PRIVATE argparse::argparse wavelength_tools_ark wavelength_tools_common
                    spdlog::spdlog)

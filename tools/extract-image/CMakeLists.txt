add_executable(extract-image main.cpp)
target_link_libraries(extract-image PRIVATE argparse::argparse wavelength_tools_common
                                            spdlog::spdlog)

add_executable(credits-avatar avatar.cpp creditsheader.cpp main.cpp)
target_link_libraries(credits-avatar PRIVATE argparse::argparse wavelength_tools_common
                                             wavelength_tools_https spdlog::spdlog)

add_library(wavelength_tools_common STATIC fileio.cpp logging.cpp)
target_include_directories(wavelength_tools_common PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(wavelength_tools_common PUBLIC spdlog::spdlog)

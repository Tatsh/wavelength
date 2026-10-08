set(WAVELENGTH_MATH_SRCS
    box.cpp
    color.cpp
    frustum.cpp
    interpolator.cpp
    plane.cpp
    quaternion.cpp
    sphere.cpp
    transformops.cpp
    vector2.cpp
    vector3.cpp)

add_library(wavelength_math STATIC ${WAVELENGTH_MATH_SRCS})
target_link_libraries(wavelength_math PRIVATE wavelength_common)

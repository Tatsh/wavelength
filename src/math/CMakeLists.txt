set(WAVELENGTH_MATH_SRCS
    box.cpp
    color.cpp
    frustum.cpp
    hash.cpp
    interpolator.cpp
    plane.cpp
    quaternion.cpp
    rand.cpp
    rect.cpp
    sine.cpp
    sphere.cpp
    statsaccumulator.cpp
    transform.cpp
    transformops.cpp
    triangle.cpp
    vector2.cpp
    vector3.cpp)

add_library(wavelength_math STATIC ${WAVELENGTH_MATH_SRCS})
target_link_libraries(wavelength_math PRIVATE wavelength_common)

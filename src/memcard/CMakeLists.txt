set(WAVELENGTH_MEMCARD_SRCS
    checkinfoop.cpp
    closeop.cpp
    createdirop.cpp
    deletefileop.cpp
    entspaceop.cpp
    formatop.cpp
    listdirop.cpp
    memcard.cpp
    memcardcbhandler.cpp
    memcardop.cpp
    memcardsavepaths.cpp
    memcardtask.cpp
    memcarduser.cpp
    openreadop.cpp
    openwriteop.cpp
    readop.cpp
    renamefileop.cpp
    seekop.cpp
    shiftjis.cpp
    unformatop.cpp
    writeop.cpp)

add_library(wavelength_memcard STATIC ${WAVELENGTH_MEMCARD_SRCS})
target_link_libraries(wavelength_memcard PRIVATE wavelength_common)

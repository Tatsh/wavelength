set(WAVELENGTH_APP_SRCS
    animhelper.cpp
    appactivefilter.cpp
    appticktask.cpp
    apptimetask.cpp
    attachment.cpp
    cutscene.cpp
    linearanim.cpp
    longop.cpp
    msgsink.cpp
    msgsource.cpp
    ovyassembly.cpp
    playbacker.cpp
    recorder.cpp
    scheduler.cpp
    task.cpp
    timeclock.cpp)

add_library(wavelength_app STATIC ${WAVELENGTH_APP_SRCS})
target_link_libraries(wavelength_app PRIVATE wavelength_common)

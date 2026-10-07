set(WAVELENGTH_UI_SRCS
    localizeerrors.cpp
    lrbutton.cpp
    uibutton.cpp
    uicomponent.cpp
    uicomponentfocuschangemsg.cpp
    uicomponentselectmsg.cpp
    uicomponentselectstartmsg.cpp
    uilabel.cpp
    uilist.cpp
    uimanager.cpp
    uinavigator.cpp
    uipanel.cpp
    uiscreen.cpp
    uiscreenchangemsg.cpp
    uistyle.cpp
    uitextentry.cpp
    uitextentrycompletemsg.cpp
    uitextentryinvalidmsg.cpp
    uitransitioncompletemsg.cpp)

add_library(wavelength_ui STATIC ${WAVELENGTH_UI_SRCS})
target_link_libraries(wavelength_ui PRIVATE wavelength_common)

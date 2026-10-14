set(WAVELENGTH_APP_SRCS
    attachment.cpp
    cutscene.cpp
    editfield.cpp
    hideablepanel.cpp
    hudcommon.cpp
    hudflyingbutton.cpp
    hudhilitebox.cpp
    hudletterbox.cpp
    hudletterexit.cpp
    hudpoints.cpp
    hudpowerup.cpp
    hudstick.cpp
    hudtextmessage.cpp
    hudtextnode.cpp
    msgsink.cpp
    msgsource.cpp
    overlay.cpp
    ovyallplayer.cpp
    ovyavatar.cpp
    ovychat.cpp
    ovycontroller.cpp
    ovydialog.cpp
    ovyduelscore.cpp
    ovyjuice.cpp
    ovylocalplayer.cpp
    ovyremixgenpanel.cpp
    ovyscore.cpp
    ovyremixpanel.cpp
    ovyremixsubpanel.cpp
    ovysectionpanel.cpp
    ovysongpos.cpp
    ovytracklabel.cpp
    rampanimator.cpp
)

add_library(wavelength_app STATIC ${WAVELENGTH_APP_SRCS})
target_link_libraries(wavelength_app PRIVATE wavelength_common)

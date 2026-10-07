#pragma once

#include "met/avatarpanel.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uicomponentselectstartmsg.h"

/**
 * Panel with which one player of a local game chooses a Freq.
 *
 * The RTTI records the class as deriving from AvatarPanel. Only the routines
 * MultiFreqSelectScreen calls are declared, and the routines of the class are not reconstructed.
 */
class FreqSelPanel : public AvatarPanel {
public:
    /**
     * Respond to a component of the panel being chosen by the panel's controller.
     *
     * @param pMsg The message.
     * @return Whether the choice was handled.
     * @ghidraAddress NTSC-U/C: 0x001713e0
     * @ghidraAddress PAL: 0x001746e8
     */
    bool HandleSelectStart(UIComponentSelectStartMsg *pMsg);

    /**
     * Respond to a component of the panel chosen with the cross button.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00171508
     * @ghidraAddress PAL: 0x00174810
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);
};

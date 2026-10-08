#pragma once

#include <list>

#include "app/hideablepanel.h"
#include "app/ovyremixgenpanel.h"
#include "app/ovyremixsubpanel.h"
#include "math/vector3.h"
#include "rnd/transformable.h"
#include "script/dataarray.h"

/**
 * Remix head-up display of one player: the main panel and the effect panels that slide out of it.
 *
 * The RTTI records the class as deriving from OvyRemixGenPanel at offset 0 and from HideablePanel
 * at `+0x48`. The object is 0x74 bytes.
 */
class OvyRemixPanel : public OvyRemixGenPanel, public HideablePanel {
public:
    /** The number of places a remix panel can sit at. */
    static constexpr int kNumPositions = 4;

    /**
     * Construct a hidden remix display for a player.
     *
     * The scene objects are `HUD<hud>r_fx<n>`, where the layout letter and n follow the community
     * of the game: `s` and 0 alone, `m` and 0 for the first local player and 1 for the others,
     * and `n` and the player's online order.
     *
     * @param nPlayer The player's index.
     * @param nPosition The place the panel sits at, an index of sPositions.
     * @param pParent The transform the panel moves with, or null.
     * @ghidraAddress NTSC-U/C: 0x001b7b40
     * @ghidraAddress PAL: 0x001c08e0
     */
    OvyRemixPanel(int nPlayer, int nPosition, Rnd::Transformable *pParent);

    /**
     * Destroy the display and its effect panels.
     *
     * @ghidraAddress NTSC-U/C: 0x001b8088
     * @ghidraAddress PAL: 0x001c0e28
     */
    ~OvyRemixPanel() override;

    /**
     * Advance the slides and the blinks, and slide out the effect panel ShowSubPanel() asked for
     * once the previous one is back.
     *
     * @param fDelta The time since the last poll.
     * @ghidraAddress NTSC-U/C: 0x001b8198
     * @ghidraAddress PAL: 0x001c0f38
     */
    void Poll(float fDelta) override;

    /**
     * Draw the effect panels and the main panel while the display is not hidden.
     *
     * @ghidraAddress NTSC-U/C: 0x001b8288
     * @ghidraAddress PAL: 0x001c1028
     */
    void Draw() override;

    /**
     * Slide out the effect panel of a type, after the shown one slides back.
     *
     * @param nType The type, one of GfxManager::RemixSubPanel.
     * @ghidraAddress NTSC-U/C: 0x001b8330
     * @ghidraAddress PAL: 0x001c10d0
     */
    void ShowSubPanel(int nType);

    /**
     * Find the panel of a type.
     *
     * @param nType The type, one of GfxManager::RemixSubPanel.
     * @return This display for the main panel, the effect panel of the type, or null.
     * @ghidraAddress NTSC-U/C: 0x001b8400
     * @ghidraAddress PAL: 0x001c11a0
     */
    OvyRemixGenPanel *FindSubPanel(int nType);

    /**
     * Read the places of the remix panels from `remix_panel_positions`.
     *
     * @param pConfig The configuration section, or null.
     * @param pDefaults The section of defaults, or null.
     * @ghidraAddress NTSC-U/C: 0x001b6980
     * @ghidraAddress PAL: 0x001bf720
     */
    static void LoadPositions(DataArray *pConfig, DataArray *pDefaults);

    /**
     * The places a remix panel can sit at, read by LoadPositions().
     *
     * @ghidraAddress NTSC-U/C: 0x0043b1a0
     */
    static Vector3 sPositions[kNumPositions];

    std::list<OvyRemixSubPanel *> mSubPanels; /*!< The effect panels, which the display owns. */
    OvyRemixSubPanel *mShownSubPanel;         /*!< The effect panel slid out, or null. */
    OvyRemixSubPanel *mNextSubPanel;          /*!< The effect panel to slide out next, or null. */
};

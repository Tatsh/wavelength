#pragma once

#include "met/avatarpanel.h"
#include "script/dataarray.h"
#include "ui/uipanel.h"

/**
 * Panel that shows the name, the place, the score, the colour, and the Freq of one player after a
 * multiplayer game.
 *
 * The RTTI records the class as deriving from AvatarPanel.
 */
class MultiGameStatsPanel : public AvatarPanel {
public:
    /**
     * Construct the panel from its script description, with its `rank_order`.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @ghidraAddress NTSC-U/C: 0x00170790
     * @ghidraAddress PAL: 0x00173a98
     */
    MultiGameStatsPanel(DataArray *pData, const char *pszDir);

    /**
     * Destroy the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x00357730
     * @ghidraAddress PAL: 0x003c4990
     */
    ~MultiGameStatsPanel() override {
    }

    /**
     * Create a panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @return The new panel.
     */
    static UIPanel *New(DataArray *pData, const char *pszDir) {
        return new MultiGameStatsPanel(pData, pszDir);
    }

    /**
     * Show a player.
     *
     * The name is inferred.
     *
     * @param nPlayer The player.
     * @ghidraAddress NTSC-U/C: 0x001708a0
     * @ghidraAddress PAL: 0x00173ba8
     */
    virtual void SetPlayer(int nPlayer);

    /**
     * Report the localised label of a place.
     *
     * The name is inferred.
     *
     * @param nRank The place, from 0.
     * @param bTied Whether the place is shared.
     * @return The label of the `<place>_place` or `<place>_place_tie` token.
     * @ghidraAddress NTSC-U/C: 0x001707f0
     * @ghidraAddress PAL: 0x00173af8
     */
    static const char *RankLabel(int nRank, bool bTied);

    int mRankOrder; /*!< `rank_order`, the place of the players this panel shows. */
    int mPlayer;    /*!< The player shown. */
    int mTied;      /*!< Non-zero when the player shares the place. */
};

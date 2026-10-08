#pragma once

#include "met/freqpanel.h"
#include "met/viewanimplayer.h"
#include "script/dataarray.h"
#include "ui/uicomponent.h"
#include "ui/uipanel.h"

/**
 * Panel that shows a tip after a lost solo game, chosen from the `lose_tips` entry of the
 * `metagame` configuration.
 *
 * The RTTI records the class as deriving from FreqPanel. The panel alternates between a tip for
 * the part of the song played and a filler tip.
 */
class SoloLoseTipsPanel : public FreqPanel {
public:
    /**
     * Construct the panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @ghidraAddress NTSC-U/C: 0x00170150
     * @ghidraAddress PAL: 0x00173458
     */
    SoloLoseTipsPanel(DataArray *pData, const char *pszDir);

    /**
     * Destroy the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x003574e8
     * @ghidraAddress PAL: 0x003c4748
     */
    ~SoloLoseTipsPanel() override {
    }

    /**
     * Create a panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @return The new panel.
     * @ghidraAddress NTSC-U/C: 0x00357498
     * @ghidraAddress PAL: 0x003c46f8
     */
    static UIPanel *New(DataArray *pData, const char *pszDir) {
        return new SoloLoseTipsPanel(pData, pszDir);
    }

    /**
     * Unload the panel and drop the view of the tip.
     *
     * @ghidraAddress NTSC-U/C: 0x00170320
     * @ghidraAddress PAL: 0x00173628
     */
    void Unload() override;

    /**
     * Choose a tip, start its view after the entry, and enter with the tip hidden.
     *
     * @param bForce Whether the panel skips its animation.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00170198
     * @ghidraAddress PAL: 0x001734a0
     */
    void Enter(bool bForce, float fTime) override;

    /**
     * Advance the view of the tip, and show the tip once the panel shows and the view ends.
     *
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00170350
     * @ghidraAddress PAL: 0x00173658
     */
    void Poll(float fTime) override;

private:
    /**
     * Choose the token of a tip.
     *
     * The name is inferred.
     *
     * @param pTips The `lose_tips` entry.
     * @return The token, or null when no `percent_done` entry matches.
     * @ghidraAddress NTSC-U/C: 0x001703d0
     * @ghidraAddress PAL: 0x001736d8
     */
    const char *PickTip(DataArray *pTips);

    /**
     * Choose an available filler tip at random, and choose a tip for the progress next time.
     *
     * The name is inferred.
     *
     * @param pFiller The `filler` entry.
     * @param nSkillLevel The skill level of the game.
     * @return The token.
     * @ghidraAddress NTSC-U/C: 0x00170528
     * @ghidraAddress PAL: 0x00173830
     */
    const char *PickFillerTip(DataArray *pFiller, int nSkillLevel);

    /**
     * Choose an available tip of a `percent_done` entry at random when the entry matches the
     * skill level and the progress, and choose a filler tip next time.
     *
     * The name is inferred.
     *
     * @param pEntry The `percent_done` entry.
     * @param nSkillLevel The skill level of the game.
     * @param fProgress The fraction of the song played.
     * @return The token, or null when the entry does not match.
     * @ghidraAddress NTSC-U/C: 0x001705c0
     * @ghidraAddress PAL: 0x001738c8
     */
    const char *PickProgressTip(DataArray *pEntry, int nSkillLevel, float fProgress);

    /**
     * Report whether a tip is available, either `always` or by the item it lists.
     *
     * The name is inferred.
     *
     * @param pTip The tip.
     * @param nSkillLevel The skill level of the game.
     * @return Whether the tip is available.
     * @ghidraAddress NTSC-U/C: 0x00170700
     * @ghidraAddress PAL: 0x00173a08
     */
    bool IsTipAvailable(DataArray *pTip, int nSkillLevel);

    int mProgressTipNext;    /*!< Non-zero when the next tip is chosen for the progress. */
    ViewAnimPlayer mTipAnim; /*!< The player of the view of the tip. */
    UIComponent *mTip;       /*!< The `tip` component. */
};

#pragma once

#include <vector>

#include "game/campaign.h"
#include "met/avatarpanel.h"
#include "script/dataarray.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uicomponentselectstartmsg.h"
#include "ui/uipanel.h"

/**
 * Panel where one player chooses a Freq from a list of profiles with the left and right buttons.
 *
 * The RTTI records the class as deriving from AvatarPanel.
 */
class FreqSelPanel : public AvatarPanel {
public:
    /**
     * Construct the panel from its script description, with its `avatar_index`.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @ghidraAddress NTSC-U/C: 0x00170b70
     * @ghidraAddress PAL: 0x00173e78
     */
    FreqSelPanel(DataArray *pData, const char *pszDir);

    /**
     * Destroy the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x00357788
     * @ghidraAddress PAL: 0x003c49e8
     */
    ~FreqSelPanel() override {
    }

    /**
     * Create a panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @return The new panel.
     * @ghidraAddress NTSC-U/C: 0x00357898
     * @ghidraAddress PAL: 0x003c4af8
     */
    static UIPanel *New(DataArray *pData, const char *pszDir) {
        return new FreqSelPanel(pData, pszDir);
    }

    /**
     * Route the choices of the player.
     *
     * @param pMsg The message.
     * @return Whether the message was handled.
     * @ghidraAddress NTSC-U/C: 0x00171350
     * @ghidraAddress PAL: 0x00174658
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Unload the panel and drop the profiles.
     *
     * @ghidraAddress NTSC-U/C: 0x001712d8
     * @ghidraAddress PAL: 0x001745e0
     */
    void Unload() override;

    /**
     * Enter and show the chosen Freq.
     *
     * In a local multiplayer game the panel with the focus first gives the chosen profile to its
     * player, unless the first player's profile is the player's own.
     *
     * @param bForce Whether the panel skips its animation.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00170ca0
     * @ghidraAddress PAL: 0x00173fa8
     */
    void Enter(bool bForce, float fTime) override;

    /**
     * Replace the profiles to choose from.
     *
     * The name is inferred.
     *
     * @param profiles The profiles.
     * @param nSelected The profile chosen first.
     * @ghidraAddress NTSC-U/C: 0x00170be8
     * @ghidraAddress PAL: 0x00173ef0
     */
    void SetProfiles(const std::vector<Campaign> &profiles, int nSelected);

    int mAvatarIndex; /*!< `avatar_index`, the player who chooses. */
    int mNumProfiles; /*!< The number of profiles, from the last entry. */
    int mSelected;    /*!< The chosen profile. */

    /**
     * Mark the choice as made or not, lighting the tab and the Freq and hiding the arrows.
     *
     * The name is inferred.
     *
     * @param bChosen Whether the choice is made.
     * @ghidraAddress NTSC-U/C: 0x00170dc8
     * @ghidraAddress PAL: 0x001740d0
     */
    void SetChosen(bool bChosen);

    /**
     * Move through the profiles with the left and right buttons, or give the chosen profile to
     * the player with the cross button.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return Whether the choice moved.
     * @ghidraAddress NTSC-U/C: 0x001713e0
     * @ghidraAddress PAL: 0x001746e8
     */
    bool HandleSelectStart(UIComponentSelectStartMsg *pMsg);

    /**
     * Mark the choice as made when the cross button chooses.
     *
     * The name is inferred.
     *
     * @param pMsg The message.
     * @return False.
     * @ghidraAddress NTSC-U/C: 0x00171508
     * @ghidraAddress PAL: 0x00174810
     */
    bool HandleSelect(UIComponentSelectMsg *pMsg);

private:
    /**
     * Show the chosen profile's Freq, name, and rank icon.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00171538
     * @ghidraAddress PAL: 0x00174840
     */
    void ShowSelected();

    int mReserved10C;                // +0x10c, not yet identified.
    std::vector<Campaign> mProfiles; /*!< The profiles to choose from. */
};

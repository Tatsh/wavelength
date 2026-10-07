#pragma once

#include "game/avatarpartset.h"
#include "met/freqpanel.h"
#include "rnd/mesh.h"
#include "rnd/view.h"
#include "script/dataarray.h"

/**
 * Panel that shows a player's Freq over the mesh `<name>_freq.mesh`, such as the `f_maker_p`
 * panel of the Freq maker.
 *
 * The RTTI records the class as deriving from FreqPanel. The object is 0x100 bytes and its vtable
 * is at `0x003cf510`. The metagame registers the class for the panel type `avatar_panel`. A
 * `spin_control` entry lets the right analogue stick of the first controller turn the Freq. The
 * view `<name>_post_avatar.view` draws over the Freq, after it rather than with the panel.
 */
class AvatarPanel : public FreqPanel {
public:
    /**
     * Construct the panel from its script description, showing no Freq.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @ghidraAddress NTSC-U/C: 0x00199df8
     * @ghidraAddress PAL: 0x001a1338
     */
    AvatarPanel(DataArray *pData, const char *pszDir);

    /**
     * Destroy the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x00199e58
     * @ghidraAddress PAL: 0x001a1398
     */
    ~AvatarPanel() override;

    /**
     * Create a panel from its script description.
     *
     * The metagame registers the routine for the entry type `avatar_panel`.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @return The new panel.
     * @ghidraAddress NTSC-U/C: 0x0035f350
     * @ghidraAddress PAL: 0x003cd430
     */
    static UIPanel *New(DataArray *pData, const char *pszDir) {
        return new AvatarPanel(pData, pszDir);
    }

    /**
     * Start the entry and face the Freq forward.
     *
     * @param bForce Whether the panel skips its animation.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0019a080
     * @ghidraAddress PAL: 0x001a15c0
     */
    void Enter(bool bForce, float fTime) override;

    /**
     * Start the exit, stop showing the Freq, and face the avatar forward.
     *
     * @param bForce Whether the panel skips its animation.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0019a0c8
     * @ghidraAddress PAL: 0x001a1608
     */
    void Exit(bool bForce, float fTime) override;

    /**
     * Draw the panel, then the Freq turned by the stick, then the view over it.
     *
     * The stick turns the Freq only while the panel shows and does not animate.
     *
     * @ghidraAddress NTSC-U/C: 0x00199fd0
     * @ghidraAddress PAL: 0x001a1510
     */
    void Draw() override;

    /**
     * Find the view that draws over the Freq and take it out of the panel's view, and find the
     * mesh the Freq covers.
     *
     * @ghidraAddress NTSC-U/C: 0x00199eb0
     * @ghidraAddress PAL: 0x001a13f0
     */
    void FinishLoad() override;

    /**
     * Show another Freq, returning the avatar player of the one shown before.
     *
     * @param pAvatar The Freq, or null for none.
     * @ghidraAddress NTSC-U/C: 0x0019a128
     * @ghidraAddress PAL: 0x001a1668
     */
    void SetAvatar(AvatarPartSet *pAvatar);

    int mSpinControl;       /*!< Non-zero when the stick turns the Freq, `spin_control`. */
    AvatarPartSet *mAvatar; /*!< The Freq that shows, or null. */
    Rnd::Mesh *mFreqMesh;   /*!< The mesh `<name>_freq.mesh` the Freq covers, or null. */
    Rnd::View *mPostView;   /*!< The view `<name>_post_avatar.view`, or null. */
    float mSpinX;           /*!< The horizontal stick position, eased toward the stick. */
    float mSpinY;           /*!< The vertical stick position, eased toward the stick. */
};

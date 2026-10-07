#pragma once

#include "met/freqpanel.h"
#include "rnd/animatable.h"
#include "rnd/drawable.h"
#include "rnd/view.h"
#include "script/dataarray.h"

/**
 * The picture of the controller on the controller menu, which highlights one button or stick.
 *
 * The RTTI records the class as deriving from FreqPanel. The object is 0x170 bytes and its vtable
 * is at `0x003cc288`. Each region of the picture is a button or a stick, indexed by JoypadButton,
 * and has a mesh that shows while the region is highlighted and an animation the view runs.
 */
class ControllerPanel : public FreqPanel {
public:
    /** The number of regions, one for each button and stick. */
    static constexpr int kNumRegions = 16;

    /** One button or stick of the picture. */
    class Region {
    public:
        /**
         * Find the animation and the highlight of the region.
         *
         * @param pszName The name of the region.
         * @ghidraAddress NTSC-U/C: 0x0016dae0
         * @ghidraAddress PAL: 0x00170d08
         */
        void Load(const char *pszName);

        Rnd::Animatable *mAnim; /*!< The animation, `o_controller_map_<name>.tnm`. */
        Rnd::Drawable *mMesh;   /*!< The highlight, `o_controller_map_<name>.mesh`. */
    };

    /**
     * Construct the panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the panel's files.
     * @ghidraAddress NTSC-U/C: 0x0016d788
     * @ghidraAddress PAL: 0x001709b0
     */
    ControllerPanel(DataArray *pData, const char *pszDir);

    /**
     * Create a panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the panel's files.
     * @return The new panel.
     * @ghidraAddress NTSC-U/C: 0x00356af0
     * @ghidraAddress PAL: 0x003c3d50
     */
    static UIPanel *New(DataArray *pData, const char *pszDir) {
        return new ControllerPanel(pData, pszDir);
    }

    /**
     * Find the picture's view and regions, hide every highlight, and enter.
     *
     * @param bForce Whether the panel skips its animation.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0016d7e8
     * @ghidraAddress PAL: 0x00170a10
     */
    void Enter(bool bForce, float fTime) override;

    /**
     * Run the picture's view to the time, then the panel.
     *
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0016d9e0
     * @ghidraAddress PAL: 0x00170c08
     */
    void Poll(float fTime) override;

    /**
     * Highlight one region and stop highlighting the previous one.
     *
     * @param nRegion The region, one of JoypadButton, or kPadNone for none.
     * @ghidraAddress NTSC-U/C: 0x0016da28
     * @ghidraAddress PAL: 0x00170c50
     */
    void SetRegion(int nRegion);

    Region mRegions[kNumRegions]; /*!< The regions, indexed by JoypadButton. */
    Rnd::View *mView;             /*!< The view of the picture, `o_controller_map.view`. */
    int mRegion;                  /*!< The highlighted region, or kPadNone. */
};

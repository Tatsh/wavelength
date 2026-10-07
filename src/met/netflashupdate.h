#pragma once

#include "rnd/mat.h"
#include "rnd/mesh.h"
#include "rnd/text.h"
#include "rnd/view.h"

/**
 * Notice that a panel's data changed: the panel's `refresh` text replaces its titles while its
 * mesh flashes between two materials.
 *
 * The RTTI records the class with no base. Panels derive from it alongside FreqPanel.
 */
class NetFlashUpdate {
public:
    /**
     * Construct a notice that is not flashing.
     *
     * @ghidraAddress NTSC-U/C: 0x00171908
     * @ghidraAddress PAL: 0x00174d50
     */
    NetFlashUpdate();

    /**
     * Find the objects of a panel and the materials of a kind of panel.
     *
     * The name is inferred.
     *
     * @param pszPanel The name of the panel.
     * @param pszKind The kind of the materials `panel_<kind>.mat` and `panel_<kind>_hi.mat`.
     * @ghidraAddress NTSC-U/C: 0x00171988
     * @ghidraAddress PAL: 0x00174dd0
     */
    void InitFlash(const char *pszPanel, const char *pszKind);

    /**
     * Stop flashing and give the mesh its final material.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00171938
     * @ghidraAddress PAL: 0x00174d80
     */
    void StopFlash();

    /**
     * Stop flashing, hide the `refresh` text, and show the titles.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00171be0
     * @ghidraAddress PAL: 0x00175028
     */
    void HideFlash();

    /**
     * Show the `refresh` text instead of the titles and start flashing.
     *
     * The name is inferred.
     *
     * @param fTime The front-end time in milliseconds.
     * @param bHilite Whether the mesh ends with the lit material.
     * @ghidraAddress NTSC-U/C: 0x00171c40
     * @ghidraAddress PAL: 0x00175088
     */
    void StartFlash(float fTime, bool bHilite);

    /**
     * Swap the material of the mesh each interval until the mesh has flashed enough times.
     *
     * The name is inferred.
     *
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x00171cb8
     * @ghidraAddress PAL: 0x00175100
     */
    void PollFlash(float fTime);

    int mHilite;           /*!< Non-zero when the mesh ends with mHiliteMat. */
    int mFlashing;         /*!< Non-zero while the mesh flashes. */
    int mFlashes;          /*!< The number of swaps so far. */
    int mMaxFlashes;       /*!< The number of swaps a flash makes. */
    Rnd::Mat *mMat;        /*!< The material `panel_<kind>.mat`. */
    Rnd::Mat *mHiliteMat;  /*!< The material `panel_<kind>_hi.mat`. */
    float mFlashTime;      /*!< The time of the last swap, or 0. */
    float mFlashInterval;  /*!< The time between swaps in milliseconds. */
    Rnd::View *mTitles;    /*!< The view `<panel>_titles.view`. */
    Rnd::Text *mRefresh;   /*!< The text `<panel>_refresh.txt`. */
    Rnd::Mesh *mPanelMesh; /*!< The mesh `<panel>_panel.mesh`. */
};

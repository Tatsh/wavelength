#pragma once

#include "math/box.h"
#include "math/vector3.h"
#include "rnd/mesh.h"
#include "rnd/view.h"
#include "script/dataarray.h"
#include "ui/uicomponent.h"
#include "ui/uipanel.h"

/**
 * Front-end panel of Amplitude's menus, with the gizmo pyramid and the panel material animations.
 *
 * The RTTI records the class as deriving from UIPanel. The object is 0xe0 bytes, and the vtable is
 * at `0x003cfc28`. The metagame registers the class for the panel type `freq_panel`, and most panel
 * classes of the menus derive from it.
 *
 * Once its file has loaded, a panel clones `pyramid.mesh` under its own name as the gizmo mesh. The
 * pyramid reaches from the panel's background mesh `<name>_bg.mesh` to the gizmo offset the panel's
 * `gizmoOffsetIndex` selects, and it draws under the panel. The panel also drives the shared
 * material animations `mat_2d_always.anim` and `mat_2d_EE.anim`, and the copy of the transform
 * animation `panel_anim_name` it makes for its `<name>_panel.mesh`.
 */
class FreqPanel : public UIPanel {
public:
    /** The number of gizmo offsets the configuration provides. */
    static constexpr int kNumGizmoOffsets = 3;

    /**
     * Construct a panel from its script description.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @ghidraAddress NTSC-U/C: 0x0019d568
     * @ghidraAddress PAL: 0x001a5280
     */
    FreqPanel(DataArray *pData, const char *pszDir);

    /**
     * Destroy the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x0019dcd8
     * @ghidraAddress PAL: 0x001a59f0
     */
    ~FreqPanel() override;

    /**
     * Create a panel from its script description.
     *
     * The metagame registers the routine for the panel type `freq_panel`.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @return The new panel.
     * @ghidraAddress NTSC-U/C: 0x003608d0
     * @ghidraAddress PAL: 0x003cede8
     */
    static UIPanel *New(DataArray *pData, const char *pszDir) {
        return new FreqPanel(pData, pszDir);
    }

    /**
     * Read the `freq_panel` section of the metagame configuration.
     *
     * @param pConfig The `metagame` section of the configuration.
     * @ghidraAddress NTSC-U/C: 0x0019d430
     * @ghidraAddress PAL: 0x001a5148
     */
    static void Init(DataArray *pConfig);

    /**
     * Drop a reference to the panel's file, forgetting the meshes and the animation on the last
     * reference.
     *
     * @ghidraAddress NTSC-U/C: 0x0019dd30
     * @ghidraAddress PAL: 0x001a5a48
     */
    void Unload() override;

    /**
     * Start the entry animation and the entry of the panel material animation.
     *
     * @param bForce Show the panel at once instead of animating.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0019e1b8
     * @ghidraAddress PAL: 0x001a5ed0
     */
    void Enter(bool bForce, float fTime) override;

    /**
     * Start the exit animation and the exit of the panel material animation.
     *
     * @param bForce Hide the panel at once instead of animating.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0019e230
     * @ghidraAddress PAL: 0x001a5f48
     */
    void Exit(bool bForce, float fTime) override;

    /**
     * Stretch the gizmo pyramid from the background mesh to the gizmo and draw it.
     *
     * @ghidraAddress NTSC-U/C: 0x0019dea0
     * @ghidraAddress PAL: 0x001a5bb8
     */
    void DrawGizmo() override;

    /**
     * Advance the panel to a time and move the panel animation to the panel's frame.
     *
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0019de18
     * @ghidraAddress PAL: 0x001a5b30
     */
    void Poll(float fTime) override;

    /**
     * Move the panel material animations to the front-end time and draw the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x0019e0d0
     * @ghidraAddress PAL: 0x001a5de8
     */
    void Draw() override;

    /**
     * Show or hide the panel's view and the gizmo pyramid.
     *
     * @param bShowing Whether the panel shows.
     * @ghidraAddress NTSC-U/C: 0x0019dda0
     * @ghidraAddress PAL: 0x001a5ab8
     */
    void SetShowing(bool bShowing) override;

    /**
     * Move the focus to a component and play the sound of the directional button that moved it.
     *
     * @param pComponent The component, or null.
     * @param nButton The controller button that moved the focus, or kPadNone.
     * @ghidraAddress NTSC-U/C: 0x0019e2a8
     * @ghidraAddress PAL: 0x001a5fc0
     */
    void SetFocus(UIComponent *pComponent, int nButton) override;

    /**
     * Find the meshes and animations of the panel once its file has loaded.
     *
     * The first load makes the panel animation, its copy of `panel_anim_name`, and the gizmo
     * pyramid, and hands each to the loader so they go when the panel unloads.
     *
     * @ghidraAddress NTSC-U/C: 0x0019d5a8
     * @ghidraAddress PAL: 0x001a52c0
     */
    void FinishLoad() override;

    /**
     * Show or hide the gizmo pyramid.
     *
     * @param bShowing Whether the pyramid shows.
     * @ghidraAddress NTSC-U/C: 0x0019dde0
     * @ghidraAddress PAL: 0x001a5af8
     */
    void ShowGizmoMesh(bool bShowing);

    /**
     * Measure the background mesh into mBgBox.
     *
     * @ghidraAddress NTSC-U/C: 0x0019dd70
     * @ghidraAddress PAL: 0x001a5a88
     */
    void UpdateBgBox();

    /**
     * The position of the gizmo, the `panel_gizmo_orig_pos` setting.
     *
     * @ghidraAddress NTSC-U/C: 0x004368d0
     */
    alignas(16) static Vector3 sGizmoOrigPos;

    /**
     * The points of the gizmo the pyramids reach, the `panel_gizmo_offset_1` to `_3` settings.
     *
     * @ghidraAddress NTSC-U/C: 0x004368e0
     */
    alignas(16) static Vector3 sGizmoOffsets[kNumGizmoOffsets];

    Rnd::Mesh *mGizmoMesh;        /*!< The panel's clone of `pyramid.mesh`, or null. */
    Rnd::Mesh *mBgMesh;           /*!< The background mesh `<name>_bg.mesh`, or null. */
    alignas(16) Box mBgBox;       /*!< The bounding box of mBgMesh in its own space. */
    int mGizmoOffsetIndex;        /*!< The entry of sGizmoOffsets the pyramid reaches. */
    Rnd::View *mMatEnterExitAnim; /*!< The shared animation `mat_2d_EE.anim`. */
    Rnd::View *mMatAlwaysAnim;    /*!< The shared animation `mat_2d_always.anim`. */
    Rnd::View *mPanelAnim;        /*!< The panel animation `<name>_f2EE.anim`, or null. */
    float mMatStart;              /*!< The time the material entry or exit started, or 0. */
    bool mMatEntering;            /*!< Whether the material animation enters rather than exits. */

private:
    /**
     * The transform animation each panel copies, the `panel_anim_name` setting.
     *
     * @ghidraAddress NTSC-U/C: 0x003af8c8
     */
    static const char *sPanelAnimName;

    /**
     * The frame the material entry starts at, the `panel_mat_anim_enter_start` setting.
     *
     * @ghidraAddress NTSC-U/C: 0x003af8b8
     */
    static float sMatEnterStart;

    /**
     * The frame the material entry stops at, the `panel_mat_anim_enter_stop` setting.
     *
     * @ghidraAddress NTSC-U/C: 0x003af8bc
     */
    static float sMatEnterStop;

    /**
     * The frame the material exit starts at, the `panel_mat_anim_exit_start` setting.
     *
     * @ghidraAddress NTSC-U/C: 0x003af8c0
     */
    static float sMatExitStart;

    /**
     * The frame the material exit stops at, the `panel_mat_anim_exit_stop` setting.
     *
     * @ghidraAddress NTSC-U/C: 0x003af8c4
     */
    static float sMatExitStop;
};

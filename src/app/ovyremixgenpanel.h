#pragma once

#include <list>
#include <vector>

#include "rnd/font.h"
#include "rnd/mat.h"
#include "rnd/mesh.h"
#include "rnd/object.h"
#include "rnd/text.h"
#include "rnd/transformable.h"

/**
 * List of labelled options on the remix head-up display, one of which a cursor selects.
 *
 * The RTTI records the class with no base. The object is 0x48 bytes with the vptr at `+0x44`. The
 * panel clones its meshes and texts from the loaded scene under a per-player name, and owns the
 * clones.
 */
class OvyRemixGenPanel {
public:
    /** One option: the mesh behind it and its label. */
    struct Option {
        Rnd::Mesh *mMesh; /*!< The mesh behind the option, which the cursor materials go on. */
        Rnd::Text *mText; /*!< The label. */
    };

    /** The indices of mFonts. */
    enum FontIndex {
        kFontNormal = 0,   /*!< The font of a label that is not lit. */
        kFontSelected = 1, /*!< The font of a lit label. */
        kNumFonts = 2,     /*!< The number of fonts. */
    };

    /** The value of mFlashTime when no blink runs. */
    static constexpr float kNoFlash = -1e9f;

    /**
     * Construct an empty panel.
     *
     * @ghidraAddress NTSC-U/C: 0x001b6a38
     * @ghidraAddress PAL: 0x001bf7d8
     */
    OvyRemixGenPanel();

    /**
     * Destroy the panel and the objects it cloned.
     *
     * @ghidraAddress NTSC-U/C: 0x001b6af8
     * @ghidraAddress PAL: 0x001bf898
     */
    virtual ~OvyRemixGenPanel();

    /**
     * Blink the cursor of the selected option for 179 ticks after Flash().
     *
     * @param fDelta The time since the last poll. The blink follows the song time instead.
     * @ghidraAddress NTSC-U/C: 0x001b75a0
     * @ghidraAddress PAL: 0x001c0340
     */
    virtual void Poll(float fDelta);

    /**
     * Draw the panel.
     *
     * @ghidraAddress NTSC-U/C: 0x001b7690
     * @ghidraAddress PAL: 0x001c0430
     */
    virtual void Draw();

    /**
     * Clone the panel's objects from the loaded scene and build its options.
     *
     * The objects are `<prefix>_bg.mesh`, `<prefix>_panel.mesh`, one `<prefix>_<nn>.mesh` and
     * `<prefix>_<nn>.txt` pair per option counting from 01, and `<prefix>_up.mesh`,
     * `<prefix>_down.mesh`, and `<prefix>_hr.mesh`. The materials and fonts are
     * `HUD<hud>r cursor_no.mat`, `HUD<hud>r cursor_hi.mat`, `HUD<hud>r selected.font`, and, unless
     * the head-up display is `s`, `HUD<hud>r freq<player> bg.mat`.
     *
     * @param chHud The letter of the head-up display layout.
     * @param pszPrefix The prefix of the scene objects.
     * @param nPlayer The player. The clones are named for it.
     * @param pParent The transform the panel moves with, or null.
     * @ghidraAddress NTSC-U/C: 0x001b6c40
     * @ghidraAddress PAL: 0x001bf9e0
     */
    void Load(char chHud, const char *pszPrefix, int nPlayer, Rnd::Transformable *pParent);

    /**
     * Clone one object of the loaded scene as `<name> pl <player>`, and own the clone.
     *
     * @param pszName The object.
     * @param nPlayer The player.
     * @return The clone, or null when the scene has no such object.
     * @ghidraAddress NTSC-U/C: 0x001b74e0
     * @ghidraAddress PAL: 0x001c0280
     */
    Rnd::Object *Clone(const char *pszName, int nPlayer);

    /**
     * Set the label of an option.
     *
     * @param nIndex The option.
     * @param pszText The label.
     * @ghidraAddress NTSC-U/C: 0x001b76b0
     * @ghidraAddress PAL: 0x001c0450
     */
    void SetText(int nIndex, const char *pszText);

    /**
     * Start blinking the cursor of the selected option, when there is one.
     *
     * @ghidraAddress NTSC-U/C: 0x001b76f0
     * @ghidraAddress PAL: 0x001c0490
     */
    void Flash();

    /**
     * Move the cursor to an option.
     *
     * @param nIndex The option, or a negative value for none.
     * @ghidraAddress NTSC-U/C: 0x001b7710
     * @ghidraAddress PAL: 0x001c04b0
     */
    void Select(int nIndex);

    /**
     * Draw the label of an option in the selected font or the normal one.
     *
     * @param nIndex The option.
     * @param bLit Whether to use the selected font.
     * @ghidraAddress NTSC-U/C: 0x001b7780
     * @ghidraAddress PAL: 0x001c0520
     */
    void SetLit(int nIndex, bool bLit);

    Rnd::Mesh *mBackground;           /*!< `<prefix>_bg.mesh`, the root of the drawing. */
    Rnd::Mesh *mPanel;                /*!< `<prefix>_panel.mesh`. */
    std::vector<Option> mOptions;     /*!< The options, in order. */
    std::list<Rnd::Object *> mClones; /*!< Every object the panel cloned and owns. */
    Rnd::Mat *mCursorOffMat;          /*!< The material of an option the cursor is not on. */
    Rnd::Mat *mCursorOnMat;           /*!< The material of the option the cursor is on. */
    Rnd::Font *mFonts[kNumFonts];     /*!< The label fonts, indexed by FontIndex. */
    Rnd::Mesh *mUpArrow;              /*!< `<prefix>_up.mesh`, or null. */
    Rnd::Mesh *mDownArrow;            /*!< `<prefix>_down.mesh`, or null. */
    Rnd::Mesh *mRule;                 /*!< `<prefix>_hr.mesh`, or null. */
    Option *mSelected;                /*!< The option the cursor is on, or null. */
    float mFlashTime;                 /*!< The song time Flash() ran at, or kNoFlash. */
};

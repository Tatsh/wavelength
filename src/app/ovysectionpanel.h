#pragma once

#include <vector>

#include "app/hideablepanel.h"
#include "os/string.h"
#include "rnd/drawable.h"
#include "rnd/mat.h"
#include "rnd/mesh.h"
#include "rnd/text.h"
#include "rnd/transformable.h"

/**
 * List of the sections of the song on the remix head-up display, with the current one lit.
 *
 * The RTTI records the class as deriving from HideablePanel. The object is 0x50 bytes. The
 * shared-screen layout `m` has room for one entry, which shows the label of the current section.
 */
class OvySectionPanel : public HideablePanel {
public:
    /** One section entry. */
    struct Section {
        String mLabel;    /*!< The label, kept for the shared-screen layout. */
        Rnd::Text *mText; /*!< The text the label shows in. */
        Rnd::Mesh *mMesh; /*!< The mesh behind the entry, which the cursor materials go on. */
    };

    /**
     * Construct a hidden list and find its scene objects.
     *
     * The objects are `<prefix>_sect.tnm`, `<prefix>_sect_bg.mesh`, `<prefix>_sect_panel.mesh`,
     * and the title `HUD<hud>r_sect_01.txt`. Every entry mesh `HUD<hud>r_sect_<nn>.mesh` from 02
     * on is hidden until SetSectionCount().
     *
     * @param chHud The letter of the head-up display layout.
     * @param prefix The prefix of the scene objects.
     * @param pParent The transform the list moves with.
     * @ghidraAddress NTSC-U/C: 0x001b8490
     * @ghidraAddress PAL: 0x001c1230
     */
    OvySectionPanel(char chHud, const String &prefix, Rnd::Transformable *pParent);

    /**
     * Destroy the list.
     *
     * @ghidraAddress NTSC-U/C: 0x001b87f0
     * @ghidraAddress PAL: 0x001c1590
     */
    ~OvySectionPanel() override;

    /**
     * Advance the slide.
     *
     * @ghidraAddress NTSC-U/C: 0x001b8930
     * @ghidraAddress PAL: 0x001c16d0
     */
    void Poll();

    /**
     * Draw the list while it is not hidden.
     *
     * @ghidraAddress NTSC-U/C: 0x001b8950
     * @ghidraAddress PAL: 0x001c16f0
     */
    void Draw();

    /**
     * Show one blank entry per section and hide the rest.
     *
     * @param nCount The number of sections.
     * @ghidraAddress NTSC-U/C: 0x001b8990
     * @ghidraAddress PAL: 0x001c1730
     */
    void SetSectionCount(int nCount);

    /**
     * Set the label of a section.
     *
     * @param nIndex The section.
     * @param pszLabel The label.
     * @ghidraAddress NTSC-U/C: 0x001b8de8
     * @ghidraAddress PAL: 0x001c1b88
     */
    void SetSectionLabel(int nIndex, const char *pszLabel);

    /**
     * Light the entry of a section, or show its label in the shared-screen layout.
     *
     * @param nIndex The section.
     * @ghidraAddress NTSC-U/C: 0x001b8e48
     * @ghidraAddress PAL: 0x001c1be8
     */
    void HighlightSection(int nIndex);

    std::vector<String> mUnused;    /*!< Constructed and destroyed only. */
    std::vector<Section> mSections; /*!< The entries, one per section. */
    Rnd::Mat *mCursorOffMat;        /*!< The material of an entry that is not lit. */
    Rnd::Mat *mCursorOnMat;         /*!< The material of the lit entry. */
    Rnd::Drawable *mBackground;     /*!< `<prefix>_sect_bg.mesh`, the root of the drawing. */
    Rnd::Transformable *mPanel;     /*!< `<prefix>_sect_panel.mesh`. */
    char mHud;                      /*!< The letter of the head-up display layout. */
};

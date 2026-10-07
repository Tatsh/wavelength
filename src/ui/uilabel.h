#pragma once

#include "os/prnstream.h"
#include "rnd/text.h"
#include "script/dataarray.h"
#include "ui/uicomponent.h"
#include "ui/uistyle.h"

/**
 * Component that shows a line of text.
 *
 * The RTTI records the class as deriving from UIComponent. The object is 0x18 bytes and its vtable
 * is at `0x003d2410`. The panel description provides the name at index 1, optionally the base of
 * the text object's name at index 2, and optionally a style at index 3. The text object is
 * `<panel>_<base>.txt`, with the name standing in for a missing base. The style selects the font of
 * each state.
 */
class UILabel : public UIComponent {
public:
    /**
     * Construct a label from its script description.
     *
     * @param pData The script description.
     * @param pszPanel The name of the panel the label belongs to.
     * @ghidraAddress NTSC-U/C: 0x00203e20
     * @ghidraAddress PAL: 0x0020cbd8
     */
    UILabel(DataArray *pData, const char *pszPanel);

    /**
     * Destroy the label.
     *
     * @ghidraAddress NTSC-U/C: 0x003773e8
     * @ghidraAddress PAL: 0x003e5b18
     */
    ~UILabel() override {
    }

    /**
     * Create a label from its script description.
     *
     * UIManager::Init() registers the routine for the component type `label_comp`.
     *
     * @param pData The script description.
     * @param pszPanel The name of the panel the label belongs to.
     * @return The new label.
     * @ghidraAddress NTSC-U/C: 0x00377398
     * @ghidraAddress PAL: 0x003e5ac8
     */
    static UIComponent *New(DataArray *pData, const char *pszPanel) {
        return new UILabel(pData, pszPanel);
    }

    /**
     * Show or hide the label and its text object.
     *
     * The text object must exist.
     *
     * @param bShowing Whether the label shows.
     * @ghidraAddress NTSC-U/C: 0x00203f80
     * @ghidraAddress PAL: 0x0020cd38
     */
    void SetShowing(bool bShowing) override;

    /**
     * Report the text of the text object.
     *
     * The text object must exist.
     *
     * @return The text as it was last set.
     * @ghidraAddress NTSC-U/C: 0x00203fb8
     * @ghidraAddress PAL: 0x0020cd70
     */
    const char *Text() const override;

    /**
     * Replace the text of the text object.
     *
     * The text object must exist.
     *
     * @param pszText The text.
     * @ghidraAddress NTSC-U/C: 0x00203fc8
     * @ghidraAddress PAL: 0x0020cd80
     */
    void SetText(const char *pszText) override;

    /**
     * Change the state and give the text object the font the style lists for it.
     *
     * The style must exist when the text object does.
     *
     * @param nState One of UIComponent::State.
     * @param bForce Apply the state even when it does not change.
     * @ghidraAddress NTSC-U/C: 0x00204060
     * @ghidraAddress PAL: 0x0020ce18
     */
    void SetState(int nState, bool bForce) override;

    /**
     * Write the name, the visibility, and the name of the text object.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x002040f0
     * @ghidraAddress PAL: 0x0020cea8
     */
    void Print(PrnStream &stream) override;

    /**
     * Replace the style and give the text object the font of the current state.
     *
     * @param pStyle The style.
     * @ghidraAddress NTSC-U/C: 0x00203ff8
     * @ghidraAddress PAL: 0x0020cdb0
     */
    void SetStyle(UIStyle *pStyle);

    Rnd::Text *mText; /*!< The text object, or null when the file has none of that name. */
    UIStyle *mStyle;  /*!< The style, or null when the description lists none. */
};

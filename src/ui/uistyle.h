#pragma once

#include "rnd/font.h"
#include "rnd/mat.h"
#include "script/dataarray.h"

/**
 * Material and font a component draws with in each of its states.
 *
 * The RTTI includes the class name. The object is 0x20 bytes, allocated with the global
 * `operator new`. UIManager builds one for each `style` entry of the front-end description and
 * finds it by mName. The entry's `normal`, `selected`, and `grey` arrays each list a material and
 * a font by object name.
 */
class UIStyle {
public:
    /**
     * Construct a style from its script description.
     *
     * Index 1 of the description is the name. A state whose array is absent keeps a null material
     * and font, and a name that resolves to no object of the expected class leaves a null pointer.
     *
     * @param pData The script description.
     * @ghidraAddress NTSC-U/C: 0x0020bc50
     * @ghidraAddress PAL: 0x00214a50
     */
    explicit UIStyle(DataArray *pData);

    /**
     * Destroy the style.
     *
     * @ghidraAddress NTSC-U/C: 0x00379620
     * @ghidraAddress PAL: 0x003e7d50
     */
    virtual ~UIStyle() {
    }

    /**
     * Report the material of a state.
     *
     * @param nState One of UIComponent::State. Any value other than the normal and selected states
     *               selects the disabled state.
     * @return The material, or null.
     * @ghidraAddress NTSC-U/C: 0x0020bf80
     * @ghidraAddress PAL: 0x00214d80
     */
    Rnd::Mat *GetMat(int nState) const;

    /**
     * Report the font of a state.
     *
     * @param nState One of UIComponent::State. Any value other than the normal and selected states
     *               selects the disabled state.
     * @return The font, or null.
     * @ghidraAddress NTSC-U/C: 0x0020bfa8
     * @ghidraAddress PAL: 0x00214da8
     */
    Rnd::Font *GetFont(int nState) const;

    /**
     * Set the material and the font of the normal state.
     *
     * @param pMat The material.
     * @param pFont The font.
     * @ghidraAddress NTSC-U/C: 0x0020bfd0
     * @ghidraAddress PAL: 0x00214dd0
     */
    void SetNormal(Rnd::Mat *pMat, Rnd::Font *pFont);

    /**
     * Set the material and the font of the selected state.
     *
     * @param pMat The material.
     * @param pFont The font.
     * @ghidraAddress NTSC-U/C: 0x0020bfe0
     * @ghidraAddress PAL: 0x00214de0
     */
    void SetSelected(Rnd::Mat *pMat, Rnd::Font *pFont);

    /**
     * Set the material and the font of the disabled state.
     *
     * @param pMat The material.
     * @param pFont The font.
     * @ghidraAddress NTSC-U/C: 0x0020bff0
     * @ghidraAddress PAL: 0x00214df0
     */
    void SetGrey(Rnd::Mat *pMat, Rnd::Font *pFont);

    /**
     * Create a style from its script description.
     *
     * UIManager::Init() registers the routine for the entry type `style`.
     *
     * @param pData The script description.
     * @return The new style.
     * @ghidraAddress NTSC-U/C: 0x0037b9a8
     * @ghidraAddress PAL: 0x003ea0d8
     */
    static UIStyle *New(DataArray *pData) {
        return new UIStyle(pData);
    }

    const char *mName;        /*!< The name, a symbol. */
    Rnd::Mat *mNormalMat;     /*!< The material of the normal state. */
    Rnd::Mat *mSelectedMat;   /*!< The material of the selected state. */
    Rnd::Mat *mGreyMat;       /*!< The material of the disabled state. */
    Rnd::Font *mNormalFont;   /*!< The font of the normal state. */
    Rnd::Font *mSelectedFont; /*!< The font of the selected state. */
    Rnd::Font *mGreyFont;     /*!< The font of the disabled state. */
};

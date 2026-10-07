#pragma once

/**
 * The effect sets a song applies to the mixer channels at given bars.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. Only the members GameLogic
 * uses are declared.
 */
class FreestyleFx {
public:
    /**
     * Prepare the effects for the song.
     *
     * @ghidraAddress NTSC-U/C: 0x0014fe70
     * @ghidraAddress PAL: 0x001517d0
     */
    void Activate();

    /**
     * Remove the effects from the channels.
     *
     * @ghidraAddress NTSC-U/C: 0x0014fe98
     * @ghidraAddress PAL: 0x001517f8
     */
    void Reset();

    /**
     * Apply one effect set to the channels.
     *
     * @param nSet The effect set.
     * @ghidraAddress NTSC-U/C: 0x00150270
     * @ghidraAddress PAL: 0x00151bd0
     */
    void ApplySet(int nSet);

    /**
     * Report the number of effect sets.
     *
     * @return The number of sets.
     * @ghidraAddress NTSC-U/C: 0x00150330
     * @ghidraAddress PAL: 0x00151c90
     */
    int GetNumSets() const;

    /**
     * Report the bar an effect set applies at.
     *
     * @param nSet The effect set.
     * @return The bar.
     * @ghidraAddress NTSC-U/C: 0x00150390
     * @ghidraAddress PAL: 0x00151cf0
     */
    int GetSetBar(int nSet) const;
};

#pragma once

#include <vector>

#include "game/stutter.h"
#include "os/binstream.h"

/**
 * The effect sets a song applies to the synthesiser channels at given bars.
 *
 * The RTTI includes the nested FXMgr::FX and FXMgr::FXSet. The class is not polymorphic. Each set
 * has one FX for each channel. While the manager is active, the current set drives the stutter
 * gate, the filter controllers, and the chorus controllers of every channel.
 */
class FXMgr {
public:
    /**
     * The effects of one channel in a set.
     *
     * The RTTI includes the nested name.
     */
    struct FX {
        int mStutter; /*!< Whether the stutter gate holds the channel. */
        int mFilter;  /*!< The value of the filter controllers. */
        int mChorus;  /*!< Whether the chorus controllers are on. */
    };

    /**
     * One set of effects and the bar it applies at.
     *
     * The RTTI includes the nested name.
     */
    struct FXSet {
        int mBar;            /*!< The bar the set applies at. */
        std::vector<FX> mFX; /*!< The effects of each channel. */
    };

    /**
     * Construct the manager with one set, at bar 0, of no effects.
     *
     * The name is inferred.
     *
     * @param nNumChannels The channels.
     * @param nChorusDepth The value of the second chorus controller while the chorus is on.
     * @ghidraAddress NTSC-U/C: 0x0014fcf0
     * @ghidraAddress PAL: 0x00151650
     */
    FXMgr(int nNumChannels, int nChorusDepth);

    /**
     * Release the sets.
     *
     * @ghidraAddress NTSC-U/C: 0x0014fd40
     * @ghidraAddress PAL: 0x001516a0
     */
    ~FXMgr();

    /**
     * Start applying the effects, beginning with the current set.
     *
     * @ghidraAddress NTSC-U/C: 0x0014fe70
     * @ghidraAddress PAL: 0x001517d0
     */
    void Activate();

    /**
     * Stop applying the effects and remove them from every channel.
     *
     * @ghidraAddress NTSC-U/C: 0x0014fe98
     * @ghidraAddress PAL: 0x001517f8
     */
    void Reset();

    /**
     * Append a set of no effects.
     *
     * The name is inferred.
     *
     * @param nBar The bar the set applies at.
     * @ghidraAddress NTSC-U/C: 0x0014ff18
     * @ghidraAddress PAL: 0x00151878
     */
    void AddSet(int nBar);

    /**
     * Make a set current, and apply it to the channels while the manager is active.
     *
     * @param nSet The set.
     * @ghidraAddress NTSC-U/C: 0x00150270
     * @ghidraAddress PAL: 0x00151bd0
     */
    void ApplySet(int nSet);

    /**
     * Report the number of sets.
     *
     * @return The number of sets.
     * @ghidraAddress NTSC-U/C: 0x00150330
     * @ghidraAddress PAL: 0x00151c90
     */
    int GetNumSets() const;

    /**
     * Report the current set.
     *
     * The name is inferred.
     *
     * @return The set.
     * @ghidraAddress NTSC-U/C: 0x00150350
     * @ghidraAddress PAL: 0x00151cb0
     */
    int GetCurrentSet() const;

    /**
     * Copy the effects of one set to another.
     *
     * The name is inferred.
     *
     * @param nFrom The set to copy.
     * @param nTo The set to replace.
     * @ghidraAddress NTSC-U/C: 0x00150358
     * @ghidraAddress PAL: 0x00151cb8
     */
    void CopySet(int nFrom, int nTo);

    /**
     * Report the bar a set applies at.
     *
     * @param nSet The set.
     * @return The bar.
     * @ghidraAddress NTSC-U/C: 0x00150390
     * @ghidraAddress PAL: 0x00151cf0
     */
    int GetSetBar(int nSet) const;

    /**
     * Set the stutter of a channel in a set, and apply it at once when the set is current.
     *
     * The name is inferred.
     *
     * @param nSet The set.
     * @param nChannel The channel.
     * @param nStutter Whether the stutter gate holds the channel.
     * @ghidraAddress NTSC-U/C: 0x001503a8
     * @ghidraAddress PAL: 0x00151d08
     */
    void SetStutter(int nSet, int nChannel, int nStutter);

    /**
     * Set the filter of a channel in a set, and apply it at once when the set is current.
     *
     * The name is inferred.
     *
     * @param nSet The set.
     * @param nChannel The channel.
     * @param nFilter The value of the filter controllers.
     * @ghidraAddress NTSC-U/C: 0x00150408
     * @ghidraAddress PAL: 0x00151d68
     */
    void SetFilter(int nSet, int nChannel, int nFilter);

    /**
     * Set the chorus of a channel in a set, and apply it at once when the set is current.
     *
     * The name is inferred.
     *
     * @param nSet The set.
     * @param nChannel The channel.
     * @param nChorus Whether the chorus controllers are on.
     * @ghidraAddress NTSC-U/C: 0x00150468
     * @ghidraAddress PAL: 0x00151dc8
     */
    void SetChorus(int nSet, int nChannel, int nChorus);

    /**
     * Report the stutter of a channel in a set.
     *
     * The name is inferred.
     *
     * @param nSet The set.
     * @param nChannel The channel.
     * @return Whether the stutter gate holds the channel.
     * @ghidraAddress NTSC-U/C: 0x001504c8
     * @ghidraAddress PAL: 0x00151e28
     */
    int GetStutter(int nSet, int nChannel) const;

    /**
     * Report the filter of a channel in a set.
     *
     * The name is inferred.
     *
     * @param nSet The set.
     * @param nChannel The channel.
     * @return The value of the filter controllers.
     * @ghidraAddress NTSC-U/C: 0x001504f0
     * @ghidraAddress PAL: 0x00151e50
     */
    int GetFilter(int nSet, int nChannel) const;

    /**
     * Report the chorus of a channel in a set.
     *
     * The name is inferred.
     *
     * @param nSet The set.
     * @param nChannel The channel.
     * @return Whether the chorus controllers are on.
     * @ghidraAddress NTSC-U/C: 0x00150518
     * @ghidraAddress PAL: 0x00151e78
     */
    int GetChorus(int nSet, int nChannel) const;

    /**
     * Replace the sets with the sets Save() writes, and make the first set current.
     *
     * Each effect reads back as 1 when its byte is not zero.
     *
     * @param stream The stream to read from.
     * @ghidraAddress NTSC-U/C: 0x00150540
     */
    void Load(BinStream &stream);

    /**
     * Write the number of sets in one byte, then for each set the bar in four bytes of the
     * console's byte order and one byte for each effect of each channel.
     *
     * @param stream The stream to write to.
     * @ghidraAddress NTSC-U/C: 0x00150a80
     */
    void Save(BinStream &stream) const;

private:
    /**
     * Send the filter controllers of a channel to the synthesiser.
     *
     * The name is inferred.
     *
     * @param nChannel The channel.
     * @param nFilter The controller value.
     * @ghidraAddress NTSC-U/C: 0x00150c38
     * @ghidraAddress PAL: 0x00152528
     */
    void ApplyFilter(int nChannel, int nFilter);

    /**
     * Hold or release a channel in the stutter gate.
     *
     * The name is inferred.
     *
     * @param nChannel The channel.
     * @param nStutter Whether the gate holds the channel.
     * @ghidraAddress NTSC-U/C: 0x00150ce0
     * @ghidraAddress PAL: 0x001525d0
     */
    void ApplyStutter(int nChannel, int nStutter);

    /**
     * Send the chorus controllers of a channel to the synthesiser.
     *
     * The name is inferred.
     *
     * @param nChannel The channel.
     * @param nChorus Whether the chorus is on.
     * @ghidraAddress NTSC-U/C: 0x00150d00
     * @ghidraAddress PAL: 0x001525f0
     */
    void ApplyChorus(int nChannel, int nChorus);

    int mNumChannels;         /*!< The channels. */
    int mChorusDepth;         /*!< The second chorus controller value while the chorus is on. */
    int mActive;              /*!< Whether the effects are applied. */
    int mCurrentSet;          /*!< The current set. */
    std::vector<FXSet> mSets; /*!< The sets, in the order they were added. */
    Stutter mStutter;         /*!< The stutter gate of the channels. */
};

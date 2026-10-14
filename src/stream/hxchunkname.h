#pragma once

#include <cstring>

class HxStream;

/**
 * Four-character name of a RIFF or Standard MIDI File chunk.
 *
 * The class is not polymorphic and has no RTTI. Its name is inferred. The fifteen known names
 * below are objects rather than literals. The static initialiser of the chunk translation unit at
 * `0x00145608` copies each four-character literal into its object at start-up with one unaligned
 * word copy (the inline constructor below). Every comparison runs strncmp() over four characters
 * against the object's address. The objects sit eight bytes apart from `0x00673a80`.
 *
 * The one member is public, because every reader compares or copies the characters directly and
 * the image exposes no accessor.
 */
struct HxChunkName {
    /**
     * Copy the first four characters of a literal.
     *
     * Inline. The static initialiser expands it for each of the fifteen names.
     *
     * @param pszText The name. Only its first four characters are copied.
     */
    explicit HxChunkName(const char *pszText) {
        memcpy(mText, pszText, kLength);
    }

    /** Number of characters in a chunk name. */
    static constexpr int kLength = 4;

    char mText[kLength]; /*!< The four characters, with no terminator. +0x00 */
};

/**
 * Compare two chunk names over their four characters.
 *
 * Inline. Every expansion is one strncmp() with a length of four.
 *
 * @param left The first name.
 * @param right The second name.
 * @return Whether the names match.
 */
inline bool operator==(const HxChunkName &left, const HxChunkName &right) {
    return strncmp(left.mText, right.mText, HxChunkName::kLength) == 0;
}

/**
 * Compare two chunk names over their four characters.
 *
 * @param left The first name.
 * @param right The second name.
 * @return Whether the names differ.
 */
inline bool operator!=(const HxChunkName &left, const HxChunkName &right) {
    return strncmp(left.mText, right.mText, HxChunkName::kLength) != 0;
}

/**
 * Read a chunk name as four raw characters, with no byte swapping.
 *
 * @param stream The stream to read from.
 * @param name The name to fill.
 * @return The stream.
 */
HxStream &operator>>(HxStream &stream, HxChunkName &name);

/**
 * `LIST`, the name of a list chunk.
 */
extern HxChunkName kListChunkID;

/**
 * `RIFF`, the name of a top-level chunk.
 */
extern HxChunkName kRiffChunkID;

/**
 * `MIDI`.
 */
extern HxChunkName g_midiChunkName;

/**
 * `MThd`, a Standard MIDI File header chunk.
 */
extern HxChunkName kMidiHeaderChunkID;

/**
 * `MTrk`, a Standard MIDI File track chunk.
 */
extern HxChunkName kMidiTrackChunkID;

/**
 * `WAVE`.
 */
extern HxChunkName g_waveChunkName;

/**
 * `fmt `.
 */
extern HxChunkName g_fmtChunkName;

/**
 * `data`.
 */
extern HxChunkName g_dataChunkName;

/**
 * `fact`.
 */
extern HxChunkName g_factChunkName;

/**
 * `inst`.
 */
extern HxChunkName g_instChunkName;

/**
 * `smpl`.
 */
extern HxChunkName g_smplChunkName;

/**
 * `cue `.
 */
extern HxChunkName g_cueChunkName;

/**
 * `labl`.
 */
extern HxChunkName g_lablChunkName;

/**
 * `ltxt`.
 */
extern HxChunkName g_ltxtChunkName;

/**
 * `adtl`.
 */
extern HxChunkName g_adtlChunkName;

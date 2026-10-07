#pragma once

#include "os/binstream.h"
#include "os/datetime.h"
#include "os/prnstream.h"

/** Bytes of RemixInfo::mName, the terminator included. */
constexpr int kRemixInfoNameSize = 30;

/** Bytes of RemixInfo::mSong, the terminator included. */
constexpr int kRemixInfoSongSize = 30;

/** The number of creators a remix records. */
constexpr int kRemixInfoCreatorCount = 4;

/** Bytes of each name in RemixInfo::mCreators, the terminator included. */
constexpr int kRemixInfoCreatorSize = 16;

/**
 * Description of a saved remix. A remix file stores one ahead of the remix.
 *
 * The class is not polymorphic. The name comes from the type information the standard library
 * containers of the class record. The object is 0xa0 bytes. Members its callers here do not use
 * are reserved.
 */
class RemixInfo {
public:
    /** Construct a cleared description. */
    RemixInfo() {
        Reset();
    }

    /**
     * Clear every field.
     *
     * @ghidraAddress NTSC-U/C: 0x0027e8b0
     * @ghidraAddress PAL: 0x002881c8
     */
    void Reset();

    /**
     * Report the bytes a description occupies in a stream.
     *
     * @return The size, a version byte more than the object.
     * @ghidraAddress NTSC-U/C: 0x0027ea98
     * @ghidraAddress PAL: 0x002883b0
     */
    static int WireSize();

    /**
     * Show the description in the labels `genre`, `date`, `rating`, `creator_1` to `creator_4`,
     * and `read_only` of a panel, and the band picture of the song in the panel.
     *
     * @param pszPanel The panel, a SongPicPanel.
     * @ghidraAddress NTSC-U/C: 0x0019a6d8
     * @ghidraAddress PAL: 0x001a1c18
     */
    void ShowDetails(const char *pszPanel) const;

    /** Values of mSource. */
    enum Source {
        kSourceNone = 0,    /*!< No source. */
        kSourceNet = 1,     /*!< Received over the network. */
        kSourceCd = 2,      /*!< On the disc. */
        kSourceMemcard = 3, /*!< On a memory card. */
    };

    int mReserved00[2];             // +0x00, not yet recovered.
    char mName[kRemixInfoNameSize]; /*!< The name the remix is saved under. +0x08 */
    char mSong[kRemixInfoSongSize]; /*!< The song the remix is made from, a symbol. +0x26 */
    int mSource;                    /*!< One of Source. +0x44 */
    int mDataSize;                  /*!< The size of the remix that follows in bytes. +0x48 */
    char mCreators[kRemixInfoCreatorCount][kRemixInfoCreatorSize]; /*!< The creators. +0x4c */
    DateTime mDate;        /*!< The six-byte date the inline constructor clears. +0x8c */
    char mReserved92[2];   // +0x92, not yet recovered.
    int mSkillLevel;       /*!< The skill level the remix is rated at. +0x94 */
    signed char mPlayable; /*!< Non-zero when the remix can be played. +0x98 */
    signed char mReadOnly; /*!< Non-zero when the remix may not be saved over. +0x99 */
    char mReserved9A[2];   // +0x9a, not yet recovered.
    float mTempo;          /*!< The tempo in beats per minute. +0x9c */
};

/**
 * Write a description as its name, key, song, size, date, skill, flags, tempo, and source.
 *
 * @param stream The stream to write to.
 * @param info The description.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x0027e8d0
 * @ghidraAddress PAL: 0x002881e8
 */
PrnStream &operator<<(PrnStream &stream, const RemixInfo &info);

/**
 * Write a description to a stream, after a version byte.
 *
 * @param stream The stream.
 * @param info The description.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x0027eaa0
 */
BinStream &operator<<(BinStream &stream, const RemixInfo &info);

/**
 * Read a description from a stream.
 *
 * @param stream The stream.
 * @param info Receives the description.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x0027eb10
 */
BinStream &operator>>(BinStream &stream, RemixInfo &info);

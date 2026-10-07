#pragma once

/**
 * How a program plays one sample over a key range, as the bank file records it.
 *
 * The class has no RTTI. Its name is inferred from the `iSd` the dump prints, and the member names
 * are the ones the dump prints. The envelope fields store the SPU2 ADSR rates and modes. The
 * serialised record is a version word followed by the fields in a fixed order with no padding.
 * Version 1 stops after #mSampleIndex, version 2 adds #mBus and #mBusMode, and version 3 adds
 * #mSurround.
 */
class SampleDesc {
public:
    /**
     * Construct a descriptor with the defaults Init() sets.
     *
     * @ghidraAddress NTSC-U/C: 0x00001678
     * @ghidraAddress PAL: 0x00001678
     */
    SampleDesc();

    /**
     * Reset every field to its default.
     *
     * @ghidraAddress NTSC-U/C: 0x000016a0
     * @ghidraAddress PAL: 0x000016a0
     */
    void Init();

    /**
     * Write the descriptor as a serialised record of version 3.
     *
     * @param data Receives the record, at any alignment.
     * @return The bytes written.
     * @ghidraAddress NTSC-U/C: 0x00001704
     * @ghidraAddress PAL: 0x00001704
     */
    int Pack(unsigned char *data) const;

    /**
     * Report the size of a serialised record of version 3.
     *
     * @return The size in bytes.
     * @ghidraAddress NTSC-U/C: 0x0000183c
     * @ghidraAddress PAL: 0x0000183c
     */
    int PackedSize() const;

    /**
     * Read the descriptor from a serialised record of version 1, 2, or 3.
     *
     * Fields a version does not record retain their defaults. A record of an unknown version does
     * not change the descriptor.
     *
     * @param data The record, at any alignment.
     * @return The bytes consumed, four for an unknown version.
     * @ghidraAddress NTSC-U/C: 0x00001844
     * @ghidraAddress PAL: 0x00001844
     */
    int Unpack(const unsigned char *data);

    /**
     * Check the descriptor after Unpack(). The body is empty.
     *
     * @ghidraAddress NTSC-U/C: 0x00001670
     * @ghidraAddress PAL: 0x00001670
     */
    void Validate();

    /**
     * Print every field to the console.
     *
     * @ghidraAddress NTSC-U/C: 0x00001cd4
     * @ghidraAddress PAL: 0x00001cd4
     */
    void Dump() const;

    unsigned char mLowKeymap;    /*!< Lowest note the descriptor plays. */
    unsigned char mHighKeymap;   /*!< Highest note the descriptor plays. */
    unsigned char mBaseKey;      /*!< Note at which the sample plays at its sample rate. */
    signed char mTranspose;      /*!< Transposition in semitones. */
    signed char mFineTranspose;  /*!< Transposition in cents. */
    unsigned char mAttackRate;   /*!< ADSR attack rate. */
    unsigned short mAttackMode;  /*!< Nonzero for an exponential attack. */
    unsigned char mDecayRate;    /*!< ADSR decay rate. */
    unsigned char mSusLevel;     /*!< ADSR sustain level. */
    unsigned char mSusRate;      /*!< ADSR sustain rate. */
    unsigned short mSusMode;     /*!< ADSR sustain mode, zero to three. */
    unsigned char mReleaseRate;  /*!< ADSR release rate. */
    unsigned short mReleaseMode; /*!< Nonzero for an exponential release. */
    unsigned char mVolume;       /*!< Volume, an index into the volume curve. */
    unsigned char mPan;          /*!< Pan offset, where 64 is the centre. */
    unsigned short mSampleIndex; /*!< Index of the sample in its bank. */
    unsigned short mBus;         /*!< Core the voice prefers when the channel defers to it. */
    unsigned short mBusMode;     /*!< Dry and reverb mix when the channel defers to it. */
    signed char mSurround;       /*!< Nonzero to invert one side for the surround decoder. */
};

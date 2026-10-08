#pragma once

/**
 * How one sample plays across a key range of an instrument, shared with the synthesiser on the
 * IOP.
 *
 * The class has no RTTI, and its name is inferred from its source file and its assert text. The
 * record is 0x1c bytes, laid out as the descriptor of the IOP synthesiser. Its stream form is a
 * version word followed by the fields, 0x1d bytes in all.
 */
class SampleDescription {
public:
    /**
     * Create a description and reset it.
     *
     * @ghidraAddress 0x1002aad5
     */
    SampleDescription();

    /**
     * Check the fields after a read. The body is empty.
     *
     * @ghidraAddress 0x1002aad0
     */
    void Validate();

    /**
     * Reset every field to its default.
     *
     * @ghidraAddress 0x1002aaef
     */
    void Init();

    /**
     * Write the description in its stream form, as version 3.
     *
     * @param data Receives StreamSize() bytes.
     * @return The number of bytes written.
     * @ghidraAddress 0x1002ab84
     */
    int ToStream(unsigned char *data) const;

    /**
     * Report the size of the stream form.
     *
     * @return The size in bytes.
     * @ghidraAddress 0x1002ae5e
     */
    static int StreamSize();

    /**
     * Read the description from its stream form. Versions 1 to 3 are accepted. Version 1 has no
     * bus, bus mode, or surround, and version 2 has no surround.
     *
     * @param data The stream form.
     * @return The number of bytes read.
     * @ghidraAddress 0x1002ae68
     */
    int FromStream(const unsigned char *data);

    /**
     * Print the fields to standard output.
     *
     * @ghidraAddress 0x1002b606
     */
    void Dump() const;

    unsigned char mLowKeymap;    /*!< Lowest note the description plays. */
    unsigned char mHighKeymap;   /*!< Highest note the description plays. */
    unsigned char mBaseKey;      /*!< Note at which the sample plays at its sample rate. */
    signed char mTranspose;      /*!< Transposition in semitones. */
    signed char mFineTranspose;  /*!< Transposition in cents. */
    unsigned char mAttackRate;   /*!< ADSR attack rate. */
    unsigned short mAttackMode;  /*!< Nonzero for an exponential attack. */
    unsigned char mDecayRate;    /*!< ADSR decay rate. */
    unsigned char mSusLevel;     /*!< ADSR sustain level. */
    unsigned char mSusRate;      /*!< ADSR sustain rate. */
    unsigned short mSusMode;     /*!< ADSR sustain mode. */
    unsigned char mReleaseRate;  /*!< ADSR release rate. */
    unsigned short mReleaseMode; /*!< Nonzero for an exponential release. */
    unsigned char mVolume;       /*!< Volume, an index into the volume curve. */
    unsigned char mPan;          /*!< Pan offset, where 64 is the centre. */
    unsigned short mSampleIndex; /*!< Index of the sample in its bank. */
    unsigned short mBus;         /*!< Core the voice prefers. */
    unsigned short mBusMode;     /*!< Dry and reverb mix. */
    signed char mSurround;       /*!< Nonzero to invert one side for the surround decoder. */
};

#pragma once

/**
 * Playback parameters of one waveform of a bank, shared with the synthesiser on the IOP.
 *
 * The class has no RTTI, and its name is inferred from its source file. The record is 0x10 bytes.
 * Its stream form is a version word followed by the fields, 0x12 bytes in all.
 */
class Sample {
public:
    /**
     * Create a sample and reset it.
     *
     * @ghidraAddress 0x1002a820
     */
    Sample();

    /**
     * Reset every field to zero.
     *
     * @ghidraAddress 0x1002a83a
     */
    void Init();

    /**
     * Read the sample from its stream form. Only version 1 is accepted.
     *
     * @param data The stream form.
     * @return The number of bytes read.
     * @ghidraAddress 0x1002a865
     */
    int FromStream(const unsigned char *data);

    /**
     * Write the sample in its stream form, as version 1.
     *
     * @param data Receives StreamSize() bytes.
     * @return The number of bytes written.
     * @ghidraAddress 0x1002a973
     */
    int ToStream(unsigned char *data) const;

    /**
     * Report the size of the stream form.
     *
     * @return The size in bytes.
     * @ghidraAddress 0x1002aa33
     */
    static int StreamSize();

    /**
     * Print the fields to the diagnostic stream.
     *
     * @ghidraAddress 0x1002aa3d
     */
    void Dump() const;

    unsigned short mSampleRate; /*!< Rate in hertz at which the waveform plays at its base key. */
    int mLoopStart;             /*!< Start of the loop. */
    int mLoopStop;              /*!< End of the loop, or zero for a one-shot waveform. */
    int mSampleStart;           /*!< Offset of the waveform in the audio data file. */
};

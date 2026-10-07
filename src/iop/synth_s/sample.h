#pragma once

/**
 * One waveform of a bank, as the bank file records it.
 *
 * The class has no RTTI, and its name is inferred from its role. Offsets are bytes from the start
 * of the bank's area in SPU2 memory. The serialised record is a version word followed by the
 * fields in member order, with no padding.
 */
class Sample {
public:
    /**
     * Construct a sample with the defaults Init() sets.
     *
     * @ghidraAddress NTSC-U/C: 0x000014f0
     * @ghidraAddress PAL: 0x000014f0
     */
    Sample();

    /**
     * Reset every field to zero.
     *
     * @ghidraAddress NTSC-U/C: 0x00001518
     * @ghidraAddress PAL: 0x00001518
     */
    void Init();

    /**
     * Read the sample from a serialised record.
     *
     * A record of an unknown version does not change the sample.
     *
     * @param data The record, at any alignment.
     * @return The bytes consumed, four for an unknown version.
     * @ghidraAddress NTSC-U/C: 0x0000152c
     * @ghidraAddress PAL: 0x0000152c
     */
    int Unpack(const unsigned char *data);

    /**
     * Write the sample as a serialised record of the current version.
     *
     * @param data Receives the record, at any alignment.
     * @return The bytes written.
     * @ghidraAddress NTSC-U/C: 0x000015dc
     * @ghidraAddress PAL: 0x000015dc
     */
    int Pack(unsigned char *data) const;

    /**
     * Report the size of a serialised record of the current version.
     *
     * @return The size in bytes.
     * @ghidraAddress NTSC-U/C: 0x00001654
     * @ghidraAddress PAL: 0x00001654
     */
    int PackedSize() const;

    /**
     * Print the sample to the console. The body is empty.
     *
     * @ghidraAddress NTSC-U/C: 0x0000165c
     * @ghidraAddress PAL: 0x0000165c
     */
    void Dump() const;

    unsigned short mSampleRate; /*!< Rate in hertz at which the waveform plays at its base key. */
    unsigned int mLoopStart;    /*!< Start of the loop. */
    unsigned int mLoopEnd;      /*!< End of the loop, or zero for a one-shot waveform. */
    unsigned int mOffset;       /*!< Start of the waveform. */
};

#pragma once

/**
 * One program (instrument) of a bank, as the bank file records it.
 *
 * The class has no RTTI, and its name is inferred from its role. A program has a run of sample
 * descriptors, one per key range. The serialised record is a version word followed by the fields
 * in member order, with no padding.
 */
class Program {
public:
    /**
     * Construct a program with the defaults Init() sets.
     *
     * @ghidraAddress NTSC-U/C: 0x00000880
     * @ghidraAddress PAL: 0x00000880
     */
    Program();

    /**
     * Reset every field to its default.
     *
     * @ghidraAddress NTSC-U/C: 0x000008a8
     * @ghidraAddress PAL: 0x000008a8
     */
    void Init();

    /**
     * Read the program from a serialised record.
     *
     * A record of an unknown version does not change the program.
     *
     * @param data The record, at any alignment.
     * @return The bytes consumed, four for an unknown version.
     * @ghidraAddress NTSC-U/C: 0x000008cc
     * @ghidraAddress PAL: 0x000008cc
     */
    int Unpack(const unsigned char *data);

    /**
     * Write the program as a serialised record of the current version.
     *
     * @param data Receives the record, at any alignment.
     * @return The bytes written.
     * @ghidraAddress NTSC-U/C: 0x00000980
     * @ghidraAddress PAL: 0x00000980
     */
    int Pack(unsigned char *data) const;

    /**
     * Report the size of a serialised record of the current version.
     *
     * @return The size in bytes.
     * @ghidraAddress NTSC-U/C: 0x000009fc
     * @ghidraAddress PAL: 0x000009fc
     */
    int PackedSize() const;

    unsigned short mProgram;        /*!< Program number a program change requests. */
    unsigned char mPan;             /*!< Pan offset, where 64 is the centre. */
    unsigned char mVolume;          /*!< Volume, an index into the volume curve. */
    signed char mTranspose;         /*!< Transposition in semitones. */
    signed char mFineTranspose;     /*!< Transposition in cents. */
    unsigned short mNumSampleDescs; /*!< Sample descriptors in the program's run. */
};

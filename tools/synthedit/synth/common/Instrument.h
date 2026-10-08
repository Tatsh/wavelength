#pragma once

/**
 * Program settings of one instrument of a bank, shared with the synthesiser on the IOP.
 *
 * The class has no RTTI, and its name is inferred from its source file. The record is 8 bytes,
 * laid out as the program of the IOP synthesiser. Its stream form is a version word followed by
 * the fields, 0xc bytes in all.
 */
class Instrument {
public:
    /**
     * Create an instrument and reset it.
     *
     * @ghidraAddress 0x1002b7f0
     */
    Instrument();

    /**
     * Reset every field to its default.
     *
     * @ghidraAddress 0x1002b80a
     */
    void Init();

    /**
     * Read the instrument from its stream form. Only version 1 is accepted.
     *
     * @param data The stream form.
     * @return The number of bytes read.
     * @ghidraAddress 0x1002b83c
     */
    int FromStream(const unsigned char *data);

    /**
     * Write the instrument in its stream form, as version 1.
     *
     * @param data Receives StreamSize() bytes.
     * @return The number of bytes written.
     * @ghidraAddress 0x1002b98c
     */
    int ToStream(unsigned char *data) const;

    /**
     * Report the size of the stream form.
     *
     * @return The size in bytes.
     * @ghidraAddress 0x1002ba8e
     */
    static int StreamSize();

    unsigned short mProgram;        /*!< Program number a program change requests. */
    unsigned char mPan;             /*!< Pan offset, where 64 is the centre. */
    unsigned char mVolume;          /*!< Volume, an index into the volume curve. */
    signed char mTranspose;         /*!< Transposition in semitones. */
    signed char mFineTranspose;     /*!< Transposition in cents. */
    unsigned short mNumSampleDescs; /*!< Sample descriptions of the instrument. */
};

#pragma once

/**
 * Header of a bank, shared with the synthesiser on the IOP.
 *
 * The class has no RTTI, and its name is inferred from its source file. The record is 6 bytes,
 * laid out as the bank header of the IOP synthesiser. Its stream form is a version word followed
 * by the fields, 9 bytes in all.
 */
class Bank {
public:
    /**
     * Create a bank header and reset it.
     *
     * @ghidraAddress 0x1002baa0
     */
    Bank();

    /**
     * Reset every field to its default.
     *
     * @ghidraAddress 0x1002baba
     */
    void Init();

    /**
     * Read the header from its stream form. Only version 1 is accepted.
     *
     * @param data The stream form.
     * @return The number of bytes read.
     * @ghidraAddress 0x1002bad7
     */
    int FromStream(const unsigned char *data);

    /**
     * Write the header in its stream form, as version 1.
     *
     * @param data Receives StreamSize() bytes.
     * @return The number of bytes written.
     * @ghidraAddress 0x1002bbaa
     */
    int ToStream(unsigned char *data) const;

    /**
     * Report the size of the stream form.
     *
     * @return The size in bytes.
     * @ghidraAddress 0x1002bc49
     */
    static int StreamSize();

    unsigned short mId;             /*!< Bank identifier a bank select requests. */
    unsigned char mVolume;          /*!< Bank volume, an index into the volume curve. */
    unsigned short mNumInstruments; /*!< Instruments the bank defines. */
};

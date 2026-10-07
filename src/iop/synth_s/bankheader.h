#pragma once

/**
 * Identity and mix settings of an instrument bank, as the bank file records them.
 *
 * The class has no RTTI, and its name is inferred from its role. The serialised record is a
 * version word followed by the fields in member order, with no padding.
 */
class BankHeader {
public:
    /**
     * Construct a header with the defaults Init() sets.
     *
     * @ghidraAddress NTSC-U/C: 0x00000000
     * @ghidraAddress PAL: 0x00000000
     */
    BankHeader();

    /**
     * Reset every field to its default.
     *
     * @ghidraAddress NTSC-U/C: 0x00000028
     * @ghidraAddress PAL: 0x00000028
     */
    void Init();

    /**
     * Read the header from a serialised record.
     *
     * A record of an unknown version does not change the header.
     *
     * @param data The record, at any alignment.
     * @return The bytes consumed, four for an unknown version.
     * @ghidraAddress NTSC-U/C: 0x0000003c
     * @ghidraAddress PAL: 0x0000003c
     */
    int Unpack(const unsigned char *data);

    /**
     * Write the header as a serialised record of the current version.
     *
     * @param data Receives the record, at any alignment.
     * @return The bytes written.
     * @ghidraAddress NTSC-U/C: 0x000000cc
     * @ghidraAddress PAL: 0x000000cc
     */
    int Pack(unsigned char *data) const;

    /**
     * Report the size of a serialised record of the current version.
     *
     * @return The size in bytes.
     * @ghidraAddress NTSC-U/C: 0x00000124
     * @ghidraAddress PAL: 0x00000124
     */
    int PackedSize() const;

    unsigned short mId;          /*!< Bank identifier a bank select requests. */
    unsigned char mVolume;       /*!< Bank volume, an index into the volume curve. */
    unsigned short mNumPrograms; /*!< Programs the bank defines. */
};

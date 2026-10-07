#pragma once

#include <map>

/**
 * Text output stream.
 *
 * The RTTI includes the class name. One data word precedes the vptr at `+0x04`. The word is not
 * yet identified and is not declared. The vtable runs the type function, the destructor, and
 * Print().
 *
 * Every insertion operator returns the stream. An insertion chain therefore writes its pieces in
 * order through Print().
 */
class PrnStream {
public:
    /**
     * Release the stream.
     *
     * @ghidraAddress NTSC-U/C: 0x00339ea0
     * @ghidraAddress PAL: 0x003a7450
     */
    virtual ~PrnStream();

    /**
     * Write text to the stream's destination.
     *
     * The member is pure in this class.
     *
     * @param pszText The text, written unchanged.
     */
    virtual void Print(const char *pszText) = 0;

    /**
     * Format text into the shared format buffer and write the result through Print().
     *
     * @param pszFormat The `printf` format.
     * @ghidraAddress NTSC-U/C: 0x0029e1b0
     * @ghidraAddress PAL: 0x002a7e78
     */
    void Printf(const char *pszFormat, ...);

    /**
     * Write text.
     *
     * @param pszText The text, written unchanged.
     * @return The stream.
     * @ghidraAddress NTSC-U/C: 0x0029e410
     * @ghidraAddress PAL: 0x002a80d8
     */
    PrnStream &operator<<(const char *pszText);

    /**
     * Write a character.
     *
     * @param ch The character.
     * @return The stream.
     * @ghidraAddress NTSC-U/C: 0x0029e230
     * @ghidraAddress PAL: 0x002a7ef8
     */
    PrnStream &operator<<(char ch);

    /**
     * Write an integer in decimal.
     *
     * @param nValue The value.
     * @return The stream.
     * @ghidraAddress NTSC-U/C: 0x0029e2a0
     * @ghidraAddress PAL: 0x002a7f68
     */
    PrnStream &operator<<(int nValue);

    /**
     * Write a byte as an unsigned integer in decimal.
     *
     * @param nValue The value.
     * @return The stream.
     * @ghidraAddress NTSC-U/C: 0x0029e2e0
     * @ghidraAddress PAL: 0x002a7fa8
     */
    PrnStream &operator<<(unsigned char nValue);

    /**
     * Write an unsigned integer in decimal.
     *
     * @param nValue The value.
     * @return The stream.
     * @ghidraAddress NTSC-U/C: 0x0029e350
     * @ghidraAddress PAL: 0x002a8018
     */
    PrnStream &operator<<(unsigned int nValue);

    /**
     * Write a number with two decimals.
     *
     * @param fValue The value.
     * @return The stream.
     * @ghidraAddress NTSC-U/C: 0x0029e3c0
     * @ghidraAddress PAL: 0x002a8088
     */
    PrnStream &operator<<(float fValue);

    /**
     * Write `true` or `false`.
     *
     * @param bValue The value.
     * @return The stream.
     * @ghidraAddress NTSC-U/C: 0x0029e450
     * @ghidraAddress PAL: 0x002a8118
     */
    PrnStream &operator<<(bool bValue);
};

/**
 * Write a map as its size followed by one tab-indented key and value line per entry.
 *
 * The output starts with "(size:", the entry count, and ")". Each entry then writes a newline and a
 * tab, "key:", the key, " value:", and the value.
 *
 * @param stream The stream to write to.
 * @param map The map to write.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00333a20
 * @ghidraAddress PAL: 0x003a0fd0
 */
template <typename Key, typename Value>
PrnStream &operator<<(PrnStream &stream, const std::map<Key, Value> &map);

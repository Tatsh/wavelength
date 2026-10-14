#pragma once

#include <list>
#include <map>
#include <vector>

/**
 * Text output stream.
 *
 * The RTTI includes the class name. The vtable runs the type function, the destructor, and
 * Print().
 *
 * Every insertion operator returns the stream. An insertion chain therefore writes its pieces in
 * order through Print().
 */
class PrnStream {
public:
    /**
     * Construct a stream with a dump level of 0.
     *
     * The constructor has no out-of-line copy.
     */
    PrnStream() : mDumpLevel(0) {
    }

    /**
     * Release the stream.
     *
     * @ghidraAddress NTSC-U/C: 0x00339ea0
     * @ghidraAddress PAL: 0x003a7450
     */
    virtual ~PrnStream() {
    }

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
     * Write a short integer in decimal.
     *
     * @param nValue The value.
     * @return The stream.
     * @ghidraAddress NTSC-U/C: 0x0029e268
     * @ghidraAddress PAL: 0x002a7f30
     */
    PrnStream &operator<<(short nValue);

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
     * Write an unsigned short integer in decimal.
     *
     * @param nValue The value.
     * @return The stream.
     * @ghidraAddress NTSC-U/C: 0x0029e318
     * @ghidraAddress PAL: 0x002a7fe0
     */
    PrnStream &operator<<(unsigned short nValue);

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
     * Write an unsigned 64-bit number in decimal.
     *
     * @param nValue The value.
     * @return The stream.
     * @ghidraAddress NTSC-U/C: 0x0029e388
     * @ghidraAddress PAL: 0x002a8050
     */
    PrnStream &operator<<(unsigned long long nValue);

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

    /**
     * Write a number of spaces, one Print() call each.
     *
     * The name is inferred.
     *
     * @param nCount The number of spaces.
     * @ghidraAddress NTSC-U/C: 0x0029e4a8
     * @ghidraAddress PAL: 0x002a8170
     */
    void Space(int nCount);

    /**
     * Detail of an object dump. RndObject::DumpText() lists the referrers only above 0.
     *
     * The name is inferred.
     */
    int mDumpLevel;
};

/** The PrnStream::mDumpLevel value that includes the fourth word of a written Vector3. */
constexpr int kPrnModeFull = 10;

/**
 * Write a list as its size followed by one tab-indented line per element.
 *
 * The output starts with "(size:", the element count, and ")". Each element then writes a newline,
 * its index, a tab, and the element. Each element type has its own instance, one address each.
 *
 * @param stream The stream to write to.
 * @param list The list to write.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x003901b8
 * @ghidraAddress PAL: 0x003fe8c0
 * @ghidraAddress NTSC-U/C: 0x0037db08
 * @ghidraAddress PAL: 0x003ec238
 * @ghidraAddress NTSC-U/C: 0x0037dc10
 * @ghidraAddress PAL: 0x003ec340
 * @ghidraAddress NTSC-U/C: 0x0037f4a8
 * @ghidraAddress PAL: 0x003edbc0
 * @ghidraAddress NTSC-U/C: 0x00380270
 * @ghidraAddress PAL: 0x003ee988
 * @ghidraAddress NTSC-U/C: 0x00380b10
 * @ghidraAddress PAL: 0x003ef228
 * @ghidraAddress NTSC-U/C: 0x00382538
 * @ghidraAddress PAL: 0x003f0c40
 * @ghidraAddress NTSC-U/C: 0x0038fca0
 * @ghidraAddress PAL: 0x003fe3a8
 * @ghidraAddress NTSC-U/C: 0x00391798
 * @ghidraAddress PAL: 0x003ffea0
 * @ghidraAddress NTSC-U/C: 0x0039cad0
 * @ghidraAddress PAL: 0x0040b7f8
 * @ghidraAddress NTSC-U/C: 0x0039cbd8
 * @ghidraAddress PAL: 0x0040b900
 * @ghidraAddress NTSC-U/C: 0x0039cce0
 * @ghidraAddress PAL: 0x0040ba08
 * @ghidraAddress NTSC-U/C: 0x0039cde8
 * @ghidraAddress PAL: 0x0040bb10
 * @ghidraAddress NTSC-U/C: 0x0039cef0
 * @ghidraAddress PAL: 0x0040bc18
 * @ghidraAddress NTSC-U/C: 0x0039cff8
 * @ghidraAddress PAL: 0x0040bd20
 */
template <typename T>
PrnStream &operator<<(PrnStream &stream, const std::list<T> &list) {
    stream << "(size:" << static_cast<unsigned int>(list.size()) << ")";
    int nIndex = 0;
    for (const auto &element : list) {
        stream << "\n" << nIndex << "\t" << element;
        ++nIndex;
    }
    return stream;
}

/**
 * Write a vector as its size followed by one tab-indented line per element.
 *
 * The output has the shape of the list writer's. Each element type has its own instance, one
 * address each.
 *
 * @tparam T The type of the elements.
 * @param stream The stream to write to.
 * @param elements The vector to write.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00387300
 * @ghidraAddress PAL: 0x003f5a08
 * @ghidraAddress NTSC-U/C: 0x00388f60
 * @ghidraAddress PAL: 0x003f7668
 * @ghidraAddress NTSC-U/C: 0x003851b8
 * @ghidraAddress PAL: 0x003f38c0
 * @ghidraAddress NTSC-U/C: 0x00385308
 * @ghidraAddress PAL: 0x003f3a10
 * @ghidraAddress NTSC-U/C: 0x00387450
 * @ghidraAddress PAL: 0x003f5b58
 * @ghidraAddress NTSC-U/C: 0x00389130
 * @ghidraAddress PAL: 0x003f7838
 * @ghidraAddress NTSC-U/C: 0x0038b868
 * @ghidraAddress PAL: 0x003f9f70
 * @ghidraAddress NTSC-U/C: 0x0038bab0
 * @ghidraAddress PAL: 0x003fa1b8
 * @ghidraAddress NTSC-U/C: 0x0038bcf8
 * @ghidraAddress PAL: 0x003fa400
 * @ghidraAddress NTSC-U/C: 0x00392050
 * @ghidraAddress PAL: 0x00400758
 * @ghidraAddress NTSC-U/C: 0x00394ff0
 * @ghidraAddress PAL: 0x004036e0
 * @ghidraAddress NTSC-U/C: 0x003950c8
 * @ghidraAddress PAL: 0x004037b8
 * @ghidraAddress NTSC-U/C: 0x0039d100
 * @ghidraAddress PAL: 0x0040be28
 * @ghidraAddress NTSC-U/C: 0x0039d6c8
 * @ghidraAddress PAL: 0x0040c3f0
 * @ghidraAddress NTSC-U/C: 0x00383510
 * @ghidraAddress PAL: 0x003f1c18
 * @ghidraAddress NTSC-U/C: 0x00389038
 * @ghidraAddress PAL: 0x003f7740
 * @ghidraAddress NTSC-U/C: 0x0038b9b8
 * @ghidraAddress PAL: 0x003fa0c0
 * @ghidraAddress NTSC-U/C: 0x0038bc00
 * @ghidraAddress PAL: 0x003fa308
 * @ghidraAddress NTSC-U/C: 0x0038be48
 * @ghidraAddress PAL: 0x003fa550
 * @ghidraAddress NTSC-U/C: 0x00393cd8
 * @ghidraAddress PAL: 0x004023e0
 */
template <typename T>
PrnStream &operator<<(PrnStream &stream, const std::vector<T> &elements) {
    stream << "(size:" << static_cast<unsigned int>(elements.size()) << ")";
    int nIndex = 0;
    for (const T &element : elements) {
        stream << "\n" << nIndex << "\t" << element;
        ++nIndex;
    }
    return stream;
}

/**
 * Write a map as its size followed by one tab-indented key and value line per entry.
 *
 * The output starts with "(size:", the entry count, and ")". Each entry then writes a newline and a
 * tab, "key:", the key, " value:", and the value. The address is of the instantiation for the
 * objects of RndManager.
 *
 * @tparam Key The type of the keys.
 * @tparam Value The type of the values.
 * @tparam Compare The ordering of the keys.
 * @param stream The stream to write to.
 * @param map The map to write.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00333a20
 * @ghidraAddress PAL: 0x003a0fd0
 */
template <typename Key, typename Value, typename Compare>
PrnStream &operator<<(PrnStream &stream, const std::map<Key, Value, Compare> &map) {
    stream << "(size:" << static_cast<unsigned int>(map.size()) << ")";
    for (const auto &entry : map) {
        stream << "\n\t" << "key:" << entry.first << " value:" << entry.second;
    }
    return stream;
}

/**
 * Write a map in the default key order, in the form of the map writer above.
 *
 * @tparam Key The type of the keys.
 * @tparam Value The type of the values.
 * @param stream The stream to write to.
 * @param map The map to write.
 * @return The stream.
 */
template <typename Key, typename Value>
PrnStream &operator<<(PrnStream &stream, const std::map<Key, Value> &map);

#pragma once

#include <iostream>

#include "os/mem.h"

/**
 * Text an empty HxStr points at, and the substitute for a null buffer.
 *
 * In the North American release a string whose mStr is null is the empty representation, and a
 * caller handing that string to an interface taking a plain pointer substitutes this global rather
 * than passing null. Twenty routines across unrelated subsystems read it through the same idiom.
 * The European release points every empty HxStr at this text instead of storing null, and its
 * asserts call the global `kNullStr`.
 */
extern const char *g_szEmptyString;

/**
 * Heap-allocated NUL-terminated string.
 *
 * Named after `HxStr.cpp`, the file recorded by its own asserts. The member `mStr` comes from the
 * text of the assert `mStr != 0`, and `mLen` from `pos <= mLen`. The class is not polymorphic and
 * has no RTTI, so it has no vptr. Its destructor is inlined at every call site in the image, which
 * is why it is defined here.
 *
 * The North American release represents an empty string by a null `mStr` with `mLen` zero. Every
 * accessor that dereferences `mStr` asserts first, and the comparison operators treat a null
 * pointer and an empty buffer as equal. In the PAL build an empty string points at
 * g_szEmptyString as the European release does. That release drops the null asserts, never frees
 * the shared text, and compares through `strcmp()` alone.
 *
 * `mLen` excludes the terminator, and the buffer is always `mLen + 1` bytes.
 *
 * Both members are public, and each records its own evidence below. The comparison operators are
 * written as members because a member and a free function with the string as its first operand
 * compile to the same two-argument call, which leaves the image unable to distinguish them.
 */
class HxStr {
public:
    /**
     * Construct an empty string.
     *
     * Defined in the header rather than compiled out of line, which is why it has no address of
     * its own. A static initialisation that constructs a string global writes the two members in
     * place with no call, which is what establishes both that the constructor exists and that it
     * does nothing beyond producing the empty representation.
     */
#ifdef VIDEO_STANDARD_PAL
    HxStr() : mLen(0), mStr(const_cast<char *>(g_szEmptyString)) {
    }
#else
    HxStr() : mLen(0), mStr(nullptr) {
    }
#endif

    /**
     * Construct a copy of a C string.
     *
     * A null argument produces an empty string with no allocation.
     *
     * @param pszText The text to copy.
     */
    HxStr(const char *pszText);

    /**
     * Construct a copy of another string.
     *
     * Only a null buffer produces the empty representation. In the PAL build a source
     * pointing at g_szEmptyString is copied into a fresh one-byte buffer, as the European release
     * does.
     *
     * @param other The string to copy.
     */
    HxStr(const HxStr &other);

    /**
     * Construct a run of one repeated character.
     *
     * @param nCount The number of characters, which becomes the length.
     * @param ch The character to repeat.
     */
    HxStr(unsigned nCount, char ch);

    ~HxStr() {
#ifdef VIDEO_STANDARD_PAL
        if (mStr != g_szEmptyString && mStr != nullptr) {
            delete[] mStr;
        }
#else
        if (mStr != nullptr) {
            delete[] mStr;
        }
#endif
    }

    /**
     * Append another string.
     *
     * @param other The string to append.
     * @return This string.
     */
    HxStr &operator+=(const HxStr &other);

    /**
     * Append a C string.
     *
     * The routine builds the temporary string itself rather than taking one a call site built,
     * which is what establishes that the overload exists rather than callers converting and
     * reaching the HxStr overload. The temporary is released before returning.
     *
     * The body is inline. Units that use it emit their own identical out-of-line copy. The North
     * American copies are at `0x00183a70`, `0x00254870`, `0x0028c198`, `0x0030d618`, `0x0038fc58`,
     * `0x003fc688`, `0x00429500`, `0x00431c30`, `0x00453d88`, `0x004beb98`, `0x004dfaa0`, and
     * `0x0050cc00`. The European ones are at `0x00188ca0`, `0x00269f18`, `0x002a7e08`,
     * `0x00333128`, `0x003c1508`, `0x00435068`, `0x00464b20`, `0x0046d8b8`, `0x00491288`,
     * `0x004fcc00`, `0x0051e1e0`, and `0x0054c098`.
     *
     * @param pszText The text to append.
     * @return This string.
     */
    HxStr &operator+=(const char *pszText) {
        return *this += HxStr(pszText);
    }

    /**
     * Append one character.
     *
     * @param ch The character to append.
     * @return This string.
     */
    HxStr &operator+=(char ch);

    /**
     * Replace this string with a copy of a C string.
     *
     * @param pszText The text to copy, or null for an empty string.
     * @return This string.
     */
    HxStr &operator=(const char *pszText);

    /**
     * Replace this string with a copy of another.
     *
     * @param other The string to copy.
     * @return This string.
     */
    HxStr &operator=(const HxStr &other);

    /**
     * Read one character.
     *
     * The index is permitted to equal the length, which reads the terminator.
     *
     * @param i The index.
     * @return The character.
     */
    char operator[](unsigned i) const;

    /**
     * Compare against a C string for inequality.
     *
     * A null pointer and an empty buffer compare equal. In the PAL build a null argument
     * always differs and every other argument goes straight to `strcmp()`.
     *
     * @param pszRight The text to compare against.
     * @return True when the two differ.
     */
    bool operator!=(const char *pszRight) const;

    /**
     * Compare against another string for inequality.
     *
     * @param right The string to compare against.
     * @return True when the two differ.
     */
    bool operator!=(const HxStr &right) const;

    /**
     * Compare against a C string for equality.
     *
     * In the PAL build a null argument never compares equal.
     *
     * @param pszRight The text to compare against.
     * @return True when the two agree.
     */
    bool operator==(const char *pszRight) const;

    /**
     * Compare against another string for equality.
     *
     * @param right The string to compare against.
     * @return True when the two agree.
     */
    bool operator==(const HxStr &right) const;

    /**
     * Order against another string.
     *
     * An empty string sorts before every non-empty string.
     *
     * @param right The string to compare against.
     * @return True when this string sorts before the other.
     */
    bool operator<(const HxStr &right) const;

    /**
     * Discard the text and reserve a zero-filled buffer.
     *
     * The previous buffer is released through the single-object operator delete rather than
     * through operator delete[]. It is the one place in HxStr that uses the scalar release path.
     * The new length is recorded before the allocation is checked.
     *
     * @param nLen The new length, excluding the terminator.
     */
    void Alloc(unsigned nLen);

    /**
     * Find the first occurrence of a character.
     *
     * @param ch The character to look for.
     * @return The index, or -1 when the character is absent.
     */
    int Find(char ch) const;

    /**
     * Find the first occurrence of a character at or after a position.
     *
     * @param ch The character to look for.
     * @param nStart The index to start from.
     * @return The index from the start of the string, or -1 when the character is absent.
     */
    int Find(char ch, unsigned nStart) const;

    /**
     * Find the first occurrence of a substring.
     *
     * @param pszText The text to look for.
     * @return The index, or -1 when the text is absent.
     */
    int Find(const char *pszText) const;

    /**
     * Find the last occurrence of a character.
     *
     * @param ch The character to look for.
     * @return The index, or -1 when the character is absent.
     */
    int ReverseFind(char ch) const;

    /**
     * Find the last occurrence of any of a set of characters.
     *
     * @param pszChars The candidate characters, as a NUL-terminated set.
     * @return The highest index at which any candidate appears, or -1 when none appears and when
     *         the argument is null.
     */
    int ReverseFindOneOf(const char *pszChars) const;

    /**
     * Compare a run of this string against a C string.
     *
     * In the PAL build a null argument reports -1 rather than tripping an assert.
     *
     * @param pos The index to compare from.
     * @param len The number of characters to compare.
     * @param str The text to compare against.
     * @return A negative value, zero, or a positive value, as `strncmp()` defines it.
     */
    int Compare(unsigned pos, unsigned len, const char *str) const;

    /**
     * Extract the text from a position to the end.
     *
     * @param pos The index to start from.
     * @return The extracted text.
     */
    HxStr Mid(unsigned pos) const;

    /**
     * Extract a run of text.
     *
     * A run that would pass the end is shortened to the text from pos onward.
     *
     * @param pos The index to start from.
     * @param len The number of characters to extract.
     * @return The extracted text.
     */
    HxStr Mid(unsigned pos, unsigned len) const;

    /**
     * Overwrite one character.
     *
     * Only a single-character replacement is implemented. A run of any other length trips the
     * `len == 1` assert and is then written as a single character regardless.
     *
     * @param pos The index to overwrite.
     * @param len The number of characters to overwrite, which must be 1.
     * @param ch The replacement character.
     * @return This string.
     */
    HxStr &Replace(unsigned pos, unsigned len, char ch);

    /**
     * Overwrite a run of text with another string.
     *
     * A run that would pass the end is shortened to the text from pos onward.
     *
     * @param pos The index to overwrite from.
     * @param len The number of characters to overwrite.
     * @param other The replacement text.
     * @return This string.
     */
    HxStr &Replace(unsigned pos, unsigned len, const HxStr &other);

    /**
     * Empty the string in place.
     *
     * This is a distinct routine rather than Truncate(0). It tolerates an empty string, where
     * Truncate() would assert, and it asserts nothing itself. The buffer is retained.
     *
     * @return This string.
     */
    HxStr &Clear();

    /**
     * Shorten the string in place.
     *
     * The buffer is not reallocated. The excess capacity remains until the next assignment.
     * In the PAL build an empty string is left untouched rather than written through.
     *
     * @param pos The index to cut at, which must not exceed the current length.
     * @return This string.
     */
    HxStr &Truncate(unsigned pos);

    /**
     * Remove a run of text.
     *
     * The tail is shifted down in place with no reallocation. A run that would pass the end
     * shortens the string to pos instead.
     *
     * @param pos The index to remove from.
     * @param len The number of characters to remove.
     * @return This string.
     */
    HxStr &Erase(unsigned pos, unsigned len);

    /**
     * Insert a run of one repeated character.
     *
     * @param pos The index to insert at.
     * @param nCount The number of characters to insert.
     * @param ch The character to insert.
     * @return This string.
     */
    HxStr &Insert(unsigned pos, unsigned nCount, char ch);

    /**
     * Insert another string.
     *
     * @param pos The index to insert at.
     * @param other The text to insert.
     * @return This string.
     */
    HxStr &Insert(unsigned pos, const HxStr &other);

    /**
     * Write this string to a stream.
     *
     * The same write as operator<<(), with the string as the receiver, and likewise without a null
     * check. No routine in the image calls it. The name is inferred.
     *
     * @param stream The stream to write to.
     * @return The stream.
     */
    std::ostream &Print(std::ostream &stream) const;

    /**
     * Length excluding the terminator.
     *
     * Public because nine sites across four files in other subsystems read this member and `mStr`
     * directly, and the image exposes no accessor for either. A trivial inline accessor and a
     * public member emit the same single load, so the evidence cannot distinguish them, and the
     * public member is preferred because it adds no function that no address can be attached to.
     *
     * +0x00
     */
    unsigned mLen;

    /**
     * Text buffer.
     *
     * Public for the same reason as mLen. In the North American release an empty string is a null
     * buffer rather than a pointer to a terminator. Every reader tests for null first. In the
     * PAL build it points at g_szEmptyString.
     *
     * +0x04
     */
    char *mStr;

private:
    // Adopts a buffer of nLen + 1 bytes that the caller has already filled and terminated. The
    // name is inferred. The constructor is inlined at its one call site, in Mid().
    HxStr(char *pOwnedText, unsigned nLen) : mLen(nLen), mStr(pOwnedText) {
    }
};

/**
 * Concatenate two strings.
 *
 * Every call site inlines the body, so the operator has no address of its own. The body is
 * recovered from that emission rather than invented. A concatenation compiles to a copy
 * construction of the left operand into a temporary, the matching operator+=() on the temporary,
 * and a second copy construction of the result into the destination. `0x0017a79c` through
 * `0x0017a7b4` in SaveRemixMCT::ListRemixDir() and `0x00179de8` through `0x00179e08` are the two
 * clearest copies, and the first temporary is released immediately afterwards at each one.
 *
 * Whether the original wrote a free function or a const member cannot be settled, because a member
 * taking one operand and a free function taking two compile the same way once both are inlined.
 * The free form is used here for the same reason the comparison operators use the member form,
 * which is that one of the two has to be chosen.
 *
 * @param left The string the result starts with.
 * @param right The string appended to it.
 * @return The concatenation.
 */
inline HxStr operator+(const HxStr &left, const HxStr &right) {
    HxStr result(left);
    result += right;
    return result;
}

/**
 * Concatenate a string and a C string.
 *
 * Recovered on the same evidence as the overload above, from the triple at `0x0017a80c` through
 * `0x0017a824` and the one at `0x0038b1d4` through `0x0038b1ec` in
 * Rnd::MetScreen::ResolveContainerViews(). The middle call of each triple is the C string
 * operator+=() at `0x0010edb0` rather than the string one at `0x004b7c68`, which is what
 * distinguishes this overload from a call site converting its operand first.
 *
 * @param left The string the result starts with.
 * @param pszRight The text appended to it.
 * @return The concatenation.
 */
inline HxStr operator+(const HxStr &left, const char *pszRight) {
    HxStr result(left);
    result += pszRight;
    return result;
}

/**
 * Concatenate a string and one character.
 *
 * Recovered on the same evidence as the overloads above, from the triple at `0x0043a8b4` through
 * `0x0043a8cc` in the TnlPointer constructor, whose middle call is the character operator+=() at
 * `0x004b7d28`.
 *
 * @param left The string the result starts with.
 * @param ch The character appended to it.
 * @return The concatenation.
 */
inline HxStr operator+(const HxStr &left, char ch) {
    HxStr result(left);
    result += ch;
    return result;
}

/**
 * Sentinel a search returns when it finds nothing.
 *
 * The word is all ones and sits one word past the empty string in the same literal pool. Its six
 * readers span
 * unrelated subsystems, including the glyph mesh builder and the PyCXX sequence binding, whose
 * released form returns the standard string's own no-position constant from the member that reads
 * it here.
 */
extern const unsigned g_nHxStrNoPosition;

/**
 * Write a string to a stream.
 *
 * The buffer is passed to the stream without a null check. An empty string therefore arrives at
 * the stream as a null pointer, or in the PAL build as g_szEmptyString.
 *
 * @param stream The stream to write to.
 * @param text The string to write.
 * @return The stream.
 */
std::ostream &operator<<(std::ostream &stream, const HxStr &text);

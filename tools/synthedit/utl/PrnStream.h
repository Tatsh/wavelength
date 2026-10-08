#pragma once

/**
 * Base of the streams that print text.
 *
 * The object is 8 bytes. A derived class provides Print(). The constructor clears the word at
 * `+0x04`, and no routine of the control reads it.
 */
class PrnStream {
public:
    PrnStream() : mReserved04(0) {
    }

    /**
     * Destroy the stream.
     *
     * @ghidraAddress 0x100017e0
     */
    virtual ~PrnStream() {
    }

    /**
     * Write text.
     *
     * @param str The text.
     */
    virtual void Print(const char *str) = 0;

    /**
     * Format text with printf() conventions and write it. The text is cut to 1023 characters.
     *
     * @param format The format.
     * @ghidraAddress 0x10017710
     */
    void Printf(const char *format, ...);

    /**
     * Write a character.
     *
     * @param c The character.
     * @return The stream.
     * @ghidraAddress 0x10017740
     */
    PrnStream &operator<<(char c);

    /**
     * Write an integer in decimal.
     *
     * @param i The integer.
     * @return The stream.
     * @ghidraAddress 0x10017760
     */
    PrnStream &operator<<(int i);

    /**
     * Write a number with two decimal places.
     *
     * @param f The number.
     * @return The stream.
     * @ghidraAddress 0x10017780
     */
    PrnStream &operator<<(float f);

    /**
     * Write text.
     *
     * @param str The text.
     * @return The stream.
     * @ghidraAddress 0x100177b0
     */
    PrnStream &operator<<(const char *str);

    /**
     * Write spaces.
     *
     * @param count The number of spaces.
     * @ghidraAddress 0x100177d0
     */
    void Space(int count);

private:
    int mReserved04; // +0x04, cleared by the constructor and not read.
};

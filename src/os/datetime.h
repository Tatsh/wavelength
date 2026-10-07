#pragma once

#include "os/string.h"

class HxStr;

/**
 * Local date and time of the console clock.
 *
 * The type has no RTTI, and the name is inferred. Callers clear it before ReadClock() fills it.
 */
class DateTime {
public:
    /**
     * Read the console clock.
     *
     * The first call starts the clock. Each field is converted from its BCD byte.
     *
     * @return False, with no field written, when the clock reports an error.
     * @ghidraAddress NTSC-U/C: 0x00288648
     * @ghidraAddress PAL: 0x00291ef8
     */
    bool ReadClock();

    /**
     * Write the date as "MM/DD/YYYY". The name is inferred.
     *
     * @param text Receives the date.
     * @ghidraAddress NTSC-U/C: 0x00288a68
     * @ghidraAddress PAL: 0x00292318
     */
    void FormatDate(String &text) const;

    /**
     * Write the date and time as "MM/DD/YYYY  HH:MM:SS". The name is inferred.
     *
     * @param text Receives the date and time.
     * @ghidraAddress NTSC-U/C: 0x00288a08
     * @ghidraAddress PAL: 0x002922b8
     */
    void FormatDateTime(String &text) const;

    /**
     * Write the month and the day as "MM/DD". The name is inferred.
     *
     * @param text Receives the month and the day.
     * @ghidraAddress NTSC-U/C: 0x00288ab8
     * @ghidraAddress PAL: 0x00292368
     */
    void FormatMonthDay(String &text) const;

    unsigned char mSecond; /*!< The second, 0 to 59. */
    unsigned char mMinute; /*!< The minute, 0 to 59. */
    unsigned char mHour;   /*!< The hour, 0 to 23. */
    unsigned char mDay;    /*!< The day of the month, from 1. */
    unsigned char mMonth;  /*!< The month, from 0. */
    unsigned char mYear;   /*!< The years since 1900. */
};

/**
 * Write the console's local date and time as "MM/DD/YY, HH:MM".
 *
 * The clock is read with sceCdReadClock() and converted to local time. Each field is written as
 * the two decimal digits of its BCD byte. The day precedes the year and follows the month, in the
 * United States order. text is replaced, not appended to. A failed read or a clock whose status
 * byte is non-zero writes nothing.
 *
 * @param text Receives the date and time.
 * @return Whether text was written.
 * @ghidraAddress NTSC-U/C: 0x0053a108
 * @ghidraAddress PAL: 0x00579a38
 */
bool FormatCurrentDateTime(HxStr &text);

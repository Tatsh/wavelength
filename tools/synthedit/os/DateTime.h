#pragma once

#include "utl/PrnStream.h"
#include "utl/Str.h"

/**
 * Local calendar date and time of day, one byte per field.
 *
 * The object is 6 bytes. The name is inferred from the routines that fill and print it.
 */
class DateTime {
public:
    /**
     * Write the date and time as `MM/DD/YYYY  hh:mm:ss`.
     *
     * @param out Receives the text.
     * @ghidraAddress 0x1000ea40
     */
    void ToString(String &out) const;

    unsigned char mSec;   /*!< Seconds, 0 to 59. */
    unsigned char mMin;   /*!< Minutes, 0 to 59. */
    unsigned char mHour;  /*!< Hours, 0 to 23. */
    unsigned char mDay;   /*!< Day of the month, 1 to 31. */
    unsigned char mMonth; /*!< Month, 0 for January. */
    unsigned char mYear;  /*!< Years since 1900. */
};

/**
 * Read the local date and time.
 *
 * @param dateTime Receives the date and time.
 * @return Always true.
 * @ghidraAddress 0x1000e9f0
 */
bool GetDateTime(DateTime &dateTime);

/**
 * Print a date and time as DateTime::ToString() writes it.
 *
 * @param stream The stream.
 * @param dateTime The date and time.
 * @return The stream.
 * @ghidraAddress 0x1000ea90
 */
PrnStream &operator<<(PrnStream &stream, const DateTime &dateTime);

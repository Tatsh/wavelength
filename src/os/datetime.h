#pragma once

#include "os/binstream.h"
#include "os/prnstream.h"
#include "os/string.h"

/**
 * Local date and time of the console clock.
 *
 * The type has no RTTI, and the name is inferred. Callers clear it before ReadClock() fills it.
 * ReadClock() returns the clock reading PollClock() last refreshed, which a task of the worker
 * thread reads every 30 seconds.
 */
class DateTime {
public:
    /** Construct a date whose fields are not set. */
    DateTime() = default;

    /**
     * Construct a date from a count of seconds ToSeconds() reported.
     *
     * The name is inferred.
     *
     * @param nSeconds The seconds.
     * @ghidraAddress NTSC-U/C: 0x002888f0
     * @ghidraAddress PAL: 0x002921a0
     */
    explicit DateTime(unsigned int nSeconds);

    /**
     * Read the console clock.
     *
     * The first call reads the clock and starts the 30-second refresh. Each field is converted
     * from its BCD byte.
     *
     * @return False, with no field written, when the clock reports an error.
     * @ghidraAddress NTSC-U/C: 0x00288648
     * @ghidraAddress PAL: 0x00291ef8
     */
    bool ReadClock();

    /**
     * Start a read of the console clock on the worker thread once 30 seconds have passed since
     * the last read. SystemPoll() calls it once per frame.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00288778
     * @ghidraAddress PAL: 0x00292028
     */
    static void PollClock();

    /**
     * Report the date as a count of seconds, with 32-day months and 384-day years from 2000. The
     * count orders dates but is not a calendar count.
     *
     * The name is inferred.
     *
     * @return The seconds.
     * @ghidraAddress NTSC-U/C: 0x002889a0
     * @ghidraAddress PAL: 0x00292250
     */
    unsigned int ToSeconds() const;

    /**
     * Write the date and time as "MM/DD/YYYY  HH:MM:SS". The name is inferred.
     *
     * @param text Receives the date and time.
     * @ghidraAddress NTSC-U/C: 0x00288a08
     * @ghidraAddress PAL: 0x002922b8
     */
    void FormatDateTime(String &text) const;

    /**
     * Write the date as "MM/DD/YYYY". The name is inferred.
     *
     * @param text Receives the date.
     * @ghidraAddress NTSC-U/C: 0x00288a68
     * @ghidraAddress PAL: 0x00292318
     */
    void FormatDate(String &text) const;

    /**
     * Write the month and the day as "MM/DD". The name is inferred.
     *
     * @param text Receives the month and the day.
     * @ghidraAddress NTSC-U/C: 0x00288ab8
     * @ghidraAddress PAL: 0x00292368
     */
    void FormatMonthDay(String &text) const;

    /**
     * Write the date as "MM/DD/YY", the year counted from 2000. The name is inferred.
     *
     * @param text Receives the date.
     * @ghidraAddress NTSC-U/C: 0x00288b00
     * @ghidraAddress PAL: 0x002923b0
     */
    void FormatShortDate(String &text) const;

    /**
     * Report whether this date is earlier than another.
     *
     * @param other The other date.
     * @return Whether this date comes first.
     * @ghidraAddress NTSC-U/C: 0x00288b50
     * @ghidraAddress PAL: 0x00292400
     */
    bool operator<(const DateTime &other) const;

    unsigned char mSecond; /*!< The second, 0 to 59. */
    unsigned char mMinute; /*!< The minute, 0 to 59. */
    unsigned char mHour;   /*!< The hour, 0 to 23. */
    unsigned char mDay;    /*!< The day of the month, from 1. */
    unsigned char mMonth;  /*!< The month, from 0. */
    unsigned char mYear;   /*!< The years since 1900. */

private:
    /**
     * Read the console clock into the snapshot, on the worker thread.
     *
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x00288818
     * @ghidraAddress PAL: 0x002920c8
     */
    static int ReadClockTask();

    /**
     * Take the snapshot as the clock reading, and set the next read 30 seconds from now.
     *
     * @param nResult The result of ReadClockTask().
     * @ghidraAddress NTSC-U/C: 0x00288850
     * @ghidraAddress PAL: 0x00292100
     */
    static void OnClockRead(int nResult);
};

/**
 * Write a date and time as FormatDateTime() does.
 *
 * @param stream The stream.
 * @param date The date.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00288bf0
 * @ghidraAddress PAL: 0x002924a0
 */
PrnStream &operator<<(PrnStream &stream, const DateTime &date);

/**
 * Write the six bytes of a date.
 *
 * @param stream The stream.
 * @param date The date.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00288c68
 */
BinStream &operator<<(BinStream &stream, const DateTime &date);

/**
 * Read the six bytes of a date.
 *
 * @param stream The stream.
 * @param date Receives the date.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00288d70
 * @ghidraAddress PAL: 0x002925d8
 */
BinStream &operator>>(BinStream &stream, DateTime &date);

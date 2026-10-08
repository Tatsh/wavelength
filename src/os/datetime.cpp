#include "os/datetime.h"

#include <libcdvd.h>
#include <libscf.h>

#include "os/asynctask.h"
#include "os/system.h"

namespace {

constexpr int kBcdDigitShift = 4;
constexpr unsigned char kBcdDigitMask = 0x0f;
constexpr int kDecimalBase = 10;

// The clock months count from 1, the clock years from 2000, and DateTime from 0 and 1900.
constexpr int kMonthOffset = 1;
constexpr int kClockYearOffset = 100;
constexpr int kYearBase = 1900;

// The milliseconds between reads of the clock, and the time that stands for no read pending.
constexpr float kClockReadIntervalMs = 30000.0f;
constexpr float kNoClockRead = 1.0e30f;

constexpr unsigned int kSecondsPerMinute = 60;
constexpr unsigned int kSecondsPerHour = 3600;
constexpr unsigned int kSecondsPerDay = 86400;
constexpr unsigned int kSecondsPerMonth = 32 * kSecondsPerDay;
constexpr unsigned int kSecondsPerYear = 384 * kSecondsPerDay;

constexpr int kYearShift = 16;
constexpr int kMonthShift = 8;

// NTSC-U/C: 0x003b20cc
int sClockStarted;

// NTSC-U/C: 0x003b20c8
float sNextClockRead;

// NTSC-U/C: 0x0047fa68
sceCdCLOCK sClock;

// NTSC-U/C: 0x0047fa70
sceCdCLOCK sClockSnapshot;

// NTSC-U/C: 0x00288628, PAL: 0x00291ed8
int BcdToBinary(unsigned char nBcd) {
    return (nBcd >> kBcdDigitShift) * kDecimalBase + (nBcd & kBcdDigitMask);
}

} // namespace

DateTime::DateTime(unsigned int nSeconds) {
    const unsigned int nYears = nSeconds / kSecondsPerYear;
    mYear = static_cast<unsigned char>(nYears + kClockYearOffset);
    nSeconds -= nYears * kSecondsPerYear;
    mMonth = static_cast<unsigned char>(nSeconds / kSecondsPerMonth);
    nSeconds -= mMonth * kSecondsPerMonth;
    mDay = static_cast<unsigned char>(nSeconds / kSecondsPerDay);
    nSeconds -= mDay * kSecondsPerDay;
    mHour = static_cast<unsigned char>(nSeconds / kSecondsPerHour);
    nSeconds -= mHour * kSecondsPerHour;
    mMinute = static_cast<unsigned char>(nSeconds / kSecondsPerMinute);
    nSeconds -= mMinute * kSecondsPerMinute;
    mSecond = static_cast<unsigned char>(nSeconds);
}

bool DateTime::ReadClock() {
    if (sClockStarted == 0) {
        sClockStarted = 1;
        sceCdReadClock(&sClock);
        sceScfGetLocalTimefromRTC(&sClock);
        sNextClockRead = SystemMs() + kClockReadIntervalMs;
    }
    if (sClock.stat != 0) {
        return false;
    }
    mSecond = static_cast<unsigned char>(BcdToBinary(sClock.second));
    mMinute = static_cast<unsigned char>(BcdToBinary(sClock.minute));
    mHour = static_cast<unsigned char>(BcdToBinary(sClock.hour));
    mDay = static_cast<unsigned char>(BcdToBinary(sClock.day));
    mMonth = static_cast<unsigned char>(BcdToBinary(sClock.month) - kMonthOffset);
    mYear = static_cast<unsigned char>(BcdToBinary(sClock.year) + kClockYearOffset);
    return true;
}

void DateTime::PollClock() {
    if (sNextClockRead <= SystemMs()) {
        sNextClockRead = kNoClockRead;
        EnqueueAsyncTask(ReadClockTask, OnClockRead);
    }
}

int DateTime::ReadClockTask() {
    sceCdReadClock(&sClockSnapshot);
    sceScfGetLocalTimefromRTC(&sClockSnapshot);
    return 0;
}

void DateTime::OnClockRead([[maybe_unused]] int nResult) {
    sClock = sClockSnapshot;
    sNextClockRead = SystemMs() + kClockReadIntervalMs;
}

unsigned int DateTime::ToSeconds() const {
    return mSecond + mMinute * kSecondsPerMinute + mHour * kSecondsPerHour + mDay * kSecondsPerDay +
           mMonth * kSecondsPerMonth + (mYear - kClockYearOffset) * kSecondsPerYear;
}

void DateTime::FormatDateTime(String &text) const {
    text = FormatString("%02d/%02d/%04d  %02d:%02d:%02d",
                        mMonth + kMonthOffset,
                        mDay,
                        mYear + kYearBase,
                        mHour,
                        mMinute,
                        mSecond);
}

void DateTime::FormatDate(String &text) const {
    text = FormatString("%02d/%02d/%04d", mMonth + kMonthOffset, mDay, mYear + kYearBase);
}

void DateTime::FormatMonthDay(String &text) const {
    text = FormatString("%02d/%02d", mMonth + kMonthOffset, mDay);
}

void DateTime::FormatShortDate(String &text) const {
    text = FormatString("%02d/%02d/%02d", mMonth + kMonthOffset, mDay, mYear - kClockYearOffset);
}

bool DateTime::operator<(const DateTime &other) const {
    const unsigned int nDate = (mYear << kYearShift) + (mMonth << kMonthShift) + mDay;
    const unsigned int nOtherDate =
        (other.mYear << kYearShift) + (other.mMonth << kMonthShift) + other.mDay;
    if (nDate < nOtherDate) {
        return true;
    }
    if (nDate != nOtherDate) {
        return false;
    }
    const unsigned int nTime = (mHour << kYearShift) + (mMinute << kMonthShift) + mSecond;
    const unsigned int nOtherTime =
        (other.mHour << kYearShift) + (other.mMinute << kMonthShift) + other.mSecond;
    return nTime < nOtherTime;
}

PrnStream &operator<<(PrnStream &stream, const DateTime &date) {
    String text;
    date.FormatDateTime(text);
    stream.Print(text.c_str());
    return stream;
}

BinStream &operator<<(BinStream &stream, const DateTime &date) {
    const unsigned char aFields[] = {
        date.mSecond, date.mMinute, date.mHour, date.mDay, date.mMonth, date.mYear};
    for (const unsigned char nField : aFields) {
        stream.Write(&nField, sizeof(nField));
    }
    return stream;
}

BinStream &operator>>(BinStream &stream, DateTime &date) {
    stream.Read(&date.mSecond, sizeof(date.mSecond));
    stream.Read(&date.mMinute, sizeof(date.mMinute));
    stream.Read(&date.mHour, sizeof(date.mHour));
    stream.Read(&date.mDay, sizeof(date.mDay));
    stream.Read(&date.mMonth, sizeof(date.mMonth));
    stream.Read(&date.mYear, sizeof(date.mYear));
    return stream;
}

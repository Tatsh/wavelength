#include "os/DateTime.h"

#include <ctime>

namespace {

const int kYearBase = 1900;

} // namespace

bool GetDateTime(DateTime &dateTime) {
    time_t now;
    time(&now);
    const tm *local = localtime(&now);
    dateTime.mSec = static_cast<unsigned char>(local->tm_sec);
    dateTime.mMin = static_cast<unsigned char>(local->tm_min);
    dateTime.mHour = static_cast<unsigned char>(local->tm_hour);
    dateTime.mDay = static_cast<unsigned char>(local->tm_mday);
    dateTime.mMonth = static_cast<unsigned char>(local->tm_mon);
    dateTime.mYear = static_cast<unsigned char>(local->tm_year);
    return true;
}

void DateTime::ToString(String &out) const {
    out = FormatString(
        "%02d/%02d/%04d  %02d:%02d:%02d", mMonth + 1, mDay, mYear + kYearBase, mHour, mMin, mSec);
}

PrnStream &operator<<(PrnStream &stream, const DateTime &dateTime) {
    String text;
    dateTime.ToString(text);
    stream.Print(text.c_str());
    return stream;
}

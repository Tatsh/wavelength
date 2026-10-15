#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <format>
#include <span>
#include <string>

namespace Tools::BuildImage {

/** A UTC time with a resolution of one second. */
using Seconds = std::chrono::sys_seconds;

/**
 * Write the seven-byte date of a directory record, in UTC.
 *
 * @param bytes Buffer holding the record.
 * @param offset Offset of the date's first byte.
 * @param time Time to write.
 */
inline void writeRecordDate(std::span<std::uint8_t> bytes, std::size_t offset, Seconds time) {
    constexpr int kYearBase = 1900;
    const auto midnight = std::chrono::floor<std::chrono::days>(time);
    const std::chrono::year_month_day day{midnight};
    const std::chrono::hh_mm_ss clock{time - midnight};
    bytes[offset] = static_cast<std::uint8_t>(static_cast<int>(day.year()) - kYearBase);
    bytes[offset + 1] = static_cast<std::uint8_t>(static_cast<unsigned>(day.month()));
    bytes[offset + 2] = static_cast<std::uint8_t>(static_cast<unsigned>(day.day()));
    bytes[offset + 3] = static_cast<std::uint8_t>(clock.hours().count());
    bytes[offset + 4] = static_cast<std::uint8_t>(clock.minutes().count());
    bytes[offset + 5] = static_cast<std::uint8_t>(clock.seconds().count());
    bytes[offset + 6] = 0;
}

/**
 * Format the 16 digits of a volume descriptor date, in UTC with hundredths of zero.
 *
 * @param time Time to format.
 * @return The digits; the descriptor's 17th byte, the zone offset, is written separately.
 */
inline std::string volumeDate(Seconds time) {
    return std::format("{:%Y%m%d%H%M%S}00", time);
}

} // namespace Tools::BuildImage

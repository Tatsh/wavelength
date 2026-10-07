#pragma once

/** EE clock in cycles per millisecond, which is the divisor the elapsed time is computed with. */
constexpr unsigned kCyclesPerMillisecond = 294912;

/** Milliseconds in one second, the rate GetMillisecondsPerSecond() reports. */
constexpr int kMillisecondsPerSecond = 1000;

/**
 * Read the free-running cycle counter.
 *
 * Every caller in the image inlines the read, so the image has no out-of-line copy and there is no
 * address to record. On the shipped target the read is the EE `Count` coprocessor register, which
 * is the one genuinely machine-specific part of the timing instrumentation, so a port supplies its
 * own implementation file for this declaration.
 *
 * @return The counter, which wraps.
 */
unsigned ReadCycleCount();

/**
 * Report how many milliseconds one second has.
 *
 * The body is here rather than in a source file because callers expand it inline. Its one
 * out-of-line copy is in the unit whose static initialiser is at `0x004662d0` and which also
 * defines ShowReportedMessage().
 * HudScreenFlash's constructor calls that copy, and Sch::SystemTime divides its scale by the
 * result.
 *
 * The name is inferred from the value. Nothing in the image attests it.
 *
 * @return 1000.
 * @ghidraAddress NTSC-U/C: 0x00466360
 * @ghidraAddress PAL: 0x004a3d90
 */
inline int GetMillisecondsPerSecond() {
    return kMillisecondsPerSecond;
}

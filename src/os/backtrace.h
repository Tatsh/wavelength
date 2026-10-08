#pragma once

/**
 * Record the call sites of the functions on the stack, innermost first.
 *
 * Each frame's size and the slot of its saved return address come from the epilogue of the
 * function, found by scanning its instructions forward from the return address. A cache of 256
 * entries keyed by the return address retains each scan. The walk stops at the call of main(), at
 * a frame whose size it cannot find, or when the array is full. A terminating zero follows the
 * recorded sites in the first two cases.
 *
 * @param pFrames Receives the address of each call instruction.
 * @param nMaxFrames The size of pFrames.
 * @ghidraAddress NTSC-U/C: 0x0028dc38
 * @ghidraAddress PAL: 0x00297618
 */
void CaptureStackFrames(unsigned int *pFrames, int nMaxFrames);

/**
 * Report the address the caller returns to.
 *
 * @return The return address of the caller of this routine.
 * @ghidraAddress NTSC-U/C: 0x0028de98
 * @ghidraAddress PAL: 0x00297878
 */
__attribute__((noinline)) unsigned int BacktraceReturnAddress();

/**
 * Report the caller's stack pointer.
 *
 * @return The stack pointer.
 * @ghidraAddress NTSC-U/C: 0x0028dea4
 * @ghidraAddress PAL: 0x00297884
 */
__attribute__((noinline)) unsigned int BacktraceStackPointer();

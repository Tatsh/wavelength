#pragma once

/**
 * Load the network modules the "net" section of the configuration lists and build the network
 * managers.
 *
 * The routine does nothing when the word at `0x00100000` is non-zero. The same test guards
 * TerminateNetSubsystem() and PollNetSubsystem().
 *
 * @ghidraAddress NTSC-U/C: 0x0026a090
 * @ghidraAddress PAL: 0x00273c20
 */
void InitializeNetSubsystem();

/**
 * Destroy the network managers InitializeNetSubsystem() built.
 *
 * @ghidraAddress NTSC-U/C: 0x0026a130
 * @ghidraAddress PAL: 0x00273cc0
 */
void TerminateNetSubsystem();

/**
 * Service the network managers once per frame.
 *
 * @ghidraAddress NTSC-U/C: 0x0026a1f0
 * @ghidraAddress PAL: 0x00273d80
 */
void PollNetSubsystem();

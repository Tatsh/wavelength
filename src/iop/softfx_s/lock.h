#pragma once

/**
 * Create the semaphore that serialises the RPC command runs and the block processing.
 *
 * @ghidraAddress NTSC-U/C: 0x00000d50
 * @ghidraAddress PAL: 0x00000d50
 */
void CreateLock();

/**
 * Delete the semaphore CreateLock() created.
 *
 * @ghidraAddress NTSC-U/C: 0x00000d90
 * @ghidraAddress PAL: 0x00000d90
 */
void DeleteLock();

/**
 * Wait for the lock.
 *
 * @param owner Name of the caller. The routine does not read it.
 * @ghidraAddress NTSC-U/C: 0x00000db8
 * @ghidraAddress PAL: 0x00000db8
 */
void AcquireLock(const char *owner);

/**
 * Release the lock.
 *
 * @param owner Name of the caller. The routine does not read it.
 * @ghidraAddress NTSC-U/C: 0x00000de0
 * @ghidraAddress PAL: 0x00000de0
 */
void ReleaseLock(const char *owner);

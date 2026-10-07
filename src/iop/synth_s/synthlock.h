#pragma once

#include <kernel.h>

/**
 * The binary semaphore that stops the tick thread and the RPC server from entering the
 * synthesiser at the same time.
 *
 * The class has no RTTI, and its name is inferred from its role. Every member is static. Lock()
 * and Unlock() take the name of the holder for debugging, and ignore it.
 */
class SynthLock {
public:
    /**
     * Create the semaphore, available.
     *
     * @ghidraAddress NTSC-U/C: 0x00005d30
     * @ghidraAddress PAL: 0x00005d30
     */
    static void Create();

    /**
     * Delete the semaphore.
     *
     * @ghidraAddress NTSC-U/C: 0x00005d70
     * @ghidraAddress PAL: 0x00005d70
     */
    static void Destroy();

    /**
     * Wait for the semaphore and take it.
     *
     * @param owner Name of the holder.
     * @ghidraAddress NTSC-U/C: 0x00005d98
     * @ghidraAddress PAL: 0x00005d98
     */
    static void Lock(const char *owner);

    /**
     * Give the semaphore back.
     *
     * @param owner Name of the holder.
     * @ghidraAddress NTSC-U/C: 0x00005dc0
     * @ghidraAddress PAL: 0x00005dc0
     */
    static void Unlock(const char *owner);

private:
    static int sSemaphore;       /*!< The semaphore, or -1 before Create(). */
    static SemaParam sSemaParam; /*!< Creation parameters. */
};

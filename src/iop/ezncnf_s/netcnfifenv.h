#pragma once

#include <netcnf.h>

/**
 * A netcnf environment that loads into memory of the module.
 *
 * The module was built without RTTI. The name is inferred from the module's routines.
 */
class NetcnfifEnv : public sceNetCnfEnv {
public:
    /**
     * Clear the environment and give it the module's memory.
     *
     * @ghidraAddress NTSC-U/C: 0x00000c5c
     * @ghidraAddress PAL: 0x00000c5c
     */
    void Init();

    /**
     * Load a combination into the environment.
     *
     * @param fileName Configuration file.
     * @param userName Name the user gave the combination.
     * @return The result of sceNetCnfLoadEntry().
     * @ghidraAddress NTSC-U/C: 0x00000b40
     * @ghidraAddress PAL: 0x00000b40
     */
    int LoadEntry(const char *fileName, const char *userName);

    /**
     * Classify the interface of the loaded combination.
     *
     * @return A NetCnfDeviceType.
     * @ghidraAddress NTSC-U/C: 0x00000240
     * @ghidraAddress PAL: 0x00000240
     */
    int GetDeviceType() const;

    /**
     * Choose a value by the device of a file.
     *
     * @param fileName File.
     * @param memoryCard Value for a file on a memory card.
     * @param hardDisk Value for a file on the hard disk.
     * @param other Value for a file on another device.
     * @return The value for the device of @p fileName.
     * @ghidraAddress NTSC-U/C: 0x00000bd4
     * @ghidraAddress PAL: 0x00000bd4
     */
    static int SelectByDevice(const char *fileName, int memoryCard, int hardDisk, int other);

private:
    /** Size of sMemory. */
    static constexpr int kMemorySize = 0x800;

    /** Memory the loaded records live in. */
    alignas(16) static unsigned char sMemory[kMemorySize];
};

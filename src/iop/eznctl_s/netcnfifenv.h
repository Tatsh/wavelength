#pragma once

#include <netcnf.h>

#include "eznctl_s/netcnfifdata.h"

/**
 * A netcnf environment built in memory of the module, either loaded from a configuration file or
 * written from a NetcnfifData.
 *
 * The module was built without RTTI. The name is inferred from the module's routines.
 */
class NetcnfifEnv : public sceNetCnfEnv {
public:
    /**
     * Clear the environment and give it the module's memory.
     *
     * @ghidraAddress NTSC-U/C: 0x00000edc
     * @ghidraAddress PAL: 0x00000eec
     */
    void Init();

    /**
     * Load a combination into the environment.
     *
     * @param fileName Configuration file.
     * @param userName Name the user gave the combination.
     * @return The result of sceNetCnfLoadEntry().
     * @ghidraAddress NTSC-U/C: 0x00000dc0
     * @ghidraAddress PAL: 0x00000dd0
     */
    int LoadEntry(const char *fileName, const char *userName);

    /**
     * Build the records of a NetcnfifData in the environment.
     *
     * @param data Settings. The records point into it.
     * @param kind #SCE_NETCNF_TYPE_NET for a combination, or the kind of a single file.
     * @return Zero, or a negative error code.
     * @ghidraAddress NTSC-U/C: 0x00001bd0
     * @ghidraAddress PAL: 0x00001be0
     */
    int WriteEnv(NetcnfifData *data, int kind);

    /**
     * Prefix each dial number of a dialled PPP interface with the dial prefix of its device, and
     * give any other interface the type of its device.
     *
     * @ghidraAddress NTSC-U/C: 0x000006fc
     * @ghidraAddress PAL: 0x000006f4
     */
    void MergeDialNumbers();

    /**
     * Choose a value by the device of a file.
     *
     * @param fileName File.
     * @param memoryCard Value for a file on a memory card.
     * @param hardDisk Value for a file on the hard disk.
     * @param other Value for a file on another device.
     * @return The value for the device of @p fileName.
     * @ghidraAddress NTSC-U/C: 0x00000e54
     * @ghidraAddress PAL: 0x00000e64
     */
    static int SelectByDevice(const char *fileName, int memoryCard, int hardDisk, int other);

private:
    /** Error code for a NetcnfifData that sets nothing of a kind. */
    static constexpr int kErrorNoSettings = -100;

    /**
     * Build the combination of a NetcnfifData as one pair with its interface and device.
     *
     * @param data Settings.
     * @return Zero, or a negative error code.
     * @ghidraAddress NTSC-U/C: 0x000019e0
     * @ghidraAddress PAL: 0x000019f0
     */
    int WritePairs(NetcnfifData *data);

    /**
     * Build the interface or device of a NetcnfifData in sceNetCnfEnv::interface, allocating it
     * when there is none.
     *
     * @param data Settings.
     * @param kind #SCE_NETCNF_TYPE_INTERFACE or #SCE_NETCNF_TYPE_DEVICE.
     * @return Zero or a positive value, -2 when no memory remains, or #kErrorNoSettings with
     * sceNetCnfEnv::interface cleared when @p data sets nothing of the kind.
     * @ghidraAddress NTSC-U/C: 0x00001514
     * @ghidraAddress PAL: 0x00001524
     */
    int WriteInterface(NetcnfifData *data, int kind);

    /**
     * Add the default route and the name servers of a static configuration.
     *
     * @param data Settings.
     * @return The result of the last command added.
     * @ghidraAddress NTSC-U/C: 0x0000129c
     * @ghidraAddress PAL: 0x000012ac
     */
    int WriteAddresses(NetcnfifData *data);

    /**
     * Add the default route to sceNetCnfEnv::interface.
     *
     * @param gateway Gateway, or null for a route without one.
     * @return Zero, or a negative error code.
     * @ghidraAddress NTSC-U/C: 0x00001088
     * @ghidraAddress PAL: 0x00001098
     */
    int AddRoute(const char *gateway);

    /**
     * Add a name server to sceNetCnfEnv::interface.
     *
     * @param address Server address.
     * @param index 1 for the first server, 2 for the second.
     * @return Zero, or a negative error code.
     * @ghidraAddress NTSC-U/C: 0x000011c8
     * @ghidraAddress PAL: 0x000011d8
     */
    int AddNameServer(const char *address, int index);

    /**
     * Attach sceNetCnfEnv::interface to the first pair of the combination, creating the
     * combination or the pair when missing.
     *
     * @param kind #SCE_NETCNF_TYPE_INTERFACE or #SCE_NETCNF_TYPE_DEVICE.
     * @return Zero, or -2 when no memory remains.
     * @ghidraAddress NTSC-U/C: 0x000013a4
     * @ghidraAddress PAL: 0x000013b4
     */
    int Attach(int kind);

    /** Round the next free byte up to a word and start the free memory there. */
    inline void AlignMemory();

    /**
     * Append a command to the commands of sceNetCnfEnv::interface.
     *
     * @param command Command.
     */
    inline void AppendCommand(sceNetCnfCommand *command);

    /**
     * Append a pair to the combination.
     *
     * @param pair Pair.
     */
    inline void AppendPair(sceNetCnfPair *pair);

    /**
     * Set the defaults of a combination.
     */
    inline void ResetRoot();

    /**
     * Clear every setting of an interface or device to its unset value.
     *
     * @param interface Settings.
     * @ghidraAddress NTSC-U/C: 0x00000f90
     * @ghidraAddress PAL: 0x00000fa0
     */
    static void ResetInterface(sceNetCnfInterface *interface);

    /**
     * Report whether an address has a character other than a dot or a zero.
     *
     * @param text Address.
     * @return True when the address is not all zeros.
     * @ghidraAddress NTSC-U/C: 0x00001034
     * @ghidraAddress PAL: 0x00001044
     */
    static bool IsNonzeroAddress(const char *text);

    /** Size of sMemory. */
    static constexpr int kMemorySize = 0x800;

    /** Number of name servers. */
    static constexpr int kNameServerCount = 2;

    /** Memory the records live in. */
    alignas(16) static unsigned char sMemory[kMemorySize];

    /** Coded form of NetcnfifData::mChatScript. */
    static char sChatScript[NetcnfifData::kTextSize];

    /** Display name of the pair WritePairs() builds. */
    static char sPairName[NetcnfifData::kTextSize];

    /** The default route command. */
    static sceNetCnfRouteCommand sRoute;

    /** The name server commands. */
    static sceNetCnfNameServerCommand sNameServers[kNameServerCount];
};

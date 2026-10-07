#pragma once

#include <sifrpc.h>

#include "ezncnf_s/netcnfifenv.h"
#include "ezncnf_s/netcnfrequest.h"

/**
 * The eznetcnf module reads network configuration files for the EE through a SIF RPC server. It
 * counts and lists the entries of a file and sends loaded entries to the EE as NetcnfifData.
 *
 * The module was built without RTTI. The name is inferred from the module's name.
 */
class EzNetCnf {
public:
    /** SIF RPC server identifier of the module. */
    static constexpr unsigned int kRpcServerId = 0x75499128;

    /** RPC function numbers. */
    enum Function {
        kFunctionGetCount = 0,  /*!< Count the combinations of NetCnfRequest::mFileName. */
        kFunctionGetList = 1,   /*!< NetCnfRequest::GetList(). */
        kFunctionLoadEntry = 2, /*!< NetCnfRequest::LoadEntry(). */
    };

    /**
     * Start the module. Create and start the RPC server thread.
     *
     * @return A ModuleStartResult.
     * @ghidraAddress NTSC-U/C: 0x00000034
     * @ghidraAddress PAL: 0x00000034
     */
    static int ModuleStart();

    /**
     * Unload the module. Only an `other` argument stops the RPC server thread.
     *
     * @param argc Argument count.
     * @param argv Arguments.
     * @return A ModuleStartResult.
     * @ghidraAddress NTSC-U/C: 0x000000d8
     * @ghidraAddress PAL: 0x000000d8
     */
    static int ModuleStop(int argc, char **argv);

    /**
     * Copy a buffer to EE memory by SIF DMA. A size that is not a multiple of 16 bytes is padded,
     * with a warning.
     *
     * @param data Source in IOP memory.
     * @param destination Destination in EE memory.
     * @param size Byte count.
     * @param noWait False to wait for the transfer to complete.
     * @return The transfer identifier.
     * @ghidraAddress NTSC-U/C: 0x00000a68
     * @ghidraAddress PAL: 0x00000a68
     */
    static unsigned int SendToEe(void *data, void *destination, int size, bool noWait);

    /** Thread that runs the RPC server. */
    static int sThread;

    /** Interrupt state that CpuSuspendIntr() saves around memory allocation and DMA. */
    static int sInterruptState;

    /** Environment each load fills. */
    static NetcnfifEnv sEnv;

private:
    /**
     * Thread entry. Register the RPC server and serve requests forever.
     *
     * @ghidraAddress NTSC-U/C: 0x00000174
     * @ghidraAddress PAL: 0x00000174
     */
    static void RpcServerThread();

    /**
     * Remove the RPC server and its queue.
     *
     * @ghidraAddress NTSC-U/C: 0x000001ec
     * @ghidraAddress PAL: 0x000001ec
     */
    static void RemoveRpcServer();

    /**
     * Prepare the module after the RPC server thread starts.
     *
     * @return Zero.
     * @ghidraAddress NTSC-U/C: 0x00000230
     * @ghidraAddress PAL: 0x00000230
     */
    static int Initialize();

    /**
     * Release the module before it unloads.
     *
     * @return Zero.
     * @ghidraAddress NTSC-U/C: 0x00000238
     * @ghidraAddress PAL: 0x00000238
     */
    static int Finalize();

    /**
     * Serve one request. The first word of the buffer receives the result.
     *
     * @param function A Function.
     * @param buffer The NetCnfRequest.
     * @param size Size of the request.
     * @return @p buffer.
     * @ghidraAddress NTSC-U/C: 0x00000998
     * @ghidraAddress PAL: 0x00000998
     */
    static void *RpcHandler(unsigned int function, void *buffer, int size);

    /** Request queue of the RPC server. */
    static sceSifQueueData sQueue;

    /** The RPC server. */
    static sceSifServeData sServer;

    /** Argument buffer of the RPC server. */
    alignas(16) static NetCnfRequest sBuffer;
};

/**
 * Module entry. A negative argument count unloads the module.
 *
 * @param argc Argument count, negated to unload.
 * @param argv Arguments.
 * @return A ModuleStartResult.
 * @ghidraAddress NTSC-U/C: 0x00000000
 * @ghidraAddress PAL: 0x00000000
 */
extern "C" int start(int argc, char **argv);

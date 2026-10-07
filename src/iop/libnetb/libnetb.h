#pragma once

#include <kernel.h>

#include "libnetb/libnetconfig.h"

/**
 * The Libnet module serves the inet and inetctl libraries to the EE through a multi-client SIF RPC
 * server. Most RPC functions forward one library call. Others queue the inetctl events and run
 * asynchronous transfers on connections (see AsyncInfo). Its arguments are `-verbose` and
 * `-send_delay=<milliseconds>`.
 *
 * The module was built without RTTI. The name comes from the module's library name.
 */
class Libnet {
public:
    /** SIF RPC server identifier of the module. */
    static constexpr unsigned int kRpcServerId = 0x80001201;

    /** RPC function numbers, with the record types of their arguments and replies. */
    enum Function {
        kFunctionInet6 = 1,            /*!< LibnetParamArgs to inet_6(). */
        kFunctionInet7 = 2,            /*!< LibnetConnectionArgs to inet_7(). */
        kFunctionInet8 = 3,            /*!< LibnetConnectionArgs to inet_8(). */
        kFunctionRecv = 4,             /*!< LibnetTransferArgs, LibnetRecvReply. */
        kFunctionSend = 5,             /*!< LibnetSendArgs, LibnetFlagsReply. */
        kFunctionName2Address = 6,     /*!< LibnetName2AddressArgs, LibnetAddressReply. */
        kFunctionAddress2String = 7,   /*!< LibnetAddress2StringArgs, LibnetAddress2StringReply. */
        kFunctionInet24 = 8,           /*!< LibnetBufferArgs, LibnetBufferReply. */
        kFunctionInterfaceControl = 9, /*!< LibnetControlArgs, LibnetControlReply. */
        kFunctionInet27 = 10,          /*!< LibnetBufferArgs, LibnetBufferReply. */
        kFunctionInet30 = 11,          /*!< LibnetBufferArgs, LibnetBufferReply. */
        kFunctionInet36 = 12,          /*!< LibnetValueArgs to inet_36(). */
        kFunctionRecvFrom = 13,        /*!< LibnetTransferArgs, LibnetRecvFromReply. */
        kFunctionSendTo = 14,          /*!< LibnetSendToArgs, LibnetFlagsReply. */
        kFunctionInet11 = 15,          /*!< LibnetConnectionArgs to inet_11(). */
        kFunctionInet41 = 16,          /*!< No arguments, to inet_41(). */
        kFunctionInet38 = 17,          /*!< LibnetPairArgs, LibnetBufferReply. */
        kFunctionInet14 = 18,          /*!< LibnetInet14Args to inet_14(). */
        kFunctionControl = 19,         /*!< LibnetControlArgs, LibnetControlReply. */
        kFunctionInet16 = 20,          /*!< LibnetPairArgs, LibnetOffsetBufferReply. */
        kFunctionOpenEvents = 30,      /*!< Start queueing the inetctl events. */
        kFunctionCloseEvents = 31,     /*!< Stop queueing the inetctl events. */
        kFunctionInetCtl4 = 32,        /*!< LibnetEnvArgs to inetctl_4(). */
        kFunctionWaitInterfaceUp = 33, /*!< Wait for InetEventQueue::kInterfaceUp. */
        kFunctionWaitInterfaceDown = 34, /*!< Wait for InetEventQueue::kInterfaceDown. */
        kFunctionGetEvent = 35,          /*!< LibnetEventReply. */
        kFunctionInetCtl5 = 50,          /*!< LibnetValueArgs to inetctl_5(). */
        kFunctionInetCtl6 = 51,          /*!< LibnetValueArgs to inetctl_6(). */
        kFunctionInetCtl7 = 52,          /*!< LibnetValueArgs to inetctl_7(). */
        kFunctionGetState = 53,          /*!< LibnetValueArgs, LibnetStateReply. */
        kFunctionStartAsyncRead = 100,   /*!< An AsyncInfo to receive with. */
        kFunctionStartAsyncSend = 101,   /*!< An AsyncInfo to send with. */
        kFunctionCloseAsync = 102,       /*!< LibnetCloseAsyncArgs. */
        kFunctionDumpState = 103,        /*!< Print the state of the module. */
    };

    /**
     * Copy a buffer to EE memory by SIF DMA and wait for the transfer.
     *
     * @param data Source.
     * @param destination Destination in EE memory.
     * @param size Byte count.
     * @return Zero, 2 for a null argument or no bytes, or 4 when the DMA queue is full.
     * @ghidraAddress NTSC-U/C: 0x00001240
     * @ghidraAddress PAL: 0x00001220
     */
    static int SendToEe(const void *data, void *destination, int size);

    /**
     * Print an inet error code and its description.
     *
     * @param error A #sceInetError, or a nonnegative value for no error.
     * @ghidraAddress NTSC-U/C: 0x0000343c
     * @ghidraAddress PAL: 0x000032ec
     */
    static void PrintInetError(int error);

    /**
     * Create and start a thread with an argument.
     *
     * @param param Parameters with the attribute and the option set.
     * @param name Thread name for messages.
     * @param entry Entry point.
     * @param stackSize Stack size.
     * @param priority Priority.
     * @param argument Argument of the entry point.
     * @return The thread identifier, or -1.
     * @ghidraAddress NTSC-U/C: 0x0000284c
     * @ghidraAddress PAL: 0x00002704
     */
    static int CreateNamedThread(ThreadParam *param,
                                 const char *name,
                                 void (*entry)(void *),
                                 int stackSize,
                                 int priority,
                                 void *argument);

    /**
     * Read the version text.
     *
     * @return sVersion.
     * @ghidraAddress NTSC-U/C: 0x00000b50
     * @ghidraAddress PAL: 0x00000b40
     */
    static const char *GetVersion();

    /**
     * Write the build date and time as `MM.DD.YYYY.hh.mm.ss`.
     *
     * @param text Receives the text, or null.
     * @param size Size of @p text.
     * @return True when @p text is not null.
     * @ghidraAddress NTSC-U/C: 0x00000b5c
     * @ghidraAddress PAL: 0x00000b4c
     */
    static bool FormatBuildDate(char *text, int size);

    /**
     * Print the delay between sends.
     *
     * @ghidraAddress NTSC-U/C: 0x000011fc
     * @ghidraAddress PAL: 0x000011dc
     */
    static void PrintSendDelay();

    /**
     * Read the module arguments into sConfig.
     *
     * @param argc Argument count.
     * @param argv Arguments.
     * @ghidraAddress NTSC-U/C: 0x00000a38
     * @ghidraAddress PAL: 0x00000a34
     */
    static void ParseArguments(int argc, char **argv);

    /**
     * Start the thread that registers the RPC server.
     *
     * @return #RESIDENT_END, or #NO_RESIDENT_END when the thread does not start.
     * @ghidraAddress NTSC-U/C: 0x000009b0
     * @ghidraAddress PAL: 0x000009b0
     */
    static int StartRpcThread();

    /** Nonzero for messages about each event and retry. Set by `-verbose`. */
    static int sVerbose;

    /** Version text. */
    static const char *sVersion;

    /** Microseconds between sends. */
    static int sSendDelay;

    /** Settings from the module arguments. */
    static LibnetConfig sConfig;

private:
    /**
     * Thread entry. Register the RPC server with the multi-client RPC library.
     *
     * @ghidraAddress NTSC-U/C: 0x00000974
     * @ghidraAddress PAL: 0x00000974
     */
    static void RpcThread();

    /**
     * Serve one request.
     *
     * @param function A Function.
     * @param buffer Arguments, overwritten by the reply.
     * @param size Size of the request.
     * @return @p buffer.
     * @ghidraAddress NTSC-U/C: 0x000003f8
     * @ghidraAddress PAL: 0x000003f8
     */
    static void *RpcHandler(unsigned int function, void *buffer, int size);

    /**
     * Serve the asynchronous transfer functions and the state dump.
     *
     * @param function A Function from #kFunctionStartAsyncRead.
     * @param buffer Arguments, overwritten by the reply.
     * @return @p buffer.
     * @ghidraAddress NTSC-U/C: 0x00000d18
     * @ghidraAddress PAL: 0x00000d08
     */
    static void *HandleAsync(unsigned int function, void *buffer);

    /**
     * Print the state of each connection of the receiving list.
     *
     * @ghidraAddress NTSC-U/C: 0x0000251c
     * @ghidraAddress PAL: 0x00002434
     */
    static void DumpConnections();

    /**
     * Wait for an interface state, as #kFunctionWaitInterfaceUp and #kFunctionWaitInterfaceDown do.
     *
     * @param state The awaited InetEventQueue::InterfaceState.
     * @param buffer Request buffer that receives the reply.
     */
    static inline void WaitInterfaceState(int state, void *buffer);
};

/**
 * Module entry. Read the arguments, print the version, and start the RPC server thread.
 *
 * @param argc Argument count.
 * @param argv Arguments.
 * @return A ModuleStartResult.
 * @ghidraAddress NTSC-U/C: 0x00000b00
 * @ghidraAddress PAL: 0x00000afc
 */
extern "C" int start(int argc, char **argv);

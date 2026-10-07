#pragma once

#include <inetctl.h>
#include <sifrpc.h>

#include "eznctl_s/netcnfifdata.h"
#include "eznctl_s/netcnfifenv.h"

/**
 * The eznetctl module connects the network for the EE through a SIF RPC server. It applies a
 * NetcnfifData the EE writes, or a combination loaded from a configuration file, waits for the
 * interface to start, and reports its state.
 *
 * The module was built without RTTI. The name is inferred from the module's name.
 */
class EzNetCtl {
public:
    /** SIF RPC server identifier of the module. */
    static constexpr unsigned int kRpcServerId = 0x75488909;

    /** RPC function numbers. */
    enum Function {
        kFunctionGetDataAddress = 0, /*!< Report the address of sData for the EE to write. */
        kFunctionApplyData = 1,      /*!< Apply sData and connect. */
        kFunctionLoadEntry = 2,      /*!< Load a combination and connect. */
        kFunctionGetStatus = 3,      /*!< NetCtlStatus::Update(). */
        kFunctionInetCtl7 = 4,       /*!< inetctl_7() on NetCtlRequest::mInterfaceId. */
        kFunctionInetCtl5 = 5,       /*!< inetctl_5() on NetCtlRequest::mInterfaceId. */
        kFunctionInetCtl6 = 6,       /*!< inetctl_6() on NetCtlRequest::mInterfaceId. */
        kFunctionLookUpName = 7,     /*!< LookUpName(). */
        kFunctionNetCheck = 8,       /*!< Print a message and report -1. */
#ifdef VIDEO_STANDARD_PAL
        kFunctionSetNetChecked = 9, /*!< Set sNetChecked and report -1. */
#endif
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

    /** Interface identifier of the started Ethernet interface, or zero. */
    static int sEthernetId;

    /** Interface identifier of the started PPP interface, or zero. */
    static int sPppId;

#ifdef VIDEO_STANDARD_PAL
    /** Nonzero once the EE calls #kFunctionSetNetChecked. */
    static int sNetChecked;
#endif

private:
    /** Event flag bit the event handler sets when an interface starts. */
    static constexpr unsigned int kInterfaceStartedBit = 0x0001;

    /** Shift of the interface identifier in the event flag. */
    static constexpr int kInterfaceIdShift = 16;

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
     * Register the event handler and create the event flag.
     *
     * @return Zero, or -1 when the event flag cannot be created.
     * @ghidraAddress NTSC-U/C: 0x00000230
     * @ghidraAddress PAL: 0x00000230
     */
    static int Initialize();

    /**
     * Remove the event handler and delete the event flag.
     *
     * @return The result of DeleteEventFlag(), or zero.
     * @ghidraAddress NTSC-U/C: 0x000002c8
     * @ghidraAddress PAL: 0x000002c8
     */
    static int Finalize();

    /**
     * Alarm handler. End the wait of Connect().
     *
     * @param common Address of the thread identifier to release.
     * @return Zero, to stop the alarm.
     * @ghidraAddress NTSC-U/C: 0x0000064c
     * @ghidraAddress PAL: 0x00000644
     */
    static unsigned int AlarmHandler(void *common);

    /**
     * Event handler. Record a started interface and wake Connect().
     *
     * @param interfaceId Interface identifier.
     * @param event An #InetCtlEvent.
     * @ghidraAddress NTSC-U/C: 0x00000670
     * @ghidraAddress PAL: 0x00000668
     */
    static void EventHandler(int interfaceId, int event);

    /**
     * Start the interface of an environment and wait up to two seconds for it.
     *
     * @param env Environment.
     * @return The started interface identifier, the error of inetctl_4(), or -1 on timeout.
     * @ghidraAddress NTSC-U/C: 0x00000a54
     * @ghidraAddress PAL: 0x00000a4c
     */
    static int Connect(NetcnfifEnv *env);

    /**
     * Resolve a host name and write its address over it. The name becomes empty on failure.
     *
     * @param name Host name, at least 32 bytes long.
     * @param size Size of the request.
     * @return @p name.
     * @ghidraAddress NTSC-U/C: 0x00000b10
     * @ghidraAddress PAL: 0x00000b08
     */
    static char *LookUpName(char *name, int size);

    /**
     * Serve one request. The first word of the buffer receives the result, except for
     * #kFunctionGetStatus and #kFunctionLookUpName.
     *
     * @param function A Function.
     * @param buffer The request.
     * @param size Size of the request.
     * @return @p buffer.
     * @ghidraAddress NTSC-U/C: 0x00000c38
     * @ghidraAddress PAL: 0x00000c30
     */
    static void *RpcHandler(unsigned int function, void *buffer, int size);

    /** Size of sBuffer. */
    static constexpr int kRpcBufferSize = 0x140;

    /** Thread that runs the RPC server. */
    static int sThread;

    /** Event flag the event handler sets. */
    static int sEventFlag;

    /** Registration of EventHandler(). */
    static sceInetCtlEventHandler sEventHandler;

    /** Request queue of the RPC server. */
    static sceSifQueueData sQueue;

    /** The RPC server. */
    static sceSifServeData sServer;

    /** Argument buffer of the RPC server. */
    alignas(16) static unsigned char sBuffer[kRpcBufferSize];

    /** Environment each connection fills. */
    static NetcnfifEnv sEnv;

    /** Settings the EE writes for #kFunctionApplyData. */
    alignas(16) static NetcnfifData sData;
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

#pragma once

#include <libeznetcnf.h>

#include "netflow/netinet.h"
#include "os/string.h"

/**
 * The network connection of the console on the network configuration library.
 *
 * The RTTI includes the name and records NetInet as the base. The vtable is at `0x003d43f0` and the
 * object is 0x60 bytes. InitializeNetSubsystem() builds the one instance. The members start their
 * work as asynchronous tasks and report the results to the receivers as messages. The routines
 * other than the members of NetInet have inferred names.
 */
class InetImpl : public NetInet {
public:
    /** Values of mState. */
    enum State {
        kStateIdle = 0,       /*!< No interface is up or starting. */
        kStateStarting = 1,   /*!< The configuration is being loaded. */
        kStateConnecting = 2, /*!< The interface is coming up. */
        kStateConnected = 3,  /*!< The interface is up. */
    };

    /**
     * Construct the connection and make it TheNetInet.
     *
     * @ghidraAddress NTSC-U/C: 0x00253438
     * @ghidraAddress PAL: 0x0025c3e8
     */
    InetImpl();

    /**
     * Vtable slot 1. Release the connection.
     *
     * @ghidraAddress NTSC-U/C: 0x00253528
     * @ghidraAddress PAL: 0x0025c4e0
     */
    ~InetImpl() override;

    /**
     * Vtable slot 2. Start the network configuration program from the disc, ending the game.
     *
     * @ghidraAddress NTSC-U/C: 0x002535e0
     * @ghidraAddress PAL: 0x0025c598
     */
    void LaunchConfigTool() override;

    /**
     * Vtable slot 3.
     *
     * @param pSink The receiver of the result.
     * @ghidraAddress NTSC-U/C: 0x002536b0
     * @ghidraAddress PAL: 0x0025c628
     */
    void RequestConfigs(MsgSink *pSink) override;

    /**
     * Vtable slot 4.
     *
     * @param pSink The receiver of the result.
     * @param nConfig The identifier of the configuration.
     * @ghidraAddress NTSC-U/C: 0x002539d0
     * @ghidraAddress PAL: 0x0025c948
     */
    void Connect(MsgSink *pSink, int nConfig) override;

    /**
     * Vtable slot 5.
     *
     * @param pSink The receiver of the result.
     * @ghidraAddress NTSC-U/C: 0x00253c58
     * @ghidraAddress PAL: 0x0025cb98
     */
    void Disconnect(MsgSink *pSink) override;

    /**
     * Vtable slot 6.
     *
     * @param pSink The receiver of the result.
     * @param pszHost The host name.
     * @ghidraAddress NTSC-U/C: 0x00253c68
     * @ghidraAddress PAL: 0x0025cba8
     */
    void LookupHost(MsgSink *pSink, const char *pszHost) override;

    /**
     * Vtable slot 7. Forget the receiver of the pending lookup.
     *
     * @ghidraAddress NTSC-U/C: 0x00253c60
     * @ghidraAddress PAL: 0x0025cba0
     */
    void ClearPending() override;

    /**
     * Vtable slot 8.
     *
     * @param pSink The receiver of the result.
     * @ghidraAddress NTSC-U/C: 0x00253e58
     * @ghidraAddress PAL: 0x0025cda8
     */
    void CheckConnection(MsgSink *pSink) override;

    /**
     * Vtable slot 9.
     *
     * @return mConnectionType.
     * @ghidraAddress NTSC-U/C: 0x00397580
     * @ghidraAddress PAL: 0x00405e08
     */
    int GetConnectionType() override {
        return mConnectionType;
    }

#ifdef VIDEO_STANDARD_PAL
    /**
     * Vtable slot 10. Report mDNASPending.
     *
     * @return mDNASPending.
     * @ghidraAddress PAL: 0x0025cd98
     */
    int IsDNASPending() override;

    /**
     * Vtable slot 11. Clear mDNASPending.
     *
     * @ghidraAddress PAL: 0x0025cda0
     */
    void ClearDNASPending() override;
#endif

    /**
     * Vtable slot 10, and slot 12 in the European release.
     *
     * @ghidraAddress NTSC-U/C: 0x00253f90
     * @ghidraAddress PAL: 0x0025cee0
     */
    void RequestPowerOff() override;

    /**
     * Service the connection once per frame: report the disconnection, the progress of a
     * connection, and turn the console off when requested.
     *
     * @ghidraAddress NTSC-U/C: 0x00253fa0
     * @ghidraAddress PAL: 0x0025cef0
     */
    void Poll();

private:
    /**
     * Take the interface down and mark the connection offline.
     *
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x002535a8
     * @ghidraAddress PAL: 0x0025c560
     */
    static int DownInterface();

    /**
     * Asynchronous task. Read the list of configurations from the memory card.
     *
     * @return The number of combinations, or a negative error.
     * @ghidraAddress NTSC-U/C: 0x002536f0
     * @ghidraAddress PAL: 0x0025c668
     */
    static int ConfigsTask();

    /**
     * Completion of ConfigsTask().
     *
     * @param nResult The result of the task.
     * @ghidraAddress NTSC-U/C: 0x00253720
     * @ghidraAddress PAL: 0x0025c698
     */
    static void OnConfigsTaskDone(int nResult);

    /**
     * Send the configurations found to the receiver of RequestConfigs().
     *
     * @param nResult The result of ConfigsTask().
     * @ghidraAddress NTSC-U/C: 0x00253750
     * @ghidraAddress PAL: 0x0025c6c8
     */
    void DeliverConfigs(int nResult);

    /**
     * Asynchronous task. Load the configuration, the IOP modules it needs, and bring the interface
     * up.
     *
     * @return 0, or an InetConnectResultMsg::Result.
     * @ghidraAddress NTSC-U/C: 0x00253a30
     * @ghidraAddress PAL: 0x0025c9b0
     */
    static int ConnectTask();

    /**
     * Completion of ConnectTask().
     *
     * @param nResult The result of the task.
     * @ghidraAddress NTSC-U/C: 0x00253b10
     * @ghidraAddress PAL: 0x0025ca90
     */
    static void OnConnectTaskDone(int nResult);

    /**
     * Asynchronous task. Look up the host of LookupHost().
     *
     * @return The result of the lookup.
     * @ghidraAddress NTSC-U/C: 0x00253d08
     * @ghidraAddress PAL: 0x0025cc48
     */
    static int LookupTask();

    /**
     * Completion of LookupTask().
     *
     * @param nResult The result of the task.
     * @ghidraAddress NTSC-U/C: 0x00253d30
     * @ghidraAddress PAL: 0x0025cc70
     */
    static void OnLookupTaskDone(int nResult);

    /**
     * Asynchronous task. Read the state of the interface.
     *
     * @return The result of the query.
     * @ghidraAddress NTSC-U/C: 0x00253ee0
     * @ghidraAddress PAL: 0x0025ce30
     */
    static int StatusTask();

    /**
     * Completion of StatusTask().
     *
     * @param nResult The result of the task, unused.
     * @ghidraAddress NTSC-U/C: 0x00253f00
     * @ghidraAddress PAL: 0x0025ce50
     */
    static void OnStatusTaskDone(int nResult);

    /**
     * Add an IOP module to the modules the next LoadModules() loads.
     *
     * @param pszName The file name of the module.
     * @ghidraAddress NTSC-U/C: 0x002543d0
     * @ghidraAddress PAL: 0x0025d308
     */
    static void AddModule(const char *pszName);

    /**
     * Unload the loaded IOP modules that are no longer wanted, and load the wanted ones that are
     * not loaded.
     *
     * @return 0, or the error of the module that failed to load.
     * @ghidraAddress NTSC-U/C: 0x002545b0
     * @ghidraAddress PAL: 0x0025d4e8
     */
    static int LoadModules();

    /**
     * Choose the IOP modules, the connection type, and the timeout of a configuration, and load the
     * modules.
     *
     * @param pData The configuration.
     * @return The result of LoadModules().
     * @ghidraAddress NTSC-U/C: 0x00254968
     * @ghidraAddress PAL: 0x0025d8a0
     */
    static int SelectModules(const ezNetCnfData_t *pData);

    /**
     * The power-off callback of the drive. Request that Poll() turns the console off.
     *
     * @param pArg Unused.
     * @ghidraAddress NTSC-U/C: 0x00254b30
     */
    static void OnPowerOff(void *pArg);

    int mState;             /*!< One of State. */
    float mStartTime;       /*!< SystemMs() when the interface last progressed. */
    int mLinkStatus;        /*!< The link status reported last, 999 before the first report. */
    String mStatusText;     /*!< The status text reported last. */
    float mTimeout;         /*!< The milliseconds a connection may take, by connection type. */
    int mConnectionType;    /*!< One of NetInet::ConnectionType. */
    int mConfigId;          /*!< The configuration of Connect(). */
    int mPowerOffRequested; /*!< Non-zero when Poll() should turn the console off. */
    String mHost;           /*!< The host of LookupHost(). */
    int mLookupPending;     /*!< Non-zero while LookupTask() runs. */
#ifdef VIDEO_STANDARD_PAL
    int mDNASPending; /*!< Non-zero from Connect() until the DNAS authentication passes. */
#endif
    MsgSink *mConnectSink;    /*!< The receiver of Connect(), or null. */
    MsgSink *mLookupSink;     /*!< The receiver of LookupHost(), or null. */
    MsgSink *mCheckSink;      /*!< The receiver of CheckConnection(), or null. */
    MsgSink *mConfigsSink;    /*!< The receiver of RequestConfigs(), or null. */
    MsgSink *mDisconnectSink; /*!< The receiver of Disconnect(), or null. */

    /**
     * The instance the asynchronous tasks and the callbacks use.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0ab8
     * @ghidraAddress PAL: 0x0042f984
     */
    static InetImpl *sInstance;
};

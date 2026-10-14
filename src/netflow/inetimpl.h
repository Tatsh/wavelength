#pragma once

#include <libeznetcnf.h>

#include "netflow/netinet.h"
#include "os/string.h"

/**
 * The network connection of the console on the network configuration and control servers.
 *
 * The RTTI includes the name and records NetInet as the base. The vtable is at `0x003d43f0` and the
 * object is 0x60 bytes. The source file was `InetPS2.cpp`. InitializeNetSubsystem() builds the one
 * instance. The slow requests run as asynchronous tasks, whose routines reach the instance
 * through sInstance.
 */
class InetImpl : public NetInet {
public:
    /** Values of mState. */
    enum State {
        kStateIdle = 0,       /*!< No connection. */
        kStateConnecting = 1, /*!< The configuration is being loaded. */
        kStateStarting = 2,   /*!< The interface is coming up. */
        kStateUp = 3,         /*!< The interface is up. */
    };

    /**
     * Construct the connection, make it TheNetInet and sInstance, and bind the servers.
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
     * Vtable slot 2. Replace the game with the network configuration program of the disc.
     *
     * @ghidraAddress NTSC-U/C: 0x002535e0
     * @ghidraAddress PAL: 0x0025c598
     */
    void LaunchConfigTool() override;

    /**
     * Vtable slot 3. Start listing the configurations of the memory card in the first slot.
     *
     * @param pSink The receiver of the InetConfigsResultMsg.
     * @ghidraAddress NTSC-U/C: 0x002536b0
     * @ghidraAddress PAL: 0x0025c628
     */
    void RequestConfigs(MsgSink *pSink) override;

    /**
     * Vtable slot 4. Bring the interface down, then start bringing it up with a configuration.
     *
     * @param pSink The receiver of the InetConnectStatusMsg and InetConnectResultMsg.
     * @param nConfig The number of the configuration.
     * @ghidraAddress NTSC-U/C: 0x002539d0
     * @ghidraAddress PAL: 0x0025c948
     */
    void Connect(MsgSink *pSink, int nConfig) override;

    /**
     * Vtable slot 5. Request that the next poll outside a connection bring the interface down.
     *
     * @param pSink The receiver of the InetDisconnectResultMsg.
     * @ghidraAddress NTSC-U/C: 0x00253c58
     */
    void Disconnect(MsgSink *pSink) override;

    /**
     * Vtable slot 6. Start looking up the address of a host, unless a lookup is running.
     *
     * @param pSink The receiver of the InetDNSLookupResultMsg.
     * @param pszHost The host name.
     * @ghidraAddress NTSC-U/C: 0x00253c68
     * @ghidraAddress PAL: 0x0025cba8
     */
    void LookupHost(MsgSink *pSink, const char *pszHost) override;

    /**
     * Vtable slot 7. Forget the receiver of the running lookup.
     *
     * @ghidraAddress NTSC-U/C: 0x00253c60
     */
    void ClearPending() override;

    /**
     * Vtable slot 8. Start reading the state of the interface, or report at once that it is down.
     *
     * @param pSink The receiver of the InetCheckConnectionResultMsg.
     * @ghidraAddress NTSC-U/C: 0x00253e58
     * @ghidraAddress PAL: 0x0025cda8
     */
    void CheckConnection(MsgSink *pSink) override;

    /**
     * Vtable slot 9. Report mConnectionType.
     *
     * Inline.
     *
     * @return One of NetInet::ConnectionType.
     * @ghidraAddress NTSC-U/C: 0x00397580
     */
    int GetConnectionType() override {
        return mConnectionType;
    }

    /**
     * Vtable slot 10. Set mPowerOff.
     *
     * @ghidraAddress NTSC-U/C: 0x00253f90
     */
    void PowerOff() override;

    /**
     * Follow the interface while it comes up, report a disconnection, and switch the console off
     * once mPowerOff is set.
     *
     * @ghidraAddress NTSC-U/C: 0x00253fa0
     */
    void Poll();

    /**
     * The connection the asynchronous routines report to, or null.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0ab8
     */
    static InetImpl *sInstance;

    int mState;               /*!< One of State. */
    float mStartMs;           /*!< The SystemMs() reading of the last progress of the interface. */
    int mLinkStatus;          /*!< The link state the last status reported. */
    String mMessage;          /*!< The description the last status reported. */
    float mTimeoutMs;         /*!< The longest wait for progress of the interface. */
    int mConnectionType;      /*!< One of NetInet::ConnectionType. */
    int mConfig;              /*!< The number of the configuration of the connection. */
    int mPowerOff;            /*!< Non-zero when the next poll switches the console off. */
    String mHostName;         /*!< The host of the running lookup. */
    int mLookupPending;       /*!< Non-zero while a lookup runs. */
    MsgSink *mConnectSink;    /*!< The receiver of Connect(), or null. */
    MsgSink *mLookupSink;     /*!< The receiver of LookupHost(), or null. */
    MsgSink *mCheckSink;      /*!< The receiver of CheckConnection(), or null. */
    MsgSink *mConfigsSink;    /*!< The receiver of RequestConfigs(), or null. */
    MsgSink *mDisconnectSink; /*!< The receiver of Disconnect(), or null. */

private:
    /**
     * Bring the interface down and forget the connection.
     *
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x002535a8
     * @ghidraAddress PAL: 0x0025c560
     */
    static int StopInterface();

    /**
     * The asynchronous task of RequestConfigs(), which lists the configurations of `mc0:`.
     *
     * @return The result of the server.
     * @ghidraAddress NTSC-U/C: 0x002536f0
     * @ghidraAddress PAL: 0x0025c668
     */
    static int ListConfigs();

    /**
     * The end of ListConfigs(), which passes the result to ReportConfigs() of sInstance.
     *
     * @param nResult The result of ListConfigs().
     * @ghidraAddress NTSC-U/C: 0x00253720
     * @ghidraAddress PAL: 0x0025c698
     */
    static void OnConfigsListed(int nResult);

    /**
     * Send the configurations ListConfigs() found to mConfigsSink.
     *
     * @param nResult The result of ListConfigs().
     * @ghidraAddress NTSC-U/C: 0x00253750
     * @ghidraAddress PAL: 0x0025c6c8
     */
    void ReportConfigs(int nResult);

    /**
     * The asynchronous task of Connect(), which loads the configuration, loads its IOP modules,
     * and starts bringing the interface up.
     *
     * @return 0, or a InetConnectResultMsg::Result.
     * @ghidraAddress NTSC-U/C: 0x00253a30
     * @ghidraAddress PAL: 0x0025c9b0
     */
    static int OpenConfig();

    /**
     * The end of OpenConfig(), which starts following the interface or reports the failure.
     *
     * @param nResult The result of OpenConfig().
     * @ghidraAddress NTSC-U/C: 0x00253b10
     */
    static void OnConfigOpened(int nResult);

    /**
     * The asynchronous task of LookupHost().
     *
     * @return The result of the server.
     * @ghidraAddress NTSC-U/C: 0x00253d08
     * @ghidraAddress PAL: 0x0025cc48
     */
    static int LookUpName();

    /**
     * The end of LookUpName(), which reports the address.
     *
     * @param nResult The result of LookUpName().
     * @ghidraAddress NTSC-U/C: 0x00253d30
     * @ghidraAddress PAL: 0x0025cc70
     */
    static void OnNameLookedUp(int nResult);

    /**
     * The asynchronous task of CheckConnection(), which reads the state of the interface.
     *
     * @return The result of the server.
     * @ghidraAddress NTSC-U/C: 0x00253ee0
     * @ghidraAddress PAL: 0x0025ce30
     */
    static int ReadStatus();

    /**
     * The end of ReadStatus(), which reports whether the interface is up.
     *
     * @param nResult The result of ReadStatus(), not read.
     * @ghidraAddress NTSC-U/C: 0x00253f00
     * @ghidraAddress PAL: 0x0025ce50
     */
    static void OnStatusRead(int nResult);

    /**
     * Add an IOP module to the modules the configuration needs.
     *
     * @param pszName The file of the module.
     * @ghidraAddress NTSC-U/C: 0x002543d0
     * @ghidraAddress PAL: 0x0025d308
     */
    static void WantModule(const char *pszName);

    /**
     * Unload the loaded IOP modules the configuration does not need, and load the ones it needs
     * from the `optional_modules` of the `net` configuration.
     *
     * @return 0, or the error of the first module that did not load.
     * @ghidraAddress NTSC-U/C: 0x002545b0
     * @ghidraAddress PAL: 0x0025d4e8
     */
    static int SyncModules();

    /**
     * Choose the IOP modules, the kind of connection, and the timeout of a configuration, and
     * load the modules.
     *
     * @param pData The configuration.
     * @return The result of SyncModules().
     * @ghidraAddress NTSC-U/C: 0x00254968
     * @ghidraAddress PAL: 0x0025d8a0
     */
    static int SetupModules(const ezNetCnfData_t *pData);

    /**
     * The power-off callback of the drive, which sets mPowerOff of sInstance.
     *
     * @param pArg Not read.
     * @ghidraAddress NTSC-U/C: 0x00254b30
     */
    static void OnPowerOffButton(void *pArg);
};

#pragma once

#include "netflow/netinet.h"

/**
 * The network connection of the console on the network configuration library.
 *
 * The RTTI includes the name and records NetInet as the base. The vtable is at `0x003d43f0` and the
 * object is 0x60 bytes. InitializeNetSubsystem() builds the one instance. Only the members the
 * network subsystem uses are declared.
 */
class InetImpl : public NetInet {
public:
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
     * Vtable slot 2.
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
     * @param pSink The receiver, or null.
     * @ghidraAddress NTSC-U/C: 0x00253c58
     */
    void SetSink(MsgSink *pSink) override;

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
     * Vtable slot 7.
     *
     * @ghidraAddress NTSC-U/C: 0x00253c60
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
     * @return The kind of connection.
     * @ghidraAddress NTSC-U/C: 0x00397580
     */
    int GetConnectionType() override;

    /**
     * Vtable slot 10.
     *
     * @ghidraAddress NTSC-U/C: 0x00253f90
     */
    void SetReady() override;

    /**
     * Service the connection once per frame.
     *
     * @ghidraAddress NTSC-U/C: 0x00253fa0
     */
    void Poll();
};

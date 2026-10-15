#pragma once

#ifdef VIDEO_STANDARD_PAL

#include <libdnas2.h>

#include "netflow/nettask.h"

/**
 * Step of the lobby connection of the European release that authenticates the title with DNAS.
 *
 * The RTTI includes the name and records NetTask as the base. The object is 32 bytes and its vtable
 * is at `0x0046ace8`. The step reports DNASStatusMsg to the receiver of the lobby connection while
 * the authentication runs, and fails with LobbyConnectResultMsg::kResultDNASFailed. A connection
 * that already passed the authentication skips it.
 */
class DNASTask : public NetTask {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress PAL: 0x004058d0
     */
    ~DNASTask() override = default;

    /**
     * Report the localisation tokens of the error of the last failed authentication.
     *
     * @param pnSubCode Receives the error code of the library.
     * @param ppszError Receives the token of the error.
     * @param ppszHint Receives the token of the advice under the error.
     * @ghidraAddress PAL: 0x0025c230
     */
    static void GetError(int *pnSubCode, const char **ppszError, const char **ppszHint);

    /**
     * The status code of the last poll, or sceDNAS2_SS_SERVER_ERROR when the poll failed.
     *
     * @ghidraAddress PAL: 0x0042f958
     */
    static int sStatusCode;

    /**
     * The error of the last poll.
     *
     * @ghidraAddress PAL: 0x0042f95c
     */
    static int sSubCode;

protected:
    /**
     * Start the authentication, or finish at once when the connection needs none.
     *
     * @ghidraAddress PAL: 0x0025bde0
     */
    void OnStart() override;

    /**
     * Abort a running authentication and report its end.
     *
     * @ghidraAddress PAL: 0x0025bfd0
     */
    void OnStop() override;

    /**
     * Poll the progress, and finish once the authentication passes or fails.
     *
     * @ghidraAddress PAL: 0x0025c0c8
     */
    void OnPoll() override;

private:
    float mStartMs;    /*!< SystemMs() when the authentication started. */
    int mLastCode;     /*!< The status code the last poll printed. */
    int mLastSubCode;  /*!< The error the last poll printed. */
    int mLastProgress; /*!< The progress the last poll printed. */
    int mSkipped;      /*!< Non-zero when the connection needed no authentication. */
};

#endif

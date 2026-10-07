#ifndef INETCTL_H
#define INETCTL_H

#include <netcnf.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Interface management of the inetctl library. */

/** Events an event handler receives. */
enum InetCtlEvent {
    INETCTL_EVENT_START = 6, /*!< An interface started. */
};

/** Interface states sceInetCtlGetState() reports. */
enum InetCtlState {
    INETCTL_STATE_DETACHED = 0,      /*!< Detached. */
    INETCTL_STATE_CONNECTING = 1,    /*!< Connecting. */
    INETCTL_STATE_RETRYING = 2,      /*!< Retrying. */
    INETCTL_STATE_CONNECTED = 3,     /*!< Connected. */
    INETCTL_STATE_DISCONNECTING = 4, /*!< Disconnecting. */
    INETCTL_STATE_DISCONNECTED = 5,  /*!< Disconnected. */
};

/** A registered event handler, linked into a list. */
typedef struct sceInetCtlEventHandler {
    struct sceInetCtlEventHandler *next;         /*!< Next handler. */
    struct sceInetCtlEventHandler *previous;     /*!< Previous handler. */
    void (*handler)(int interfaceId, int event); /*!< Receives each #InetCtlEvent. */
    unsigned int reserved;                       /* +0x0c */
} sceInetCtlEventHandler;

/**
 * Export 4. The eznetctl module passes a loaded environment and then waits for an interface to
 * start.
 *
 * @param env Environment.
 * @return Zero, or an error code.
 */
int inetctl_4(sceNetCnfEnv *env);

/**
 * Export 5. The eznetctl module calls it for RPC function 5.
 *
 * @param interfaceId Interface identifier.
 * @return A result.
 */
int inetctl_5(int interfaceId);

/**
 * Export 6. The eznetctl module calls it for RPC function 6.
 *
 * @param interfaceId Interface identifier.
 * @return A result.
 */
int inetctl_6(int interfaceId);

/**
 * Export 7. The eznetctl module calls it for RPC function 4.
 *
 * @param interfaceId Interface identifier.
 * @return A result.
 */
int inetctl_7(int interfaceId);

/**
 * Register an event handler. Export 8.
 *
 * @param handler Handler record.
 * @return Zero, or a negative error code.
 */
int sceInetCtlRegisterEventHandler(sceInetCtlEventHandler *handler);

/**
 * Remove an event handler. Export 9.
 *
 * @param handler Handler record.
 * @return Zero, or a negative error code.
 */
int sceInetCtlUnregisterEventHandler(sceInetCtlEventHandler *handler);

/**
 * Read the state of an interface. Export 10.
 *
 * @param interfaceId Interface identifier.
 * @param state Receives an #InetCtlState.
 * @return Zero, or a negative error code.
 */
int sceInetCtlGetState(int interfaceId, int *state);

#ifdef __cplusplus
}
#endif

#endif

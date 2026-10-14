#ifndef MSIFRPC_H
#define MSIFRPC_H

#include <sifrpc.h>

#ifdef __cplusplus
extern "C" {
#endif

/** The client of the multi-threaded SIF RPC servers the msifrpc module runs on the IOP. */

/** A client of one msifrpc server. */
typedef struct {
    void *paddr;              /*!< The packet of the request in flight. */
    unsigned int pid;         /*!< The identifier of that packet. */
    int tid;                  /*!< The semaphore the caller waits on, or -1. */
    unsigned int nReserved12; /*!< Not written by the library. */
    unsigned int command;     /*!< The server identifier. */
    void *buff;               /*!< The IOP argument buffer the bind reply reports. */
    void *cbuff;              /*!< The IOP cancel buffer the bind reply reports. */
    sceSifEndFunc func;       /*!< The completion callback of the call in flight, or null. */
    void *para;               /*!< The argument of the completion callback. */
    void *serve;              /*!< The IOP server record the bind reply reports, or null. */
    int sema;                 /*!< The semaphore that serialises the requests of the client. */
    int nUnbindResult;        /*!< The result the unbind reply reports. */
    int buffersize;           /*!< The argument buffer size the server thread is given. */
    int stacksize;            /*!< The stack size of the server thread. */
    int priority;             /*!< The priority of the server thread. */
    unsigned int nReserved60; /*!< Not written by the library. */
} sceSifMClientData;

/**
 * Register the completion handler and tell the msifrpc module that the client is ready. A second
 * call does nothing.
 *
 * @param mode Not used.
 * @ghidraAddress NTSC-U/C: 0x00316310
 * @ghidraAddress PAL: 0x00382bc8
 */
void sceSifMInitRpc(unsigned int mode);

/**
 * Bind a client to an msifrpc server and wait for the reply.
 *
 * @param cd The client.
 * @param command The server identifier.
 * @param mode Must be zero.
 * @param buffersize The argument buffer size the server thread is given.
 * @param stacksize The stack size of the server thread.
 * @param priority The priority of the server thread.
 * @return Zero, -1 for a nonzero mode or when no packet is free, -2 when the request cannot be
 * sent, or -3 when no semaphore can be created.
 * @ghidraAddress NTSC-U/C: 0x003165b8
 * @ghidraAddress PAL: 0x00382e70
 */
int sceSifMBindRpcParam(sceSifMClientData *cd,
                        unsigned int command,
                        unsigned int mode,
                        int buffersize,
                        int stacksize,
                        int priority);

/**
 * Unbind a client from its server and wait for the reply.
 *
 * @param cd The client.
 * @param mode Must be zero.
 * @return The result the server reports, -1 for a nonzero mode or when no packet is free, -2 when
 * the request cannot be sent, or -3 when no semaphore can be created.
 * @ghidraAddress NTSC-U/C: 0x00316768
 * @ghidraAddress PAL: 0x00383020
 */
int sceSifMUnBindRpc(sceSifMClientData *cd, unsigned int mode);

/**
 * Call a function of a bound server and wait for the reply.
 *
 * Both buffers must start on a 64-byte boundary.
 *
 * @param cd The bound client.
 * @param rpc_number The function number.
 * @param mode Must be zero.
 * @param send The argument buffer.
 * @param ssize The size of the arguments in bytes.
 * @param receive The reply buffer.
 * @param rsize The size of the reply in bytes.
 * @param end_function The completion callback, or null.
 * @param end_param The argument of the completion callback.
 * @return Zero, -1 for a nonzero mode, a misaligned buffer, or when no packet is free, -2 when the
 * request cannot be sent, or -3 when no semaphore can be created.
 * @ghidraAddress NTSC-U/C: 0x00316888
 * @ghidraAddress PAL: 0x00383140
 */
int sceSifMCallRpc(sceSifMClientData *cd,
                   unsigned int rpc_number,
                   unsigned int mode,
                   void *send,
                   int ssize,
                   void *receive,
                   int rsize,
                   sceSifEndFunc end_function,
                   void *end_param);

#ifdef __cplusplus
}
#endif

#endif

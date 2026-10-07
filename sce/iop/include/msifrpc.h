#ifndef MSIFRPC_H
#define MSIFRPC_H

#include <sifrpc.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * SIF remote procedure calls of the msifrpc library for several clients at once. An export whose
 * role the calling modules do not reveal is named after its export number.
 */

/** Server record of the library. Its members are not identified. */
typedef struct sceSifMServeData {
    unsigned int reserved[6]; /*!< Never touched by the libnetb module. */
} sceSifMServeData;

/**
 * Export 4. The libnetb module calls it once with zero before it serves requests.
 *
 * @param mode Zero.
 */
void msifrpc_4(int mode);

/**
 * Export 17. The libnetb module registers its request handler through it and then returns.
 *
 * @param serve Server record.
 * @param command Server identifier.
 * @param handler Request handler.
 * @param mode Zero.
 * @return A result the libnetb module discards.
 */
int msifrpc_17(sceSifMServeData *serve, unsigned int command, sceSifRpcFunc handler, int mode);

#ifdef __cplusplus
}
#endif

#endif

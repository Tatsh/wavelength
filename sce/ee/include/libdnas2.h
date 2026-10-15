#ifndef LIBDNAS2_H
#define LIBDNAS2_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Client of the DNAS 2 authentication service of the European release, for a console without a
 * hard disk. The library authenticates the disc and the title with the DNAS server over SSL in its
 * threads, and the caller polls the progress with sceDNAS2GetStatus(). The names of the status
 * codes, the fields, and the parameters are inferred.
 */

/** The library is not initialised. */
#define sceDNAS2_ERR_NOT_INITIALIZED (-8)

/** Values of sceDNAS2Status_t::code. */
typedef enum {
    sceDNAS2_SS_COMPLETE = 5,      /*!< The authentication succeeded. */
    sceDNAS2_SS_SERVER_ERROR = 6,  /*!< The server or the library refused it; see sub_code. */
    sceDNAS2_SS_NETWORK_ERROR = 7, /*!< The network failed. */
} sceDNAS2StatusCode;

/** A block of data with its size. */
typedef struct {
    const unsigned char *data; /*!< The bytes. */
    int size;                  /*!< The size in bytes. */
} sceDNAS2Data_t;

/** The identity of the title the server authenticates. */
typedef struct {
    int lineType;              /*!< The kind of connection. The game passes 1. */
    sceDNAS2Data_t passPhrase; /*!< The pass phrase of the title, 8 bytes. */
    sceDNAS2Data_t authData;   /*!< The authentication data of the title, 65536 bytes. */
} sceDNAS2TitleAuthData_t;

/** The time limits of the authentication. */
typedef struct {
    int timeout; /*!< The limit of the whole authentication in seconds. */
    int retry;   /*!< The second limit the game passes. */
} sceDNAS2TimeoutInfo_t;

/** The progress sceDNAS2GetStatus() reports, 16 bytes. */
typedef struct {
    int code;                 /*!< One of sceDNAS2StatusCode, or an earlier step. */
    int subCode;              /*!< The error of a failed authentication. */
    int progress;             /*!< The progress of the current step. */
    unsigned char feature[4]; /*!< Not read by the game. */
} sceDNAS2Status_t;

/**
 * Initialise the library for a console without a hard disk and start its threads.
 *
 * @param pAuthData The identity of the title.
 * @param priority The priority of the threads of the library.
 * @param debugFlag Zero in the game. A positive value is stored for the worker thread.
 * @param sslFlag Zero in the game. A value outside -1055 to 0 is refused with -9.
 * @param pTimeout The time limits, or null.
 * @return Zero, or a negative error.
 * @ghidraAddress PAL: 0x00311e80
 */
int sceDNAS2InitNoHDD(sceDNAS2TitleAuthData_t *pAuthData,
                      int priority,
                      int debugFlag,
                      int sslFlag,
                      sceDNAS2TimeoutInfo_t *pTimeout);

/**
 * Start the authentication over the network.
 *
 * @return Zero, or a negative error.
 * @ghidraAddress PAL: 0x00312038
 */
int sceDNAS2AuthNetStart(void);

/**
 * Stop a running authentication.
 *
 * @return Zero, or a negative error.
 * @ghidraAddress PAL: 0x00311eb8
 */
int sceDNAS2Abort(void);

/**
 * Report the progress of the authentication.
 *
 * @param pStatus Receives the progress.
 * @return Zero, or sceDNAS2_ERR_NOT_INITIALIZED.
 * @ghidraAddress PAL: 0x00311fd0
 */
int sceDNAS2GetStatus(sceDNAS2Status_t *pStatus);

#ifdef __cplusplus
}
#endif

#endif

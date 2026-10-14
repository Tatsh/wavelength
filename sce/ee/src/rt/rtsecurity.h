#ifndef RTSECURITY_H
#define RTSECURITY_H

#include "rt/rtlargeint.h"
#include "rt/rtrc4.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Keys and packet encryption of the SCE-RT library. Each connection has a key record, and a record
 * per peer under it. A packet is encrypted with RC4 under the key of the peer, the key of the
 * connection, or the RSA public key of the peer, and carries a header word that names the method
 * in its top 3 bits and holds 29 bits of the SHA-1 digest of the plain packet below them.
 */

/** The identifier of the record of the server connection. */
#define RT_SECURITY_SERVER_ID (-1)

/** The bit that marks the identifier of a listening connection. */
#define RT_SECURITY_LISTEN_ID 0x80000000

/** The peer that stands for every peer of a connection. */
#define RT_SECURITY_ALL_PEERS 0xffff

/** Methods in the top 3 bits of a packet header. */
#define RT_SECURITY_METHOD_MASK 0xe0000000U       /*!< The bits of the method. */
#define RT_SECURITY_DIGEST_MASK 0x1fffffffU       /*!< The bits of the digest. */
#define RT_SECURITY_METHOD_NONE 0x00000000U       /*!< Plain. */
#define RT_SECURITY_METHOD_CONNECTION 0x20000000U /*!< RC4 under the key of the connection. */
#define RT_SECURITY_METHOD_INVALID 0x40000000U    /*!< Rejected. */
#define RT_SECURITY_METHOD_PEER 0x60000000U       /*!< RC4 under the key of the peer. */
#define RT_SECURITY_METHOD_PUBLIC 0xe0000000U     /*!< RSA under the public key of the peer. */

/** The results of the routines. */
#define RT_SECURITY_RESULT_FAILED (-1)   /*!< The record is missing, or a key is empty. */
#define RT_SECURITY_RESULT_MISMATCH (-2) /*!< The digest of a decrypted packet differs. */
#define RT_SECURITY_RESULT_NO_KEYS (-3)  /*!< No key pair is stored. */

/** An RSA key pair. */
typedef rtNetKeyPair RtKeyPair;

/** The keys of one peer of a connection. */
typedef struct RtPeerKeys {
    struct RtPeerKeys *pPrevious; /*!< The previous peer of the connection. */
    struct RtPeerKeys *pNext;     /*!< The next peer of the connection. */
    unsigned int uPeer;           /*!< The peer, in its low 29 bits. */
    RtKey *pPublicKey;            /*!< The RSA public key of the peer, or null. */
    RtRc4 *pEncrypt;              /*!< The cipher of the packets to the peer, or null. */
    RtRc4 *pDecrypt;              /*!< The cipher of the packets from the peer, or null. */
    RtKey *pSessionKey;           /*!< The RC4 key of the peer, or null. */
} RtPeerKeys;

/** The keys of a connection, in a circular list. */
typedef struct RtConnectionKeys {
    struct RtConnectionKeys *pPrevious; /*!< The previous connection. */
    struct RtConnectionKeys *pNext;     /*!< The next connection. */
    RtPeerKeys *pFirstPeer;             /*!< The first peer. */
    RtPeerKeys *pLastPeer;              /*!< The last peer. */
    int nId;                            /*!< The connection. */
    int nPeer;                          /*!< The peer the connection is to. */
    int nKeyIndex;                      /*!< The index of the RC4 key. */
    RtKey *pModulus;                    /*!< The RSA modulus of this console, or null. */
    RtKey *pPrivateExponent;            /*!< The RSA private exponent of this console, or null. */
    RtRc4 *pEncrypt;                    /*!< The cipher of the packets sent, or null. */
    RtRc4 *pDecrypt;                    /*!< The cipher of the packets received, or null. */
    RtKey *pSessionKey;                 /*!< The RC4 key of the connection, or null. */
} RtConnectionKeys;

/**
 * Store the key pair of the listening connections or of the others, or clear it.
 *
 * @param nSelector RT_SECURITY_LISTEN_ID for the listening connections, anything else for the rest.
 * @param pKeyPair The key pair, or null to clear it.
 * @ghidraAddress NTSC-U/C: 0x002a9200
 * @ghidraAddress PAL: 0x002b3170
 */
void RtStoreKeyPair(int nSelector, const RtKeyPair *pKeyPair);

/**
 * Create the key record of a connection, unless it exists. The record takes the stored key pair,
 * and the public key of the peer when one is given.
 *
 * @param nId The connection.
 * @param nPeer The peer the connection is to.
 * @param pPeerKey The RSA public key of the peer.
 * @return 0, or RT_SECURITY_RESULT_FAILED when the allocation failed.
 * @ghidraAddress NTSC-U/C: 0x002a93bc
 * @ghidraAddress PAL: 0x002b332c
 */
int RtCreateConnectionKeys(int nId, int nPeer, const RtKey *pPeerKey);

/**
 * Set the peer the key record of a connection is to.
 *
 * @param nId The connection.
 * @param nPeer The peer.
 * @ghidraAddress NTSC-U/C: 0x002a94f4
 * @ghidraAddress PAL: 0x002b3464
 */
void RtSetConnectionPeer(int nId, int nPeer);

/**
 * Store the RSA public key of a peer, or clear it when the key is empty.
 *
 * @param nId The connection.
 * @param nPeer The peer.
 * @param pPublicKey The key.
 * @return 0, or RT_SECURITY_RESULT_FAILED when the connection has no record.
 * @ghidraAddress NTSC-U/C: 0x002a9520
 * @ghidraAddress PAL: 0x002b3490
 */
int RtStorePeerPublicKey(int nId, int nPeer, const RtKey *pPublicKey);

/**
 * Copy the modulus of a stored key pair.
 *
 * @param nSelector RT_SECURITY_LISTEN_ID for the listening connections, anything else for the rest.
 * @param pModulus Receives the modulus, zero when none is stored, or null.
 * @return 0, or RT_SECURITY_RESULT_NO_KEYS.
 * @ghidraAddress NTSC-U/C: 0x002a9834
 * @ghidraAddress PAL: 0x002b37a4
 */
int RtReadKeyPairModulus(int nSelector, RtKey *pModulus);

/**
 * Store the RC4 key of a connection. A key for a connection other than the server connection is
 * also stored for the server connection.
 *
 * @param nId The connection.
 * @param nKeyIndex The index of the key.
 * @param pKey The key.
 * @return 0, or RT_SECURITY_RESULT_FAILED.
 * @ghidraAddress NTSC-U/C: 0x002a99cc
 * @ghidraAddress PAL: 0x002b393c
 */
int RtSetConnectionKey(int nId, int nKeyIndex, const RtKey *pKey);

/**
 * Store the RC4 key of a peer.
 *
 * @param nId The connection.
 * @param nPeer The peer.
 * @param pKey The key.
 * @return 0, or RT_SECURITY_RESULT_FAILED.
 * @ghidraAddress NTSC-U/C: 0x002a9b30
 * @ghidraAddress PAL: 0x002b3aa0
 */
int RtSetPeerKey(int nId, int nPeer, const RtKey *pKey);

/**
 * Sign the RC4 key of a peer with an RSA key.
 *
 * @param nId The connection.
 * @param nPeer The peer.
 * @param pModulus The modulus.
 * @param pExponent The exponent.
 * @param pSignature Receives the signature.
 * @return 0, or RT_SECURITY_RESULT_FAILED.
 * @ghidraAddress NTSC-U/C: 0x002a9ca8
 * @ghidraAddress PAL: 0x002b3c18
 */
int RtSignPeerKey(
    int nId, int nPeer, const RtKey *pModulus, const RtKey *pExponent, RtKey *pSignature);

/**
 * Destroy the key record of a connection and of its peers.
 *
 * @param nId The connection.
 * @ghidraAddress NTSC-U/C: 0x002a9d88
 * @ghidraAddress PAL: 0x002b3cf8
 */
void RtDestroyConnectionKeys(int nId);

/**
 * Destroy the key records of a class of connections.
 *
 * @param nClass RT_SECURITY_SERVER_ID for the server connection, RT_SECURITY_LISTEN_ID for the
 * listening connections, or 0 for the rest.
 * @ghidraAddress NTSC-U/C: 0x002a9eb8
 * @ghidraAddress PAL: 0x002b3e28
 */
void RtDestroyConnectionKeysOfClass(int nClass);

/**
 * Encrypt a packet in place, with the best key the records have for its receiver.
 *
 * @param nId The connection.
 * @param nPeer The receiver, or RT_SECURITY_ALL_PEERS.
 * @param pData The packet.
 * @param uSize The size of the packet in bytes.
 * @return The header of the packet, or 0 when the connection has no record.
 * @ghidraAddress NTSC-U/C: 0x002a9fb0
 * @ghidraAddress PAL: 0x002b3f20
 */
unsigned int RtEncryptPacket(int nId, int nPeer, void *pData, unsigned int uSize);

/**
 * Decrypt a packet in place and check its digest.
 *
 * @param nId The connection.
 * @param nPeer The sender.
 * @param uHeader The header of the packet.
 * @param pData The packet.
 * @param uSize The size of the packet in bytes.
 * @return 0, RT_SECURITY_RESULT_FAILED, or RT_SECURITY_RESULT_MISMATCH.
 * @ghidraAddress NTSC-U/C: 0x002aa1b4
 * @ghidraAddress PAL: 0x002b4124
 */
int RtDecryptPacket(int nId, int nPeer, unsigned int uHeader, void *pData, unsigned int uSize);

#ifdef __cplusplus
}
#endif

#endif

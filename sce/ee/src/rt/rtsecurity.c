#include "rt/rtsecurity.h"

#include <string.h>

#include "rt/rtmemory.h"
#include "rt/rtrandom.h"
#include "rt/rtrsa.h"
#include "rt/rtsha1.h"

enum {
    kKeyWordCount = 16,
    kKeyByteCount = 64,
    // The peer that a missing peer of a packet stands for.
    kNoPeer = 0xfffe,
    // The seed of the generator of the keys.
    kKeySeed = 0x35d2325,
    // The bits of a peer that its record compares.
    kPeerMask = 0x1fffffff,
};

// NTSC-U/C: 0x003b619c, PAL: 0x0043505c
static RtConnectionKeys *g_pConnectionKeys;
// NTSC-U/C: 0x003b61a0, PAL: 0x00435060
static unsigned char g_bKeysSeeded;
// The key pair of the listening connections.
// NTSC-U/C: 0x00519de0
static RtKeyPair g_listenKeyPair;
// The key pair of the other connections.
// NTSC-U/C: 0x00519e60
static RtKeyPair g_keyPair;

// NTSC-U/C: 0x002a8ef8, PAL: 0x002b2e68
static int RtIsKeyZero(const RtKey *pKey) {
    unsigned int i;

    if (pKey == NULL) {
        return 1;
    }
    for (i = 0; i < kKeyWordCount; ++i) {
        if (pKey->words[i] != 0) {
            return 0;
        }
    }
    return 1;
}

// NTSC-U/C: 0x002a8f38, PAL: 0x002b2ea8
static int RtAreKeyBytesZero(const RtKey *pKey) {
    unsigned int i;

    if (pKey == NULL) {
        return 1;
    }
    for (i = 0; i < kKeyByteCount; ++i) {
        if (pKey->bytes[i] != 0) {
            return 0;
        }
    }
    return 1;
}

// NTSC-U/C: 0x002a8f78, PAL: 0x002b2ee8
static RtConnectionKeys *RtFindConnectionKeys(int nId) {
    RtConnectionKeys *pKeys;

    pKeys = g_pConnectionKeys;
    if (pKeys == NULL) {
        return NULL;
    }
    while (pKeys->nId != nId) {
        pKeys = pKeys->pNext;
        if (pKeys == g_pConnectionKeys) {
            return NULL;
        }
    }
    return pKeys;
}

// NTSC-U/C: 0x002a8fc0, PAL: 0x002b2f30
static RtPeerKeys *RtAllocPeerKeys(void) {
    RtPeerKeys *pPeer;

    pPeer = NULL;
    RtAllocBlock((void **)&pPeer, sizeof(RtPeerKeys));
    if (pPeer != NULL) {
        pPeer->pPrevious = NULL;
        pPeer->pNext = NULL;
        pPeer->pPublicKey = NULL;
        pPeer->pEncrypt = NULL;
        pPeer->pDecrypt = NULL;
        pPeer->pSessionKey = NULL;
        pPeer->uPeer = 0;
    }
    return pPeer;
}

// NTSC-U/C: 0x002a9024, PAL: 0x002b2f94
static void RtFreePeerKeys(RtPeerKeys *pPeer) {
    if (pPeer == NULL) {
        return;
    }
    if (pPeer->pPublicKey != NULL) {
        RtFreeBlock((void **)&pPeer->pPublicKey);
    }
    if (pPeer->pSessionKey != NULL) {
        RtFreeBlock((void **)&pPeer->pSessionKey);
    }
    if (pPeer->pEncrypt != NULL) {
        RtFreeBlock((void **)&pPeer->pEncrypt);
    }
    if (pPeer->pDecrypt != NULL) {
        RtFreeBlock((void **)&pPeer->pDecrypt);
    }
    RtFreeBlock((void **)&pPeer);
}

// NTSC-U/C: 0x002a90a4, PAL: 0x002b3014
static void RtAppendPeerKeys(RtConnectionKeys *pKeys, RtPeerKeys *pPeer) {
    if (pKeys == NULL) {
        return;
    }
    pPeer->pNext = NULL;
    if (pKeys->pFirstPeer == NULL) {
        pKeys->pFirstPeer = pPeer;
        pPeer->pPrevious = NULL;
    } else {
        pPeer->pPrevious = pKeys->pLastPeer;
        pKeys->pLastPeer->pNext = pPeer;
    }
    pKeys->pLastPeer = pPeer;
}

// NTSC-U/C: 0x002a90dc, PAL: 0x002b304c
static RtPeerKeys *RtFindPeerKeys(RtConnectionKeys *pKeys, unsigned int uPeer) {
    RtPeerKeys *pPeer;

    pPeer = NULL;
    uPeer &= kPeerMask;
    if (pKeys != NULL) {
        pPeer = pKeys->pFirstPeer;
    }
    while (pPeer != NULL && pPeer->uPeer != uPeer) {
        pPeer = pPeer->pNext;
    }
    return pPeer;
}

// NTSC-U/C: 0x002a9110, PAL: 0x002b3080
static void RtRemovePeerKeys(RtConnectionKeys *pKeys, RtPeerKeys *pPeer) {
    if (pKeys == NULL || pPeer == NULL) {
        return;
    }
    if (pPeer->pNext != NULL) {
        pPeer->pNext->pPrevious = pPeer->pPrevious;
    } else {
        pKeys->pLastPeer = pPeer->pPrevious;
    }
    if (pPeer->pPrevious != NULL) {
        pPeer->pPrevious->pNext = pPeer->pNext;
    } else {
        pKeys->pFirstPeer = pPeer->pNext;
    }
}

// NTSC-U/C: 0x002a9158, PAL: 0x002b30c8
static void RtSeedKeysOnce(void) {
    if (!g_bKeysSeeded) {
        g_bKeysSeeded = 1;
        RtSeedRandomFromEntropy(kKeySeed);
    }
}

// NTSC-U/C: 0x002a9194, PAL: 0x002b3104
static void RtSha1(const void *pData, unsigned int uSize, void *pDigest, unsigned int uDigestSize) {
    RtSha1Context context;

    RtSha1Init(&context);
    RtSha1Update(&context, pData, uSize);
    RtSha1Final(&context, pDigest, uDigestSize);
}

// NTSC-U/C: 0x002a9674, PAL: 0x002b35e4
static int RtStoreConnectionKeyPair(int nId, const RtKeyPair *pKeyPair) {
    RtConnectionKeys *pKeys;

    pKeys = RtFindConnectionKeys(nId);
    if (pKeys == NULL) {
        return RT_SECURITY_RESULT_FAILED;
    }
    if (RtIsKeyZero(&pKeyPair->modulus) || RtIsKeyZero(&pKeyPair->privateExponent)) {
        if (pKeys->pModulus != NULL) {
            memset(pKeys->pModulus, 0, sizeof(RtKey));
        }
        if (pKeys->pPrivateExponent != NULL) {
            memset(pKeys->pPrivateExponent, 0, sizeof(RtKey));
        }
        return 0;
    }
    if (pKeys->pModulus == NULL) {
        RtAllocBlock((void **)&pKeys->pModulus, sizeof(RtKey));
    }
    if (pKeys->pPrivateExponent == NULL) {
        RtAllocBlock((void **)&pKeys->pPrivateExponent, sizeof(RtKey));
    }
    *pKeys->pModulus = pKeyPair->modulus;
    *pKeys->pPrivateExponent = pKeyPair->privateExponent;
    return 0;
}

void RtStoreKeyPair(int nSelector, const RtKeyPair *pKeyPair) {
    if (nSelector == (int)RT_SECURITY_LISTEN_ID) {
        if (pKeyPair != NULL) {
            g_listenKeyPair = *pKeyPair;
        } else {
            memset(&g_listenKeyPair, 0, sizeof(RtKeyPair));
        }
    } else if (pKeyPair != NULL) {
        g_keyPair = *pKeyPair;
    } else {
        memset(&g_keyPair, 0, sizeof(RtKeyPair));
    }
}

int RtCreateConnectionKeys(int nId, int nPeer, const RtKey *pPeerKey) {
    RtConnectionKeys *pKeys;

    pKeys = RtFindConnectionKeys(nId);
    if (pKeys != NULL) {
        return 0;
    }
    RtAllocBlock((void **)&pKeys, sizeof(RtConnectionKeys));
    // Yes, the binary clears the record before it checks the allocation.
    memset(pKeys, 0, sizeof(RtConnectionKeys));
    if (pKeys == NULL) {
        return RT_SECURITY_RESULT_FAILED;
    }
    pKeys->nId = nId;
    pKeys->nPeer = nPeer;
    pKeys->nKeyIndex = 0;
    if (g_pConnectionKeys != NULL) {
        pKeys->pPrevious = g_pConnectionKeys->pPrevious;
        pKeys->pPrevious->pNext = pKeys;
    } else {
        g_pConnectionKeys = pKeys;
    }
    g_pConnectionKeys->pPrevious = pKeys;
    pKeys->pNext = g_pConnectionKeys;
    if (nId < 0 && nId != RT_SECURITY_SERVER_ID) {
        RtStoreConnectionKeyPair(nId, &g_listenKeyPair);
    } else {
        RtStoreConnectionKeyPair(nId, &g_keyPair);
    }
    if (nPeer != RT_SECURITY_ALL_PEERS && nId != RT_SECURITY_SERVER_ID) {
        RtStorePeerPublicKey(nId, RT_SECURITY_ALL_PEERS, pPeerKey);
    }
    return 0;
}

void RtSetConnectionPeer(int nId, int nPeer) {
    RtConnectionKeys *pKeys;

    pKeys = RtFindConnectionKeys(nId);
    if (pKeys != NULL) {
        pKeys->nPeer = nPeer;
    }
}

int RtStorePeerPublicKey(int nId, int nPeer, const RtKey *pPublicKey) {
    RtConnectionKeys *pKeys;
    RtPeerKeys *pPeer;

    pKeys = RtFindConnectionKeys(nId);
    if (pKeys == NULL) {
        return RT_SECURITY_RESULT_FAILED;
    }
    pPeer = RtFindPeerKeys(pKeys, nPeer);
    if (RtIsKeyZero(pPublicKey)) {
        if (pPeer != NULL && pPeer->pPublicKey != NULL) {
            memset(pPeer->pPublicKey, 0, sizeof(RtKey));
        }
        return 0;
    }
    if (pPeer == NULL) {
        pPeer = RtAllocPeerKeys();
        pPeer->uPeer = nPeer;
        RtAppendPeerKeys(pKeys, pPeer);
    }
    if (pPeer->pPublicKey == NULL) {
        RtAllocBlock((void **)&pPeer->pPublicKey, sizeof(RtKey));
    }
    *pPeer->pPublicKey = *pPublicKey;
    return 0;
}

int RtReadKeyPairModulus(int nSelector, RtKey *pModulus) {
    const RtKeyPair *pKeyPair;
    int nResult;

    nResult = 0;
    if (nSelector == (int)RT_SECURITY_LISTEN_ID) {
        pKeyPair = &g_listenKeyPair;
    } else {
        pKeyPair = &g_keyPair;
    }
    if (RtIsKeyZero(&pKeyPair->modulus)) {
        nResult = RT_SECURITY_RESULT_NO_KEYS;
    }
    if (pModulus == NULL) {
        return nResult;
    }
    *pModulus = pKeyPair->modulus;
    if (nResult != 0) {
        memset(pModulus, 0, sizeof(RtKey));
    }
    return nResult;
}

int RtSetConnectionKey(int nId, int nKeyIndex, const RtKey *pKey) {
    RtConnectionKeys *pKeys;

    pKeys = RtFindConnectionKeys(nId);
    if (pKeys == NULL) {
        return RT_SECURITY_RESULT_FAILED;
    }
    if (RtAreKeyBytesZero(pKey)) {
        return RT_SECURITY_RESULT_FAILED;
    }
    if (pKeys->pSessionKey == NULL) {
        RtAllocBlock((void **)&pKeys->pSessionKey, sizeof(RtKey));
    }
    if (pKeys->pEncrypt == NULL) {
        RtAllocBlock((void **)&pKeys->pEncrypt, sizeof(RtRc4));
    }
    if (pKeys->pDecrypt == NULL) {
        RtAllocBlock((void **)&pKeys->pDecrypt, sizeof(RtRc4));
    }
    pKeys->nKeyIndex = nKeyIndex;
    *pKeys->pSessionKey = *pKey;
    RtRc4SetKey(pKeys->pEncrypt, pKey);
    RtRc4SetKey(pKeys->pDecrypt, pKey);
    if (nId != RT_SECURITY_SERVER_ID) {
        RtSetConnectionKey(RT_SECURITY_SERVER_ID, nKeyIndex, pKey);
    }
    return 0;
}

int RtSetPeerKey(int nId, int nPeer, const RtKey *pKey) {
    RtConnectionKeys *pKeys;
    RtPeerKeys *pPeer;

    pKeys = RtFindConnectionKeys(nId);
    if (pKeys == NULL) {
        return RT_SECURITY_RESULT_FAILED;
    }
    if (RtAreKeyBytesZero(pKey)) {
        return RT_SECURITY_RESULT_FAILED;
    }
    pPeer = RtFindPeerKeys(pKeys, nPeer);
    if (pPeer == NULL) {
        pPeer = RtAllocPeerKeys();
        pPeer->uPeer = nPeer;
        RtAppendPeerKeys(pKeys, pPeer);
    }
    if (pPeer->pSessionKey == NULL) {
        RtAllocBlock((void **)&pPeer->pSessionKey, sizeof(RtKey));
    }
    if (pPeer->pEncrypt == NULL) {
        RtAllocBlock((void **)&pPeer->pEncrypt, sizeof(RtRc4));
    }
    if (pPeer->pDecrypt == NULL) {
        RtAllocBlock((void **)&pPeer->pDecrypt, sizeof(RtRc4));
    }
    *pPeer->pSessionKey = *pKey;
    RtRc4SetKey(pPeer->pEncrypt, pKey);
    RtRc4SetKey(pPeer->pDecrypt, pKey);
    return 0;
}

int RtSignPeerKey(
    int nId, int nPeer, const RtKey *pModulus, const RtKey *pExponent, RtKey *pSignature) {
    RtConnectionKeys *pKeys;
    RtPeerKeys *pPeer;

    if (pSignature == NULL) {
        return RT_SECURITY_RESULT_FAILED;
    }
    if (RtIsKeyZero(pModulus)) {
        return RT_SECURITY_RESULT_FAILED;
    }
    if (RtIsKeyZero(pExponent)) {
        return RT_SECURITY_RESULT_FAILED;
    }
    pKeys = RtFindConnectionKeys(nId);
    if (pKeys == NULL) {
        return RT_SECURITY_RESULT_FAILED;
    }
    pPeer = RtFindPeerKeys(pKeys, nPeer);
    if (pPeer == NULL) {
        return RT_SECURITY_RESULT_FAILED;
    }
    if (RtAreKeyBytesZero(pPeer->pSessionKey)) {
        return RT_SECURITY_RESULT_FAILED;
    }
    // Yes, the binary signs only the first 4 bytes of the key.
    RtSha1(pPeer->pSessionKey, sizeof(unsigned int), pSignature, sizeof(RtKey));
    RtRsaTransform(pModulus, pExponent, pSignature, pSignature, sizeof(RtKey));
    return 0;
}

void RtDestroyConnectionKeys(int nId) {
    RtConnectionKeys *pKeys;
    RtPeerKeys *pPeer;

    pKeys = RtFindConnectionKeys(nId);
    if (pKeys == NULL) {
        return;
    }
    while ((pPeer = pKeys->pLastPeer) != NULL) {
        RtRemovePeerKeys(pKeys, pPeer);
        RtFreePeerKeys(pPeer);
    }
    if (pKeys->pPrivateExponent != NULL) {
        RtFreeBlock((void **)&pKeys->pPrivateExponent);
    }
    if (pKeys->pModulus != NULL) {
        RtFreeBlock((void **)&pKeys->pModulus);
    }
    if (pKeys->pSessionKey != NULL) {
        RtFreeBlock((void **)&pKeys->pSessionKey);
    }
    if (pKeys->pEncrypt != NULL) {
        RtFreeBlock((void **)&pKeys->pEncrypt);
    }
    if (pKeys->pDecrypt != NULL) {
        RtFreeBlock((void **)&pKeys->pDecrypt);
    }
    if (g_pConnectionKeys == pKeys) {
        g_pConnectionKeys = pKeys->pNext;
    }
    pKeys->pNext->pPrevious = pKeys->pPrevious;
    pKeys->pPrevious->pNext = pKeys->pNext;
    if (g_pConnectionKeys == pKeys) {
        g_pConnectionKeys = NULL;
    }
    RtFreeBlock((void **)&pKeys);
}

void RtDestroyConnectionKeysOfClass(int nClass) {
    RtConnectionKeys *pKeys;
    int nId;

    pKeys = g_pConnectionKeys;
    while (pKeys != NULL) {
        nId = pKeys->nId;
        if ((nClass == RT_SECURITY_SERVER_ID && nId == RT_SECURITY_SERVER_ID) ||
            (nClass == (int)RT_SECURITY_LISTEN_ID &&
             nId == (int)((nId & ~RT_SECURITY_LISTEN_ID) + RT_SECURITY_LISTEN_ID)) ||
            (nClass == 0 && nId == (int)(nId & ~RT_SECURITY_LISTEN_ID))) {
            RtDestroyConnectionKeys(nId);
            pKeys = g_pConnectionKeys;
        } else {
            pKeys = pKeys->pNext;
            if (pKeys == g_pConnectionKeys) {
                pKeys = NULL;
            }
        }
    }
}

void rtNetGenerateKeyPair(rtNetKey *publicKey, rtNetKey *privateKey) {
    RtSeedKeysOnce();
    RtRsaGenerateKeyPair(publicKey, privateKey);
}

unsigned int RtEncryptPacket(int nId, int nPeer, void *pData, unsigned int uSize) {
    RtConnectionKeys *pKeys;
    RtPeerKeys *pPeer;
    unsigned int uHeader;
    unsigned int uDigest;

    pKeys = RtFindConnectionKeys(nId);
    uHeader = 0;
    uDigest = 0;
    pPeer = NULL;
    if (uSize != 0 && pData != NULL) {
        RtSha1(pData, uSize, &uDigest, sizeof(uDigest));
        uDigest &= RT_SECURITY_DIGEST_MASK;
    } else {
        uSize = 0;
        uDigest = 0;
    }
    if (pKeys == NULL) {
        return 0;
    }
    if (nPeer == kNoPeer) {
        nPeer = RT_SECURITY_ALL_PEERS;
    }
    if (nPeer != RT_SECURITY_ALL_PEERS || pKeys->nPeer != nPeer) {
        pPeer = RtFindPeerKeys(pKeys, nPeer);
    }
    if (pPeer != NULL && pPeer->pEncrypt != NULL) {
        uHeader = (uDigest & RT_SECURITY_DIGEST_MASK) | RT_SECURITY_METHOD_PEER;
        if (uSize == 0) {
            return uHeader;
        }
        RtRc4SetSaltedKey(pPeer->pEncrypt, pPeer->pSessionKey, uHeader);
        RtRc4Encrypt(pPeer->pEncrypt, pData, uSize);
        return uHeader;
    }
    if (pPeer == NULL && nPeer != RT_SECURITY_ALL_PEERS) {
        return uHeader;
    }
    if (pKeys->pEncrypt != NULL &&
        (nPeer == RT_SECURITY_ALL_PEERS || pKeys->nPeer != RT_SECURITY_ALL_PEERS || pPeer == NULL ||
         RtIsKeyZero(pPeer->pPublicKey))) {
        uHeader = (uDigest & RT_SECURITY_DIGEST_MASK) | RT_SECURITY_METHOD_CONNECTION;
        if (uSize == 0) {
            return uHeader;
        }
        RtRc4SetSaltedKey(pKeys->pEncrypt, pKeys->pSessionKey, uHeader);
        RtRc4Encrypt(pKeys->pEncrypt, pData, uSize);
        return uHeader;
    }
    if (pPeer == NULL || RtIsKeyZero(pPeer->pPublicKey)) {
        return uHeader;
    }
    if (pKeys->nPeer != RT_SECURITY_ALL_PEERS && pPeer->pEncrypt != NULL) {
        return uHeader;
    }
    uHeader = (uDigest & RT_SECURITY_DIGEST_MASK) | RT_SECURITY_METHOD_PUBLIC;
    if (uSize == 0) {
        return uHeader;
    }
    RtRsaEncryptPublic(pPeer->pPublicKey, pData, pData, uSize);
    return uHeader;
}

int RtDecryptPacket(int nId, int nPeer, unsigned int uHeader, void *pData, unsigned int uSize) {
    RtLargeInt sum;
    RtLargeInt modulus;
    RtConnectionKeys *pKeys;
    RtPeerKeys *pPeer;
    unsigned int uDigest;
    unsigned int uMethod;
    int nResult;

    pKeys = RtFindConnectionKeys(nId);
    nResult = 0;
    uDigest = 0;
    if (pKeys == NULL) {
        return RT_SECURITY_RESULT_FAILED;
    }
    switch (uHeader & RT_SECURITY_METHOD_MASK) {
    case RT_SECURITY_METHOD_NONE:
        break;
    case RT_SECURITY_METHOD_CONNECTION:
        if (pKeys->pDecrypt == NULL) {
            nResult = RT_SECURITY_RESULT_FAILED;
            break;
        }
        RtRc4SetSaltedKey(pKeys->pDecrypt, pKeys->pSessionKey, uHeader);
        RtRc4Decrypt(pKeys->pDecrypt, pData, uSize);
        break;
    case RT_SECURITY_METHOD_PEER:
        pPeer = RtFindPeerKeys(pKeys, nPeer);
        if (pPeer == NULL || pPeer->pDecrypt == NULL) {
            nResult = RT_SECURITY_RESULT_FAILED;
            break;
        }
        RtRc4SetSaltedKey(pPeer->pDecrypt, pPeer->pSessionKey, uHeader);
        RtRc4Decrypt(pPeer->pDecrypt, pData, uSize);
        break;
    case RT_SECURITY_METHOD_PUBLIC:
        RtRsaTransform(pKeys->pModulus, pKeys->pPrivateExponent, pData, pData, uSize);
        break;
    default:
        nResult = RT_SECURITY_RESULT_FAILED;
        break;
    }
    if (uSize == 0) {
        return nResult;
    }
    if (nResult != 0) {
        return nResult;
    }
    RtSha1(pData, uSize, &uDigest, sizeof(uDigest));
    uMethod = uHeader & RT_SECURITY_METHOD_MASK;
    uDigest = (uDigest & RT_SECURITY_DIGEST_MASK) | uMethod;
    if (uHeader != uDigest && uMethod == RT_SECURITY_METHOD_PUBLIC) {
        // The plain packet may have been at least the modulus. Add the modulus back and check
        // again.
        memset(&sum.abBytes[uSize], 0, sizeof(RtLargeInt) - uSize);
        memcpy(sum.abBytes, pData, uSize);
        memset(&modulus.abBytes[uSize], 0, sizeof(RtLargeInt) - uSize);
        memcpy(modulus.abBytes, pKeys->pModulus, uSize);
        RtLargeIntAdd(&sum, &modulus);
        memcpy(pData, sum.abBytes, uSize);
        RtSha1(pData, uSize, &uDigest, sizeof(uDigest));
        uDigest = (uDigest & RT_SECURITY_DIGEST_MASK) | uMethod;
    }
    if (uHeader != uDigest) {
        nResult = RT_SECURITY_RESULT_MISMATCH;
    }
    return nResult;
}

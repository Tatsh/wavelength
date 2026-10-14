#include "netflow/netcrypt.h"

#include <cstring>

extern "C" {
#include <mycrypt.h>
}

#include "math/rand.h"

namespace {

// The bytes of a Blowfish block, of its key, and of the initial vector.
constexpr int kBlockSize = 8;

// The default rounds of Blowfish.
constexpr int kDefaultRounds = 0;

} // namespace

// NTSC-U/C: 0x0043e610
static int g_nNetCryptCipher;
// NTSC-U/C: 0x0043e618
static unsigned char g_abNetCryptKey[kBlockSize];
// NTSC-U/C: 0x0043e620
static unsigned char g_abNetCryptIv[kBlockSize];
// NTSC-U/C: 0x0043e628
static symmetric_CBC g_netCryptCbc;
// NTSC-U/C: 0x003b0cb0
static symmetric_CBC *g_pNetCryptCbc;

void NetCryptInit() {
    g_pNetCryptCbc = &g_netCryptCbc;
    register_cipher(&blowfish_desc);
    g_nNetCryptCipher = find_cipher("blowfish");
    memcpy(g_abNetCryptKey, "12345678", kBlockSize);
    memcpy(g_abNetCryptIv, "abcdefgh", kBlockSize);
    cbc_start(g_nNetCryptCipher,
              g_abNetCryptIv,
              g_abNetCryptKey,
              kBlockSize,
              kDefaultRounds,
              g_pNetCryptCbc);
}

void NetCryptMakeKey(unsigned char pKey[kNetCryptKeySize]) {
    for (int i = 0; i < kNetCryptKeyWords; ++i) {
        const unsigned int nWord = static_cast<unsigned int>(RandomWord());
        memcpy(&pKey[i * sizeof(nWord)], &nWord, sizeof(nWord));
    }
}

void NetCryptSetKey(const unsigned char pKey[kNetCryptKeySize]) {
    memcpy(g_abNetCryptKey, pKey, kBlockSize);
    memcpy(g_abNetCryptIv, &pKey[kBlockSize], kBlockSize);
}

void NetCryptEncrypt(void *pData, int nSize) {
    unsigned char *pBytes = static_cast<unsigned char *>(pData);
    cbc_setiv(g_nNetCryptCipher, g_abNetCryptIv, g_pNetCryptCbc);
    const int nBlocks = nSize / kBlockSize;
    for (int i = 0; i < nBlocks; ++i) {
        cbc_encrypt(&pBytes[i * kBlockSize], &pBytes[i * kBlockSize], g_pNetCryptCbc);
    }
}

void NetCryptDecrypt(void *pData, int nSize) {
    unsigned char *pBytes = static_cast<unsigned char *>(pData);
    cbc_setiv(g_nNetCryptCipher, g_abNetCryptIv, g_pNetCryptCbc);
    const int nBlocks = nSize / kBlockSize;
    for (int i = 0; i < nBlocks; ++i) {
        cbc_decrypt(&pBytes[i * kBlockSize], &pBytes[i * kBlockSize], g_pNetCryptCbc);
    }
}

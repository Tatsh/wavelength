#include "encvag.h"

#include <string.h>

// This is a stand-in for the encoder library. It writes silent blocks, so a bank saved with it has
// the right layout but no audio.

#define VAG_BLOCK_SIZE 16

int __stdcall EncVagInit(short mode) {
    (void)mode;
    return 0;
}

int __stdcall EncVag(short *wav, short *vag, short attribute) {
    (void)wav;
    (void)attribute;
    memset(vag, 0, VAG_BLOCK_SIZE);
    return 0;
}

int __stdcall EncVagFin(short *vag) {
    memset(vag, 0, VAG_BLOCK_SIZE);
    return 0;
}

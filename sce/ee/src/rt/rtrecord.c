#include "rt/rtrecord.h"

#include <stddef.h>

enum {
    kRecordResultInvalid = 2,
};

int RtRecordWriteByte(unsigned char *pbField, unsigned char uValue) {
    if (pbField == NULL) {
        return kRecordResultInvalid;
    }
    pbField[0] = uValue;
    return 0;
}

int RtRecordReadByte(const unsigned char *pbField, unsigned char *puValue) {
    if (pbField == NULL || puValue == NULL) {
        return kRecordResultInvalid;
    }
    *puValue = pbField[0];
    return 0;
}

int RtRecordWriteWord(unsigned char *pbField, unsigned short uValue) {
    if (pbField == NULL) {
        return kRecordResultInvalid;
    }
    pbField[1] = uValue >> 8;
    pbField[0] = uValue;
    return 0;
}

int RtRecordReadWord(const unsigned char *pbField, unsigned short *puValue) {
    if (pbField == NULL || puValue == NULL) {
        return kRecordResultInvalid;
    }
    *puValue = pbField[0];
    *puValue = pbField[0] | (pbField[1] << 8);
    return 0;
}

int RtRecordWriteDword(unsigned char *pbField, unsigned int uValue) {
    if (pbField == NULL) {
        return kRecordResultInvalid;
    }
    pbField[0] = uValue;
    pbField[1] = uValue >> 8;
    pbField[3] = uValue >> 24;
    pbField[2] = uValue >> 16;
    return 0;
}

int RtRecordReadDword(const unsigned char *pbField, unsigned int *puValue) {
    unsigned int uValue;

    if (pbField == NULL || puValue == NULL) {
        return kRecordResultInvalid;
    }
    uValue = pbField[0];
    *puValue = uValue;
    uValue |= pbField[1] << 8;
    *puValue = uValue;
    uValue |= pbField[2] << 16;
    *puValue = uValue;
    uValue |= (unsigned int)pbField[3] << 24;
    *puValue = uValue;
    return 0;
}

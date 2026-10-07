#pragma once

#include "eznctl_s/netcnfifdata.h"

/** Argument buffer of an eznetctl RPC request. The first word receives the result. */
struct NetCtlRequest {
    union {
        int mInterfaceId;    /*!< Interface of an inetctl function. */
        NetcnfifData *mData; /*!< Receives the address of the settings the EE writes. */
        int mResult;         /*!< Receives the result. */
    };
    unsigned int mReserved[3]; /*!< Never touched by the module. */
    char mFileName[256];       /*!< Configuration file to load. */
    char mUserName[48];        /*!< Name the user gave the combination to load. */
};

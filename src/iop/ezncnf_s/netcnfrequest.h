#pragma once

#include <netcnf.h>

#include "ezncnf_s/netcnflistreply.h"

/**
 * Argument buffer of an eznetcnf RPC request. The first word receives the result of every
 * function.
 *
 * The module was built without RTTI. The name is inferred from the module's routines.
 */
class NetCnfRequest {
public:
    /** Bits of mFlags. */
    enum Flag {
        kFlagQueryInterface = 0x01, /*!< GetList() records netcnf_22() as each entry's status. */
    };

    /**
     * List the combinations of mFileName, load each one, and send a NetCnfListReply to
     * mDestination.
     *
     * @return The NetCnfListReply::mEntryLimit sent, or a negative error code.
     * @ghidraAddress NTSC-U/C: 0x000003ec
     * @ghidraAddress PAL: 0x000003ec
     */
    int GetList();

    /**
     * Load the combination mUserName of mFileName and send it to mDestination as a NetcnfifData.
     *
     * @return The result of the load, or -2 when no memory remains.
     * @ghidraAddress NTSC-U/C: 0x00000888
     * @ghidraAddress PAL: 0x00000888
     */
    int LoadEntry();

    union {
        int mType;          /*!< A #sceNetCnfType, for EzNetCnf::kFunctionGetCount. */
        void *mDestination; /*!< EE address that receives the reply of the other functions. */
        int mResult;        /*!< Receives the result. */
    };
    unsigned int mFlags;       /*!< Flag bits. */
    unsigned int mReserved[2]; /*!< Never touched by the module. */
    char mFileName[256];       /*!< Configuration file. */
    char mUserName[48];        /*!< Name the user gave the combination. */

private:
    /**
     * Fill a NetCnfListReply from the listings of mFileName and send it.
     *
     * @param list The combinations, then the interfaces, then the devices.
     * @param reply Reply to fill.
     * @param netCount Number of combinations.
     * @param interfaceCount Number of interfaces.
     * @param deviceCount Number of devices.
     * @return The NetCnfListReply::mEntryLimit sent, or the error of a failed load.
     */
    inline int FillList(sceNetCnfList *list,
                        NetCnfListReply *reply,
                        int netCount,
                        int interfaceCount,
                        int deviceCount);

    /**
     * Find the user name of a file in a listing.
     *
     * @param systemName Name of the file.
     * @param list Listing.
     * @param count Number of entries in @p list.
     * @return The user name, or null.
     * @ghidraAddress NTSC-U/C: 0x000002e4
     * @ghidraAddress PAL: 0x000002e4
     */
    static char *FindUserName(const char *systemName, sceNetCnfList *list, int count);

    /**
     * Read the number in a name, starting at its first digit.
     *
     * @param name Name.
     * @return The number, or zero when the name has no digit.
     * @ghidraAddress NTSC-U/C: 0x00000374
     * @ghidraAddress PAL: 0x00000374
     */
    static int ParseNumber(const char *name);
};

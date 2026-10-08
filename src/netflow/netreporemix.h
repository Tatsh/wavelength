#pragma once

#include "game/remixinfo.h"
#include "os/string.h"

/**
 * A remix the online repository offers for download.
 *
 * The name comes from the type information the standard library containers of the type record.
 * The record is 0xc8 bytes.
 */
struct NetRepoRemix {
    String mFile;    /*!< The file of the remix on the server. +0x00 */
    String mNote;    /*!< The text the `note` label of the download list shows. +0x14 */
    RemixInfo mInfo; /*!< The description of the remix. +0x28 */
};

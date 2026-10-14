#pragma once

#include "game/remixinfo.h"
#include "os/binstream.h"
#include "os/prnstream.h"
#include "os/string.h"

/**
 * A remix the online repository offers for download.
 *
 * The name comes from the type information the standard library containers of the type record.
 * The record is 0xc8 bytes.
 */
struct NetRepoRemix {
    String mFile;    /*!< The file of the remix on the server. */
    String mNote;    /*!< The text the `note` label of the download list shows. */
    RemixInfo mInfo; /*!< The description of the remix. */
};

/**
 * Write a remix as its file, description, and note.
 *
 * @param stream The stream.
 * @param remix The remix.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x002680d8
 * @ghidraAddress PAL: 0x00271db0
 */
PrnStream &operator<<(PrnStream &stream, const NetRepoRemix &remix);

/**
 * Write a remix as its file, note, and description.
 *
 * @param stream The stream.
 * @param remix The remix.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x00268168
 * @ghidraAddress PAL: 0x00271e40
 */
BinStream &operator<<(BinStream &stream, const NetRepoRemix &remix);

/**
 * Read a remix operator<<() wrote.
 *
 * @param stream The stream.
 * @param remix Receives the remix.
 * @return The stream.
 * @ghidraAddress NTSC-U/C: 0x002681b8
 * @ghidraAddress PAL: 0x00271e90
 */
BinStream &operator>>(BinStream &stream, NetRepoRemix &remix);

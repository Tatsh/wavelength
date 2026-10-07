#pragma once

#include "os/datetime.h"

/** Bytes of MemcardDirEntry::mName, the terminator included. */
constexpr int kMemcardDirEntryNameSize = 32;

/**
 * One entry of a memory card directory listing, as MemcardCBHandler::OnGetDir() receives it.
 *
 * MemcardPoll() converts each entry the library lists into one of these. The object is 0x34 bytes.
 */
struct MemcardDirEntry {
    char mName[kMemcardDirEntryNameSize]; /*!< The entry name. */
    int mSize;                            /*!< The file size in bytes. */
    unsigned short mAttributes;           /*!< The library's attribute bits. */
    DateTime mCreated;                    /*!< When the entry was created. */
    DateTime mModified;                   /*!< When the entry was last changed. */
};

/**
 * Order two entries by the time they were last changed, the older first.
 *
 * @param a The first entry.
 * @param b The second entry.
 * @return Whether a was changed before b.
 * @ghidraAddress NTSC-U/C: 0x0028b558
 * @ghidraAddress PAL: 0x00294d50
 */
bool operator<(const MemcardDirEntry &a, const MemcardDirEntry &b);

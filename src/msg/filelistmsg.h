#pragma once

#include <list>

#include "msg/message.h"
#include "netflow/lobbymsgtypes.h"
#include "os/mem.h"
#include "os/string.h"

/**
 * Result of NetLobby::RequestFiles().
 *
 * The RTTI includes the class name and records Message as the base. The vtable is at `0x003d4d58`.
 * The members Clone(), Type(), and GetName() are inline.
 */
class FileListMsg : public Message {
public:
    /**
     * Destroy the object.
     *
     * @ghidraAddress NTSC-U/C: 0x0039b2f8
     * @ghidraAddress PAL: 0x00409f40
     */
    ~FileListMsg() override = default;

    /**
     * Construct a message that points at the names.
     *
     * Inline.
     *
     * @param pFiles The names of the files. The caller retains them.
     */
    explicit FileListMsg(std::list<String> *pFiles) : mFiles(pFiles) {
    }

    /**
     * Allocate a message, billed to the class name.
     *
     * @param nSize The object size the compiler supplies.
     * @return The block.
     */
    void *operator new(size_t nSize) {
        return PoolMemAlloc(static_cast<int>(nSize), "FileListMsg", 0);
    }

    /**
     * Release a message.
     *
     * @param pBlock The block.
     */
    void operator delete(void *pBlock) {
        PoolMemFree(pBlock);
    }

    /**
     * Produce a heap copy of this message, which points at the same names.
     *
     * @return The copy.
     * @ghidraAddress NTSC-U/C: 0x0039b3a0
     * @ghidraAddress PAL: 0x00409fe8
     */
    Message *Clone() override {
        return new FileListMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nFileListMsgType.
     * @ghidraAddress NTSC-U/C: 0x0039b3f0
     * @ghidraAddress PAL: 0x0040a038
     */
    int Type() override {
        return g_nFileListMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `FileListMsg`.
     * @ghidraAddress NTSC-U/C: 0x0039b400
     * @ghidraAddress PAL: 0x0040a048
     */
    const char *GetName() const override {
        return "FileListMsg";
    }

    std::list<String> *mFiles; /*!< The names of the files. */
};

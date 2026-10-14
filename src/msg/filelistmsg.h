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
     */
    Message *Clone() override {
        return new FileListMsg(*this);
    }

    /**
     * Report this message's registered identity.
     *
     * @return g_nFileListMsgType.
     */
    int Type() override {
        return g_nFileListMsgType;
    }

    /**
     * Report this message's class name.
     *
     * @return The literal `FileListMsg`.
     */
    const char *GetName() const override {
        return "FileListMsg";
    }

    std::list<String> *mFiles; /*!< The names of the files. */
};

#pragma once

#include <memory>

#include "memcard/mcclosetask.h"
#include "memcard/mcopentask.h"
#include "memcard/mcreadtask.h"
#include "memcard/memcardserialtask.h"

/**
 * Work that reads the RemixInfo at the start of a remix file into mBuffer: open, read, and close.
 *
 * The RTTI records the class as deriving from MemcardSerialTask. The object is 0x34 bytes.
 */
class MCGetRemixInfoTask : public MemcardSerialTask {
public:
    /**
     * Construct the steps and the buffer.
     *
     * @ghidraAddress NTSC-U/C: 0x00161348
     * @ghidraAddress PAL: 0x00163e88
     */
    MCGetRemixInfoTask();

    /**
     * Release the buffer and the steps.
     *
     * @ghidraAddress NTSC-U/C: 0x00161468
     * @ghidraAddress PAL: 0x00163fa8
     */
    ~MCGetRemixInfoTask() override;

    /**
     * Prepare the work.
     *
     * @param nPort The memory card slot.
     * @param pszName The remix file.
     * @ghidraAddress NTSC-U/C: 0x001614c8
     * @ghidraAddress PAL: 0x00164008
     */
    void Set(int nPort, const char *pszName);

    /** @return The buffer that receives the description as stored. */
    char *GetBuffer() const {
        return mBuffer.get();
    }

    /** @return The size of the buffer. */
    int GetSize() const {
        return mSize;
    }

private:
    std::unique_ptr<char[]> mBuffer; /*!< Receives the description as stored. */
    int mSize;                       /*!< The size of mBuffer. */
    // The steps belong to the SerialTasks list.
    MCOpenTask *mOpen;   /*!< The step that opens the file. Borrowed. */
    MCReadTask *mRead;   /*!< The step that reads the description. Borrowed. */
    MCCloseTask *mClose; /*!< The step that closes the file. Borrowed. */
};

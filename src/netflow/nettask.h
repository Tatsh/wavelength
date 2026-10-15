#pragma once

#include <list>

#include "netflow/nettasktag.h"
#include "os/task.h"

class NetMultiTask;

/**
 * A step of a request to the online library, run by a NetMultiTask.
 *
 * The RTTI includes the name and records Task as the base. A task that waits for a callback of the
 * library passes the callback a tag from MakeTag(), and the callback finds the task again with
 * FindTag().
 */
class NetTask : public Task {
public:
    /**
     * Construct a task outside any NetMultiTask.
     *
     * The constructor has no out-of-line copy.
     */
    NetTask() : mParent(nullptr) {
    }

    /**
     * End the task, and record a failure in the result of the NetMultiTask that runs it.
     *
     * @param bSucceeded Whether the step succeeded.
     * @param nResult The result the NetMultiTask records after a failure.
     * @ghidraAddress NTSC-U/C: 0x0026a450
     * @ghidraAddress PAL: 0x00273fe0
     */
    void Finish(bool bSucceeded, int nResult);

    /**
     * Create the next tag of a task and copy its text.
     *
     * @param pTask The task.
     * @param pszTag Receives the text of the tag.
     * @ghidraAddress NTSC-U/C: 0x0026a270
     * @ghidraAddress PAL: 0x00273e00
     */
    static void MakeTag(NetTask *pTask, char *pszTag);

    /**
     * Remove the first tag of a task.
     *
     * @param pTask The task.
     * @ghidraAddress NTSC-U/C: 0x0026a398
     * @ghidraAddress PAL: 0x00273f28
     */
    static void RemoveTag(NetTask *pTask);

    /**
     * Find the task of a tag.
     *
     * @param pszTag The text of the tag.
     * @return The task, or null when the tag was removed.
     * @ghidraAddress NTSC-U/C: 0x0026a480
     * @ghidraAddress PAL: 0x00274010
     */
    static NetTask *FindTag(const char *pszTag);

    NetMultiTask *mParent; /*!< The NetMultiTask that runs the task, or null. */

    /**
     * The tags MakeTag() created and RemoveTag() has not removed.
     *
     * @ghidraAddress NTSC-U/C: 0x0043e608
     */
    static std::list<NetTaskTag> sTags;

    /**
     * The number the next tag takes.
     *
     * @ghidraAddress NTSC-U/C: 0x003b0bd4
     * @ghidraAddress PAL: 0x0042faa4
     */
    static int sNextTag;

protected:
    /**
     * Remove the tag of the task.
     *
     * @ghidraAddress NTSC-U/C: 0x0026a430
     * @ghidraAddress PAL: 0x00273fc0
     */
    void OnStop() override;
};

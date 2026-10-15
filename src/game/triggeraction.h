#pragma once

#include "os/debug.h"
#include "rnd/rndmanager.h"
#include "script/dataarray.h"

/**
 * Something a trigger does when its conditions hold.
 *
 * The RTTI includes the name. Each kind of action is a subclass, built by New() from one node of
 * the `actions` entry of a trigger. An action runs once through Exec() after its delay. An action
 * that takes time hands itself to TriggerMgr::AddRunning(), which polls it each frame until Poll()
 * reports it done.
 */
class TriggerAction {
public:
    /**
     * Build the action a node describes.
     *
     * Node 0 names the kind. An unrecognised name becomes a DataFuncAction that runs the node as a
     * script command.
     *
     * @param pAction The node.
     * @return The action.
     * @ghidraAddress NTSC-U/C: 0x001ffef0
     * @ghidraAddress PAL: 0x00208c90
     */
    static TriggerAction *New(DataArray *pAction);

    /**
     * Release the action.
     *
     * @ghidraAddress NTSC-U/C: 0x00374670
     * @ghidraAddress PAL: 0x003e2da0
     */
    virtual ~TriggerAction() {
    }

    /**
     * Run the action.
     */
    virtual void Exec() = 0;

    /**
     * Advance an action that TriggerMgr::AddRunning() received.
     *
     * The base action finishes at once.
     *
     * @return Whether the action is done.
     * @ghidraAddress NTSC-U/C: 0x003746a0
     * @ghidraAddress PAL: 0x003e2dd0
     */
    virtual bool Poll() {
        return true;
    }

    /**
     * Run the action now, or schedule it when it has a delay.
     *
     * The clock of the action becomes the current clock of TheTriggerMgr first.
     *
     * @ghidraAddress NTSC-U/C: 0x00202328
     * @ghidraAddress PAL: 0x0020b0c8
     */
    void Start();

    float mDelay; /*!< Time on the clock of the action before Exec() runs. */
    int mClock;   /*!< The TriggerMgr::Clock the action runs on. */

protected:
    /**
     * Find a renderer object of a class by name, warning when there is none.
     *
     * Every constructor that resolves an object expands it.
     *
     * @param pAction The node of the action, which identifies it in the warning.
     * @param pszName The name of the object.
     * @return The object, or null.
     */
    template <class T>
    static T *FindObject(DataArray *pAction, const char *pszName) {
        T *pObject = dynamic_cast<T *>(TheManager.Find(pszName));
        if (pObject == nullptr) {
            DebugWarn("Couldn't find '%s' object (file %s, line %d)",
                      pszName,
                      pAction->mFile,
                      pAction->mLine);
        }
        return pObject;
    }
};

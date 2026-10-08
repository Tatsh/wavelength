#pragma once

#include <list>

#include "game/triggerhandler.h"
#include "script/dataarray.h"

/**
 * The triggers that wait on one kind of event.
 *
 * TriggerMgr includes one per Type, indexed by the type. The object is the 8-byte list of its
 * handlers, each built from one entry of a trigger file and owned by the event.
 */
class TriggerEvent {
public:
    /** Kinds of event, in the order of the names a trigger file and the debug list use. */
    enum Type {
        kTypeBeat = 0,                 /*!< `beat`, a note of the background music. */
        kTypeTime = 1,                 /*!< `time`, a new frame of the clocks. */
        kTypeComponentSelect = 2,      /*!< `component_select`. */
        kTypeComponentSelectStart = 3, /*!< `component_select_start`. */
        kTypeComponentFocus = 4,       /*!< `component_focus`. */
        kTypeScreenChange = 5,         /*!< `screen_change`. */
        kTypeButton = 6,               /*!< `button`. */
        kTypeBegin = 7,                /*!< `begin`. */
        kTypeEnd = 8,                  /*!< `end`. */
        kTypeStageComplete = 9,        /*!< `stage_complete`. */
        kTypeScore = 10,               /*!< `score`. */
        kTypeHit = 11,                 /*!< `hit`. */
        kTypeMiss = 12,                /*!< `miss`. */
        kTypePass = 13,                /*!< `pass`. The shipped build never fires it. */
        kTypeGem = 14,                 /*!< `gem`. */
        kTypeNewBar = 15,              /*!< `new_bar`. */
        kTypePhraseEnd = 16,           /*!< `phrase_end`. */
        kTypePhraseCapture = 17,       /*!< `phrase_capture`. */
        kTypePhraseMiss = 18,          /*!< `phrase_miss`. */
        kTypeNewTrack = 19,            /*!< `new_track`. */
        kTypePowerupCapture = 20,      /*!< `powerup_capture`. The shipped build never fires it. */
        kTypePowerupDeploy = 21,       /*!< `powerup_deploy`. The shipped build never fires it. */
        kTypeNewLeader = 22,           /*!< `new_leader`. The shipped build never fires it. */
        kTypeHealth = 23,              /*!< `health`. */
        kTypeDyingChange = 24,         /*!< `dying_change`. */
        kTypeBossJourney = 25,         /*!< `boss_journey`. */
        kTypeLyric = 26,               /*!< `lyric`. */
        kTypePathUnlocked = 27,        /*!< `path_unlocked`. */
        kNumTypes = 28,                /*!< The number of kinds, and the unrecognised result. */
    };

    /**
     * Find the kind of event a node of a trigger file names.
     *
     * An unrecognised name produces a warning with the file and the line of the array.
     *
     * @param pArray The array.
     * @param nNode The node with the name.
     * @return One of Type, kNumTypes for an unrecognised name.
     * @ghidraAddress NTSC-U/C: 0x001ff540
     * @ghidraAddress PAL: 0x002082e0
     */
    static int FindType(DataArray *pArray, int nNode);

    /**
     * Run every handler of the event.
     *
     * Each handler tests its conditions and starts its actions.
     *
     * @ghidraAddress NTSC-U/C: 0x001ff3c0
     * @ghidraAddress PAL: 0x00208160
     */
    void Fire();

    /**
     * Delete every handler and empty the list.
     *
     * @ghidraAddress NTSC-U/C: 0x001ff420
     * @ghidraAddress PAL: 0x002081c0
     */
    void Clear();

    /**
     * Build a handler from an entry of a trigger file and append it.
     *
     * @param pTrigger The entry. Node 0 names the event.
     * @ghidraAddress NTSC-U/C: 0x001ff490
     * @ghidraAddress PAL: 0x00208230
     */
    void AddHandler(DataArray *pTrigger);

private:
    std::list<TriggerHandler *> mHandlers;
};

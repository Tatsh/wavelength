#pragma once

/**
 * Registry of the triggers that fire actions on game and display events.
 *
 * The name is inferred from the "trigger" section of the configuration Init() reads and from the
 * TriggerEvent, TriggerAction, and TriggerCondition classes the RTTI includes. The one instance is
 * TheTriggerMgr. Its members are not yet declared. Init() sets one bit of the word at `+0x194` for
 * each debug channel the configuration lists.
 */
class TriggerMgr {
public:
    /**
     * Read the "trigger" section of the configuration.
     *
     * @ghidraAddress NTSC-U/C: 0x001fdb38
     * @ghidraAddress PAL: 0x002068d8
     */
    void Init();

    /**
     * Release every registered trigger.
     *
     * @ghidraAddress NTSC-U/C: 0x001fe6e0
     * @ghidraAddress PAL: 0x00207480
     */
    void Terminate();

    /**
     * Fire the triggers that wait on the start of a bar.
     *
     * The bar is stored for the triggers' actions to read.
     *
     * @param nBar The bar.
     * @ghidraAddress NTSC-U/C: 0x001fef50
     * @ghidraAddress PAL: 0x00207cf0
     */
    void NewBarEvent(int nBar);

    /**
     * Fire the triggers that wait on the end of a phrase.
     *
     * @param nTrack The track the phrase ended on.
     * @ghidraAddress NTSC-U/C: 0x001fefa8
     * @ghidraAddress PAL: 0x00207d48
     */
    void PhraseEndEvent(int nTrack);

    /**
     * Fire the triggers that wait on a missed phrase.
     *
     * @param nPlayer The player that missed the phrase.
     * @ghidraAddress NTSC-U/C: 0x001ff0a8
     * @ghidraAddress PAL: 0x00207e48
     */
    void PhraseMissEvent(int nPlayer);
};

/**
 * The trigger registry.
 *
 * @ghidraAddress NTSC-U/C: 0x0043b700
 */
extern TriggerMgr TheTriggerMgr;

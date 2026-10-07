#pragma once

#include "ui/uicomponentfocuschangemsg.h"
#include "ui/uicomponentselectmsg.h"
#include "ui/uicomponentselectstartmsg.h"
#include "ui/uiscreenchangemsg.h"

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

    /**
     * Fire the triggers that wait on a lyric.
     *
     * The lyric is stored for the triggers' actions to read.
     *
     * @param pszLyric The lyric text.
     * @ghidraAddress NTSC-U/C: 0x001ff308
     * @ghidraAddress PAL: 0x002080a8
     */
    void LyricEvent(const char *pszLyric);

    /**
     * Fire the triggers that wait on the front end starting.
     *
     * The three values are stored for the triggers' actions to read, together with a new random
     * value. The metagame passes its clock twice and 0. The name is inferred.
     *
     * @param flStart The first value.
     * @param flTime The second value.
     * @param flValue The third value.
     * @ghidraAddress NTSC-U/C: 0x001feb88
     * @ghidraAddress PAL: 0x00207928
     */
    void MetagameEvent(float flStart, float flTime, float flValue);

    /**
     * Fire the triggers that wait on a component being chosen.
     *
     * The names of the component, its panel, and its screen are stored for the triggers' actions
     * to read.
     *
     * @param pMsg The message that reported the choice.
     * @ghidraAddress NTSC-U/C: 0x001fe7d8
     * @ghidraAddress PAL: 0x00207578
     */
    void ComponentSelectEvent(UIComponentSelectMsg *pMsg);

    /**
     * Fire the triggers that wait on the choice of a component beginning.
     *
     * @param pMsg The message that reported the choice.
     * @ghidraAddress NTSC-U/C: 0x001fe888
     * @ghidraAddress PAL: 0x00207628
     */
    void ComponentSelectStartEvent(UIComponentSelectStartMsg *pMsg);

    /**
     * Fire the triggers that wait on the focus moving between components.
     *
     * @param pMsg The message that reported the move.
     * @ghidraAddress NTSC-U/C: 0x001fe938
     * @ghidraAddress PAL: 0x002076d8
     */
    void ComponentFocusEvent(UIComponentFocusChangeMsg *pMsg);

    /**
     * Fire the triggers that wait on a move between screens.
     *
     * @param pMsg The message that reported the move.
     * @ghidraAddress NTSC-U/C: 0x001feae0
     * @ghidraAddress PAL: 0x00207880
     */
    void ScreenChangeEvent(UIScreenChangeMsg *pMsg);

    /**
     * Fire the triggers that wait on a button.
     *
     * @param nPlayer The controller.
     * @param nButtons The buttons the controller holds.
     * @ghidraAddress NTSC-U/C: 0x001febc8
     * @ghidraAddress PAL: 0x00207968
     */
    void ButtonEvent(int nPlayer, int nButtons);
};

/**
 * The trigger registry.
 *
 * @ghidraAddress NTSC-U/C: 0x0043b700
 */
extern TriggerMgr TheTriggerMgr;

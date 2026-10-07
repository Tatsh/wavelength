#pragma once

#include "app/msgsink.h"
#include "game/inputevents.h"

/**
 * Rules of one World, the base of the game, remix, and duel logic.
 *
 * The RTTI records the class as deriving from MsgSink. Only the members GameLogic uses are
 * declared. Every handler from Start() through IsPlaying() is pure in this class except
 * SetPaused(), and their names are inferred from the GameLogic bodies. The world's input commands
 * pass each controller event to the HandleInput() overload for its type.
 */
class WorldLogic : public MsgSink {
public:
    /**
     * Construct the logic and give the input of every controller to the world.
     *
     * @ghidraAddress NTSC-U/C: 0x00146a18
     * @ghidraAddress PAL: 0x001483a8
     */
    WorldLogic();

    /**
     * Give the input of every controller back to the menus and release the logic.
     *
     * @ghidraAddress NTSC-U/C: 0x00146a90
     * @ghidraAddress PAL: 0x00148430
     */
    ~WorldLogic() override;

    /**
     * Act on a message, handling the controller selection the logic schedules.
     *
     * @param pMsg The message.
     * @ghidraAddress NTSC-U/C: 0x00146db0
     */
    void DispatchPriv(Message *pMsg) override;

    /** Start the song. */
    virtual void Start() = 0;

    /** Stop the song. */
    virtual void Stop() = 0;

    /**
     * Report whether the song has ended.
     *
     * @return Whether the song has ended.
     */
    virtual bool IsFinished() const = 0;

    /**
     * Report whether a player quit the song from the pause screen.
     *
     * @return Whether a player quit.
     */
    virtual int HasQuit() const = 0;

    /**
     * Report the flag the derived logic sets after HasQuit(). Its purpose is not yet recovered.
     *
     * @return The flag.
     */
    virtual int GetReserved60() const = 0;

    /**
     * Report whether a player is on the freestyle track.
     *
     * @param nPlayer The player.
     * @return Whether the player is on the freestyle track.
     */
    virtual bool IsFreestyling(int nPlayer) = 0;

    /**
     * Report the song position.
     *
     * @return The position, in ticks.
     */
    virtual int GetTick() = 0;

    /** The second member the derived logic supplies. Its purpose is not yet recovered. */
    virtual void Reserved11() = 0;

    /** Advance the logic by one frame. */
    virtual void Poll() = 0;

    /**
     * Report how far the song has played.
     *
     * @return The fraction of the song played, from 0 to 1.
     */
    virtual float GetProgress() = 0;

    /**
     * Act on a request to move a player to a neighbouring track.
     *
     * @param event The event.
     */
    virtual void HandleInput(const RotateEvent &event) = 0;

    /**
     * Act on a note a player played.
     *
     * @param event The event.
     */
    virtual void HandleInput(const PlayNoteEvent &event) = 0;

    /**
     * Act on button 3 of a player.
     *
     * @param event The event.
     */
    virtual void HandleInput(const BtnEvent<3> &event) = 0;

    /**
     * Act on button 4 of a player.
     *
     * @param event The event.
     */
    virtual void HandleInput(const BtnEvent<4> &event) = 0;

    /**
     * Act on button 5 of a player.
     *
     * @param event The event.
     */
    virtual void HandleInput(const BtnEvent<5> &event) = 0;

    /**
     * Act on a request to change the section of a remix.
     *
     * @param event The event.
     */
    virtual void HandleInput(const ChangeSectionEvent &event) = 0;

    /**
     * Act on button 9 of a player.
     *
     * @param event The event.
     */
    virtual void HandleInput(const BtnEvent<9> &event) = 0;

    /**
     * Act on button 10 of a player.
     *
     * @param event The event.
     */
    virtual void HandleInput(const BtnEvent<10> &event) = 0;

    /**
     * Act on button 8 of a player.
     *
     * @param event The event.
     */
    virtual void HandleInput(const BtnEvent<8> &event) = 0;

    /**
     * Act on stick event 2 of a player.
     *
     * @param event The event.
     */
    virtual void HandleInput(const StickEvent<2> &event) = 0;

    /**
     * Act on stick event 6 of a player.
     *
     * @param event The event.
     */
    virtual void HandleInput(const StickEvent<6> &event) = 0;

    /**
     * Pause or resume.
     *
     * Resuming schedules the controller check. The two remaining arguments are passed through
     * unchanged by every override recovered so far.
     *
     * @param bPaused Whether to pause.
     * @param nPad The pad that requested the change.
     * @param nReason The reason the change was requested.
     * @ghidraAddress NTSC-U/C: 0x00146bc0
     * @ghidraAddress PAL: 0x00148570
     */
    virtual void SetPaused(bool bPaused, int nPad, int nReason);

    /**
     * Report whether the song is playing.
     *
     * @return Whether the song is playing.
     */
    virtual bool IsPlaying() const = 0;

    /**
     * Turn every note off on the 16 synthesiser channels.
     *
     * @ghidraAddress NTSC-U/C: 0x00146b30
     * @ghidraAddress PAL: 0x001484e0
     */
    void AllNotesOff();

    /**
     * Schedule a check of the first connected pad one tick from now.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00146be0
     * @ghidraAddress PAL: 0x00148590
     */
    void ScheduleControllerCheck();
};

#pragma once

#include "app/msgsink.h"
#include "game/btnevent.h"
#include "game/changesectionevent.h"
#include "game/playnoteevent.h"
#include "game/rotateevent.h"
#include "game/stickevent.h"
#include "msg/message.h"

/**
 * Rules of one World, the base of the game, remix, and duel logic.
 *
 * The RTTI records the class as deriving from MsgSink. Every handler from Start() through
 * IsPlaying() is pure in this class except SetPaused(). The world's input commands pass each
 * controller event to the HandleInput() overload for its type. Only the members the derived
 * logics here use are declared.
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
     * Release the logic.
     *
     * @ghidraAddress NTSC-U/C: 0x00146a90
     * @ghidraAddress PAL: 0x00148430
     */
    ~WorldLogic() override;

    /**
     * Handle a message sent to the logic.
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
     * Report whether the song is over.
     *
     * @return True once the song is over.
     */
    virtual bool IsFinished() const = 0;

    /**
     * Report whether a player quit the song.
     *
     * @return Non-zero when a player quit.
     */
    virtual int HasQuit() const = 0;

    /**
     * Report whether the song is to start again once it ends.
     *
     * World::IsRestartRequested() returns the value.
     *
     * @return Non-zero when the song restarts.
     */
    virtual int IsRestartRequested() const = 0;

    /**
     * Report whether a player plays the freestyle track.
     *
     * @param nPlayer The player index.
     * @return True while the player plays the freestyle track.
     */
    virtual bool IsFreestyling(int nPlayer) = 0;

    /**
     * Report the song position.
     *
     * @return The song position, in ticks.
     */
    virtual int GetTick() = 0;

    /**
     * Report the song time.
     *
     * @return The song time, in milliseconds.
     */
    virtual float GetTime() = 0;

    /** Advance the logic once per frame. */
    virtual void Poll() = 0;

    /**
     * Report how much of the song has been played.
     *
     * @return The fraction played.
     */
    virtual float GetProgress() = 0;

    /**
     * Act on a rotation.
     *
     * @param event The event.
     */
    virtual void HandleInput(const RotateEvent &event) = 0;

    /**
     * Act on a played note.
     *
     * @param event The event.
     */
    virtual void HandleInput(const PlayNoteEvent &event) = 0;

    /**
     * Act on a button event.
     *
     * @param event The event.
     */
    virtual void HandleInput(const BtnEvent<3> &event) = 0;

    /**
     * Act on a button event.
     *
     * @param event The event.
     */
    virtual void HandleInput(const BtnEvent<4> &event) = 0;

    /**
     * Act on a button event.
     *
     * @param event The event.
     */
    virtual void HandleInput(const BtnEvent<5> &event) = 0;

    /**
     * Act on a section change.
     *
     * @param event The event.
     */
    virtual void HandleInput(const ChangeSectionEvent &event) = 0;

    /**
     * Act on a button event.
     *
     * @param event The event.
     */
    virtual void HandleInput(const BtnEvent<9> &event) = 0;

    /**
     * Act on a button event.
     *
     * @param event The event.
     */
    virtual void HandleInput(const BtnEvent<10> &event) = 0;

    /**
     * Act on a button event.
     *
     * @param event The event.
     */
    virtual void HandleInput(const BtnEvent<8> &event) = 0;

    /**
     * Act on a stick event.
     *
     * @param event The event.
     */
    virtual void HandleInput(const StickEvent<2> &event) = 0;

    /**
     * Act on a stick event.
     *
     * @param event The event.
     */
    virtual void HandleInput(const StickEvent<6> &event) = 0;

    /**
     * Pause or resume the song.
     *
     * @param bPaused Pause the song.
     * @param nPad The controller that paused the song, or -1.
     * @param nReason Non-zero when a disconnected controller paused the song.
     * @ghidraAddress NTSC-U/C: 0x00146bc0
     * @ghidraAddress PAL: 0x00148570
     */
    virtual void SetPaused(bool bPaused, int nPad, int nReason);

    /**
     * Report whether the song is playing.
     *
     * @return True while the song plays.
     */
    virtual bool IsPlaying() const = 0;

    /**
     * Silence every synthesiser channel.
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

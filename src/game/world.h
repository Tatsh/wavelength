#pragma once

#include "app/msgsink.h"
#include "game/inputevents.h"

/**
 * One game in progress under one rule set.
 *
 * The RTTI records the class as deriving from MsgSink. Game, Remix, and Duel derive from it, one
 * for each rule set WorldMgr::Load() can build. Only the members its callers here use are
 * declared.
 */
class World : public MsgSink {
public:
    /**
     * Construct a world with no game state.
     *
     * @ghidraAddress NTSC-U/C: 0x00144330
     * @ghidraAddress PAL: 0x00145cc0
     */
    World();

    /**
     * Release the world.
     *
     * @ghidraAddress NTSC-U/C: 0x001443f0
     * @ghidraAddress PAL: 0x00145d80
     */
    ~World() override;

    /**
     * Pass a rotation to the logic of the world.
     *
     * @param event The event.
     */
    virtual void Handle(const RotateEvent &event) = 0;

    /**
     * Pass a gem button to the logic of the world.
     *
     * @param event The event.
     */
    virtual void Handle(const PlayNoteEvent &event) = 0;

    /**
     * Pass the input event of command 8 to the logic of the world.
     *
     * @param event The event.
     */
    virtual void Handle(const BtnEvent<8> &event) = 0;

    /**
     * Pass the expression stick position to the logic of the world.
     *
     * @param event The event.
     */
    virtual void Handle(const StickEvent<2> &event) = 0;

    /**
     * Pass the position of the second bound stick to the logic of the world.
     *
     * @param event The event.
     */
    virtual void Handle(const StickEvent<6> &event) = 0;

    /**
     * Pass the pause button to the logic of the world.
     *
     * @param event The event.
     */
    virtual void Handle(const BtnEvent<3> &event) = 0;

    /**
     * Pass the input event of command 4 to the logic of the world.
     *
     * @param event The event.
     */
    virtual void Handle(const BtnEvent<4> &event) = 0;

    /**
     * Pass the input event of command 5 to the logic of the world.
     *
     * @param event The event.
     */
    virtual void Handle(const BtnEvent<5> &event) = 0;

    /**
     * Pass a section change to the logic of the world.
     *
     * @param event The event.
     */
    virtual void Handle(const ChangeSectionEvent &event) = 0;

    /**
     * Pass the input event of command 9 to the logic of the world.
     *
     * @param event The event.
     */
    virtual void Handle(const BtnEvent<9> &event) = 0;

    /**
     * Pass the input event of command 10 to the logic of the world.
     *
     * @param event The event.
     */
    virtual void Handle(const BtnEvent<10> &event) = 0;

    /**
     * Stop the song and release what the world created to play it.
     *
     * @ghidraAddress NTSC-U/C: 0x001446c0
     * @ghidraAddress PAL: 0x00146050
     */
    void Stop();

    /**
     * Report whether a player is in a freestyle while the song plays.
     *
     * @param nPlayer The player.
     * @return Whether the player is in a freestyle, and false while the song does not play.
     * @ghidraAddress NTSC-U/C: 0x00144948
     * @ghidraAddress PAL: 0x001462d8
     */
    bool IsFreestyling(int nPlayer);
};

#pragma once

#include "app/msgsink.h"
#include "game/btnevent.h"
#include "game/changesectionevent.h"
#include "game/playnoteevent.h"
#include "game/rotateevent.h"
#include "game/song.h"
#include "game/stickevent.h"
#include "game/worldlogic.h"
#include "msg/message.h"

/**
 * One game in progress under one rule set.
 *
 * The RTTI records the class as deriving from MsgSink. Game, Remix, and Duel derive from it, one
 * for each rule set WorldMgr::Load() can build.
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
     * Act on a message sent to the world.
     *
     * @param pMsg The message.
     * @ghidraAddress NTSC-U/C: 0x00145778
     * @ghidraAddress PAL: 0x00147108
     */
    void DispatchPriv(Message *pMsg) override;

    /**
     * Report the rule set the world plays under.
     *
     * The member is pure in this class.
     *
     * @return One of GameDb::RuleSet.
     */
    virtual int GetRuleSet() const = 0;

    /**
     * Report whether the running world has ended.
     *
     * The member is pure in this class. The name is inferred.
     *
     * @return Whether the world has ended.
     */
    virtual bool IsFinished() = 0;

    /**
     * Report the time the display advances to while the world runs.
     *
     * The name is inferred.
     *
     * @return The time, or 0 when the world is not running.
     * @ghidraAddress NTSC-U/C: 0x001451f8
     * @ghidraAddress PAL: 0x00146b88
     */
    virtual float GetDisplayTime();

    /**
     * Act on a rotation from the input router.
     *
     * The member is pure in this class.
     *
     * @param event The rotation.
     */
    virtual void Handle(const RotateEvent &event) = 0;

    /**
     * Act on a gem button from the input router.
     *
     * The member is pure in this class.
     *
     * @param event The button.
     */
    virtual void Handle(const PlayNoteEvent &event) = 0;

    /**
     * Act on a button event of command 8 from the input router.
     *
     * The member is pure in this class.
     *
     * @param event The button.
     */
    virtual void Handle(const BtnEvent<8> &event) = 0;

    /**
     * Act on a stick event of command 2 from the input router.
     *
     * The member is pure in this class.
     *
     * @param event The stick position.
     */
    virtual void Handle(const StickEvent<2> &event) = 0;

    /**
     * Act on a stick event of command 6 from the input router.
     *
     * The member is pure in this class.
     *
     * @param event The stick position.
     */
    virtual void Handle(const StickEvent<6> &event) = 0;

    /**
     * Act on a button event of command 3 from the input router.
     *
     * The member is pure in this class.
     *
     * @param event The button.
     */
    virtual void Handle(const BtnEvent<3> &event) = 0;

    /**
     * Act on a button event of command 4 from the input router.
     *
     * The member is pure in this class.
     *
     * @param event The button.
     */
    virtual void Handle(const BtnEvent<4> &event) = 0;

    /**
     * Act on a button event of command 5 from the input router.
     *
     * The member is pure in this class.
     *
     * @param event The button.
     */
    virtual void Handle(const BtnEvent<5> &event) = 0;

    /**
     * Act on a section change from the input router.
     *
     * The member is pure in this class.
     *
     * @param event The section change.
     */
    virtual void Handle(const ChangeSectionEvent &event) = 0;

    /**
     * Act on a button event of command 9 from the input router.
     *
     * The member is pure in this class.
     *
     * @param event The button.
     */
    virtual void Handle(const BtnEvent<9> &event) = 0;

    /**
     * Act on a button event of command 10 from the input router.
     *
     * The member is pure in this class.
     *
     * @param event The button.
     */
    virtual void Handle(const BtnEvent<10> &event) = 0;

    /**
     * Start loading the assets of the second loading stage.
     *
     * The member is pure in this class. The name is inferred.
     */
    virtual void BeginAssetLoad() = 0;

    /**
     * Advance the second loading stage by one frame.
     *
     * The member is pure in this class. The name is inferred.
     */
    virtual void PollAssetLoad() = 0;

    /**
     * Report whether the second loading stage finished.
     *
     * The member is pure in this class. The name is inferred.
     *
     * @return Whether the assets are loaded.
     */
    virtual bool IsAssetLoadDone() = 0;

    /**
     * Start loading the song, the first loading stage.
     *
     * The member is pure in this class. The name is inferred.
     */
    virtual void BeginSongLoad() = 0;

    /**
     * Advance the first loading stage by one frame.
     *
     * The member is pure in this class. The name is inferred.
     */
    virtual void PollSongLoad() = 0;

    /**
     * Report whether the first loading stage finished.
     *
     * The member is pure in this class. The name is inferred.
     *
     * @return Whether the song is loaded.
     */
    virtual bool IsSongLoadDone() = 0;

    /**
     * Start the sequence that follows the end of the game.
     *
     * The base body is empty. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x003455c0
     */
    virtual void BeginEnding() {
    }

    /**
     * Advance the sequence that follows the end of the game by one frame.
     *
     * The base body is empty. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x003455c8
     */
    virtual void PollEnding() {
    }

    /**
     * Report whether the sequence that follows the end of the game finished.
     *
     * The name is inferred.
     *
     * @return True in this class.
     * @ghidraAddress NTSC-U/C: 0x003455d0
     */
    virtual bool IsEndingDone() {
        return true;
    }

    /**
     * Build the WorldLogic of the rule set and install it with SetLogic().
     *
     * The member is pure in this class. The name is inferred.
     */
    virtual void CreateLogic() = 0;

    /**
     * Record a rotation as a serializable input command.
     *
     * @param event The rotation.
     * @ghidraAddress NTSC-U/C: 0x001441d0
     * @ghidraAddress PAL: 0x00145b60
     */
    virtual void Record(const RotateEvent &event);

    /**
     * Record a gem button as a serializable input command.
     *
     * @param event The button.
     * @ghidraAddress NTSC-U/C: 0x001441f0
     * @ghidraAddress PAL: 0x00145b80
     */
    virtual void Record(const PlayNoteEvent &event);

    /**
     * Record a button event of command 8.
     *
     * @param event The button.
     * @ghidraAddress NTSC-U/C: 0x00144210
     * @ghidraAddress PAL: 0x00145ba0
     */
    virtual void Record(const BtnEvent<8> &event);

    /**
     * Record a stick event of command 2.
     *
     * @param event The stick position.
     * @ghidraAddress NTSC-U/C: 0x00144230
     * @ghidraAddress PAL: 0x00145bc0
     */
    virtual void Record(const StickEvent<2> &event);

    /**
     * Record a stick event of command 6.
     *
     * @param event The stick position.
     * @ghidraAddress NTSC-U/C: 0x00144250
     * @ghidraAddress PAL: 0x00145be0
     */
    virtual void Record(const StickEvent<6> &event);

    /**
     * Record a button event of command 3.
     *
     * @param event The button.
     * @ghidraAddress NTSC-U/C: 0x00144270
     * @ghidraAddress PAL: 0x00145c00
     */
    virtual void Record(const BtnEvent<3> &event);

    /**
     * Record a button event of command 4.
     *
     * @param event The button.
     * @ghidraAddress NTSC-U/C: 0x00144290
     * @ghidraAddress PAL: 0x00145c20
     */
    virtual void Record(const BtnEvent<4> &event);

    /**
     * Record a button event of command 5.
     *
     * @param event The button.
     * @ghidraAddress NTSC-U/C: 0x001442b0
     * @ghidraAddress PAL: 0x00145c40
     */
    virtual void Record(const BtnEvent<5> &event);

    /**
     * Record a section change.
     *
     * @param event The section change.
     * @ghidraAddress NTSC-U/C: 0x001442d0
     * @ghidraAddress PAL: 0x00145c60
     */
    virtual void Record(const ChangeSectionEvent &event);

    /**
     * Record a button event of command 9.
     *
     * @param event The button.
     * @ghidraAddress NTSC-U/C: 0x001442f0
     * @ghidraAddress PAL: 0x00145c80
     */
    virtual void Record(const BtnEvent<9> &event);

    /**
     * Record a button event of command 10.
     *
     * @param event The button.
     * @ghidraAddress NTSC-U/C: 0x00144310
     * @ghidraAddress PAL: 0x00145ca0
     */
    virtual void Record(const BtnEvent<10> &event);

    /**
     * Finish starting the song, after Start() created the logic.
     *
     * The base body is empty. The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x003455d8
     */
    virtual void OnStart() {
    }

    /**
     * Build the track display for the song.
     *
     * The member is pure in this class. The name is inferred.
     *
     * @param nStartTick The tick the song starts at, before its first bar.
     */
    virtual void BuildTracks(int nStartTick) = 0;

    /**
     * Start the song of a world that finished loading.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x001445c0
     * @ghidraAddress PAL: 0x00145f50
     */
    void Start();

    /**
     * Stop the song and release what Start() and LoadAssets() created.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x001446c0
     * @ghidraAddress PAL: 0x00146050
     */
    void Stop();

    /**
     * Report whether the world that ended requested a restart.
     *
     * The name is inferred.
     *
     * @return Whether a restart was requested.
     * @ghidraAddress NTSC-U/C: 0x00144908
     * @ghidraAddress PAL: 0x00146298
     */
    bool IsRestartRequested();

    /**
     * Report whether a player is in a freestyle while the song plays.
     *
     * @param nPlayer The player.
     * @return Whether the player is in a freestyle, and false while the song does not play.
     * @ghidraAddress NTSC-U/C: 0x00144948
     * @ghidraAddress PAL: 0x001462d8
     */
    bool IsFreestyling(int nPlayer);

    /**
     * Advance the world by one frame.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00144988
     * @ghidraAddress PAL: 0x00146318
     */
    void Poll();

    /**
     * Report whether the assets LoadAssets() started are loaded.
     *
     * The name is inferred.
     *
     * @return Whether the world is ready to start.
     * @ghidraAddress NTSC-U/C: 0x00144f80
     * @ghidraAddress PAL: 0x00146910
     */
    bool IsLoaded() const;

    /**
     * Start loading the song and the assets of the world.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00144f90
     * @ghidraAddress PAL: 0x00146920
     */
    void LoadAssets();

    /**
     * Report the song position in ticks.
     *
     * The name is inferred.
     *
     * @return The position.
     * @ghidraAddress NTSC-U/C: 0x00145130
     * @ghidraAddress PAL: 0x00146ac0
     */
    int GetTick();

    /**
     * Report the song position on the song clock.
     *
     * The name is inferred.
     *
     * @return The position.
     * @ghidraAddress NTSC-U/C: 0x001451a8
     * @ghidraAddress PAL: 0x00146b38
     */
    float GetTime();

    /**
     * Destroy the installed logic and install another.
     *
     * The name is inferred.
     *
     * @param pLogic The logic to install.
     * @ghidraAddress NTSC-U/C: 0x00144550
     * @ghidraAddress PAL: 0x00145ee0
     */
    void SetLogic(WorldLogic *pLogic);

    int mState;  /*!< The stage of the world, from loading through the end of the song. */
    Song *mSong; /*!< The song LoadAssets() built. */
    // The members after +0x08 are not yet declared.
};

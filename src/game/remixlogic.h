#pragma once

#include "game/song.h"
#include "game/worldlogic.h"

/**
 * Game rules of a remix session.
 *
 * The RTTI includes the class name and records WorldLogic as the base. The object is 0x1a8 bytes.
 * The vtable overrides every handler WorldLogic declares. Only the members Remix and a remix
 * tutorial use are declared.
 */
class RemixLogic : public WorldLogic {
public:
    /**
     * Construct the rules for a song.
     *
     * @param pSong The song.
     * @ghidraAddress NTSC-U/C: 0x0012fdb0
     * @ghidraAddress PAL: 0x001315d8
     */
    explicit RemixLogic(Song *pSong);

    /**
     * Release the rules.
     *
     * @ghidraAddress NTSC-U/C: 0x00131228
     * @ghidraAddress PAL: 0x00132a40
     */
    ~RemixLogic() override;

    /**
     * Handle a message sent to the logic.
     *
     * @param pMsg The message.
     * @ghidraAddress NTSC-U/C: 0x00136390
     * @ghidraAddress PAL: 0x00137ba8
     */
    void DispatchPriv(Message *pMsg) override;

    /**
     * Start the song.
     *
     * @ghidraAddress NTSC-U/C: 0x001318c8
     * @ghidraAddress PAL: 0x001330d8
     */
    void Start() override;

    /**
     * Stop the song.
     *
     * @ghidraAddress NTSC-U/C: 0x00131c08
     * @ghidraAddress PAL: 0x00133418
     */
    void Stop() override;

    /**
     * Report whether the remix ended.
     *
     * @return Whether the remix ended.
     * @ghidraAddress NTSC-U/C: 0x00131f88
     */
    bool IsFinished() const override;

    /**
     * Report whether a player quit the remix.
     *
     * @return Non-zero when a player quit.
     * @ghidraAddress NTSC-U/C: 0x00131f98
     */
    int HasQuit() const override;

    /**
     * Report whether the song is to start again once it ends.
     *
     * @return Non-zero when the song restarts.
     * @ghidraAddress NTSC-U/C: 0x0033f070
     * @ghidraAddress PAL: 0x003ac5a8
     */
    int IsRestartRequested() const override;

    /**
     * Report whether a player plays the freestyle track.
     *
     * @param nPlayer The player index.
     * @return True while the player plays the freestyle track.
     * @ghidraAddress NTSC-U/C: 0x00131fa0
     * @ghidraAddress PAL: 0x001337b0
     */
    bool IsFreestyling(int nPlayer) override;

    /**
     * Report the song tick.
     *
     * @return The tick.
     * @ghidraAddress NTSC-U/C: 0x00131fe0
     * @ghidraAddress PAL: 0x001337f0
     */
    int GetTick() override;

    /**
     * Report the song time.
     *
     * @return The time.
     * @ghidraAddress NTSC-U/C: 0x00132000
     * @ghidraAddress PAL: 0x00133810
     */
    float GetTime() override;

    /**
     * Advance the logic once per frame.
     *
     * @ghidraAddress NTSC-U/C: 0x00131e98
     * @ghidraAddress PAL: 0x001336a8
     */
    void Poll() override;

    /**
     * Report how much of the song has been played.
     *
     * @return The fraction played.
     * @ghidraAddress NTSC-U/C: 0x00132020
     */
    float GetProgress() override;

    /**
     * Act on a rotation.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00132bd8
     * @ghidraAddress PAL: 0x001343e8
     */
    void HandleInput(const RotateEvent &event) override;

    /**
     * Act on a played note.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00132cf8
     * @ghidraAddress PAL: 0x00134508
     */
    void HandleInput(const PlayNoteEvent &event) override;

    /**
     * Act on a button event.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00132ee0
     * @ghidraAddress PAL: 0x001346f0
     */
    void HandleInput(const BtnEvent<3> &event) override;

    /**
     * Act on a button event.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x0033f078
     * @ghidraAddress PAL: 0x003ac5b0
     */
    void HandleInput(const BtnEvent<4> &event) override;

    /**
     * Act on a button event.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x0033f080
     * @ghidraAddress PAL: 0x003ac5b8
     */
    void HandleInput(const BtnEvent<5> &event) override;

    /**
     * Act on a section change.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00132f50
     * @ghidraAddress PAL: 0x00134760
     */
    void HandleInput(const ChangeSectionEvent &event) override;

    /**
     * Act on a button event.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x001331d8
     * @ghidraAddress PAL: 0x001349e8
     */
    void HandleInput(const BtnEvent<9> &event) override;

    /**
     * Act on a button event.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x001333d0
     * @ghidraAddress PAL: 0x00134be0
     */
    void HandleInput(const BtnEvent<10> &event) override;

    /**
     * Act on a button event.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00132e50
     * @ghidraAddress PAL: 0x00134660
     */
    void HandleInput(const BtnEvent<8> &event) override;

    /**
     * Act on a stick event.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00133330
     */
    void HandleInput(const StickEvent<2> &event) override;

    /**
     * Act on a stick event.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00133380
     */
    void HandleInput(const StickEvent<6> &event) override;

    /**
     * Pause or resume the song.
     *
     * @param bPaused Pause the song.
     * @param nPad The controller that paused the song, or -1.
     * @param nReason Non-zero when a disconnected controller paused the song.
     * @ghidraAddress NTSC-U/C: 0x00132090
     * @ghidraAddress PAL: 0x001338a0
     */
    void SetPaused(bool bPaused, int nPad, int nReason) override;

    /**
     * Report whether the song is playing.
     *
     * @return True while the song plays.
     * @ghidraAddress NTSC-U/C: 0x0033f088
     */
    bool IsPlaying() const override;

    /**
     * Turn looping of a lane's track on or off.
     *
     * @param nLane The lane.
     * @param bLooping Loop the track.
     * @ghidraAddress NTSC-U/C: 0x00134ca0
     * @ghidraAddress PAL: 0x001364b0
     */
    void SetLooping(int nLane, bool bLooping);

    /**
     * Clear the gems of every section.
     *
     * @ghidraAddress NTSC-U/C: 0x00135510
     * @ghidraAddress PAL: 0x00136d10
     */
    void ClearGems();

    /**
     * Open the remix menu.
     *
     * @ghidraAddress NTSC-U/C: 0x001355c0
     * @ghidraAddress PAL: 0x00136dc0
     */
    void OpenMenu();

    /**
     * Open the tempo control.
     *
     * @ghidraAddress NTSC-U/C: 0x001355e0
     * @ghidraAddress PAL: 0x00136de0
     */
    void OpenTempo();

    /**
     * Close the tempo control.
     *
     * @ghidraAddress NTSC-U/C: 0x00135600
     * @ghidraAddress PAL: 0x00136e00
     */
    void CloseTempo();

    /**
     * Raise the tempo by one step.
     *
     * @ghidraAddress NTSC-U/C: 0x00135620
     * @ghidraAddress PAL: 0x00136e20
     */
    void RaiseTempo();

    /**
     * Lower the tempo by one step.
     *
     * @ghidraAddress NTSC-U/C: 0x00135648
     * @ghidraAddress PAL: 0x00136e48
     */
    void LowerTempo();
};

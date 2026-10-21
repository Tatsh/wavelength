#pragma once

#include <memory>
#include <vector>

#include "game/btnevent.h"
#include "game/changesectionevent.h"
#include "game/freestyletrack.h"
#include "game/fxmgr.h"
#include "game/gametrackselector.h"
#include "game/netarbiter.h"
#include "game/pitchtrack.h"
#include "game/player.h"
#include "game/playmap.h"
#include "game/playnoteevent.h"
#include "game/remixhud.h"
#include "game/remixtrack.h"
#include "game/rotateevent.h"
#include "game/sectionboundaries.h"
#include "game/song.h"
#include "game/stickevent.h"
#include "game/voxtrack.h"
#include "game/worldbeat.h"
#include "game/worldlogic.h"
#include "met/metagame.h"
#include "msg/arbiterpacket.h"
#include "msg/editgempacket.h"
#include "msg/erasebarpacket.h"
#include "msg/erasesectionpacket.h"
#include "msg/fxpacket.h"
#include "msg/gameendedmsg.h"
#include "msg/keyboardkeymsg.h"
#include "msg/message.h"
#include "msg/mutevocalspacket.h"
#include "msg/playerabortedmsg.h"
#include "msg/previewpacket.h"
#include "msg/remixerupdatepacket.h"
#include "msg/sectionchangepacket.h"
#include "os/binstream.h"
#include "os/command.h"
#include "os/ptr.h"

class RemixTutorial;

/**
 * Game rules of a remix session.
 *
 * The RTTI includes the class name and records WorldLogic as the base. The object is 0x1a8 bytes.
 * The vtable overrides every handler WorldLogic declares. The players edit the patterns of the
 * remix tracks while the song loops over one playable section, and a preview plays the whole remix
 * from a bar. The consoles of an online session synchronise through the packets DispatchPriv()
 * receives. Every routine name other than those of the WorldLogic overrides and of the members a
 * remix tutorial calls is inferred.
 */
class RemixLogic : public WorldLogic {
public:
    /** Values of mState. */
    enum State {
        kStatePlaying = 0,     /*!< The song plays. */
        kStatePaused = 1,      /*!< The pause menu shows. */
        kStateTutorialEnd = 2, /*!< The menu at the end of the tutorial shows. */
        kStateEnding = 3,      /*!< The song fades out. */
        kStateFinished = 4,    /*!< The remix ended. */
    };

    /**
     * Construct the rules for a song.
     *
     * The constructor builds the tracks, the players, and a RemixHUD for each controller, reads a
     * saved remix outside the tutorial, and assigns the players to the tracks. A read-only remix
     * starts no section change.
     *
     * @param pSong The song.
     * @ghidraAddress NTSC-U/C: 0x0012fdb0
     * @ghidraAddress PAL: 0x001315d8
     */
    explicit RemixLogic(Song *pSong);

    /**
     * Save the remix outside the tutorial and release the rules.
     *
     * The host of an online session reports an empty score list first.
     *
     * @ghidraAddress NTSC-U/C: 0x00131228
     * @ghidraAddress PAL: 0x00132a40
     */
    ~RemixLogic() override;

    /**
     * Pass a packet of the session to its handler.
     *
     * @param pMsg The message.
     * @return False for a message a handler of this class receives, otherwise the result of
     *         WorldLogic::DispatchPriv().
     * @ghidraAddress NTSC-U/C: 0x00136390
     * @ghidraAddress PAL: 0x00137ba8
     */
    bool DispatchPriv(Message *pMsg) override;

    /**
     * Start the tracks, the players, the background music, and the intro of the song.
     *
     * A read-only remix starts a preview at once.
     *
     * @ghidraAddress NTSC-U/C: 0x001318c8
     * @ghidraAddress PAL: 0x001330d8
     */
    void Start() override;

    /**
     * Stop the song and the tracks.
     *
     * @ghidraAddress NTSC-U/C: 0x00131c08
     * @ghidraAddress PAL: 0x00133418
     */
    void Stop() override;

    /**
     * Report whether the remix ended.
     *
     * @return Whether mState is kStateFinished.
     * @ghidraAddress NTSC-U/C: 0x00131f88
     * @ghidraAddress PAL: 0x00133798
     */
    bool IsFinished() const override;

    /**
     * Report whether a player quit the remix from a menu.
     *
     * @return mQuit.
     * @ghidraAddress NTSC-U/C: 0x00131f98
     * @ghidraAddress PAL: 0x001337a8
     */
    int HasQuit() const override;

    /**
     * Report that no restart was requested. The body returns zero.
     *
     * @return Zero.
     * @ghidraAddress NTSC-U/C: 0x0033f070
     * @ghidraAddress PAL: 0x003ac5a8
     */
    int IsRestartRequested() const override {
        return 0;
    }

    /**
     * Report whether a player plays the freestyle track.
     *
     * @param nPlayer The player index.
     * @return True while the player plays mFreestyleTrack.
     * @ghidraAddress NTSC-U/C: 0x00131fa0
     * @ghidraAddress PAL: 0x001337b0
     */
    bool IsFreestyling(int nPlayer) override;

    /**
     * Report the song position in ticks.
     *
     * @return The tick of the song clock.
     * @ghidraAddress NTSC-U/C: 0x00131fe0
     * @ghidraAddress PAL: 0x001337f0
     */
    int GetTick() override;

    /**
     * Report the song position on the song clock.
     *
     * @return The time of the song clock.
     * @ghidraAddress NTSC-U/C: 0x00132000
     * @ghidraAddress PAL: 0x00133810
     */
    float GetTime() override;

    /**
     * End the tutorial once its script is done, and stop the song once the outro and the fade are
     * done.
     *
     * @ghidraAddress NTSC-U/C: 0x00131e98
     * @ghidraAddress PAL: 0x001336a8
     */
    void Poll() override;

    /**
     * Report how much of the song has been played.
     *
     * Outside a preview the body reports 50.
     *
     * @return During a preview, the song position past mLoopBar as a fraction of the song length,
     *         otherwise 50.
     * @ghidraAddress NTSC-U/C: 0x00132020
     * @ghidraAddress PAL: 0x00133830
     */
    float GetProgress() override;

    /**
     * Rotate a player to the next or the previous track while the song plays.
     *
     * A player on the freestyle track does not rotate, and a preview ignores the rotation.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00132bd8
     * @ghidraAddress PAL: 0x001343e8
     */
    void HandleInput(const RotateEvent &event) override;

    /**
     * Pass a gem button to its player while the song plays.
     *
     * In an online session only a player whose track the arbitration confirms plays the note.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00132cf8
     * @ghidraAddress PAL: 0x00134508
     */
    void HandleInput(const PlayNoteEvent &event) override;

    /**
     * Pause the song while it plays.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00132ee0
     * @ghidraAddress PAL: 0x001346f0
     */
    void HandleInput(const BtnEvent<3> &event) override;

    /**
     * Ignore a button event of command 4. The body is empty.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x0033f078
     * @ghidraAddress PAL: 0x003ac5b0
     */
    void HandleInput([[maybe_unused]] const BtnEvent<4> &event) override {
    }

    /**
     * Ignore a button event of command 5. The body is empty.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x0033f080
     * @ghidraAddress PAL: 0x003ac5b8
     */
    void HandleInput([[maybe_unused]] const BtnEvent<5> &event) override {
    }

    /**
     * Move to the next or the previous playable section.
     *
     * A guest of an online session sends a request to the chat of the session instead.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00132f50
     * @ghidraAddress PAL: 0x00134760
     */
    void HandleInput(const ChangeSectionEvent &event) override;

    /**
     * Start or stop a preview.
     *
     * A read-only remix ignores the event, and a guest of an online session sends a request to
     * the chat of the session instead.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x001331d8
     * @ghidraAddress PAL: 0x001349e8
     */
    void HandleInput(const BtnEvent<9> &event) override;

    /**
     * Pass a button event of command 10 to its player while the song plays.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x001333d0
     * @ghidraAddress PAL: 0x00134be0
     */
    void HandleInput(const BtnEvent<10> &event) override;

    /**
     * Pass a button event of command 8 to its player while the song plays outside a preview.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00132e50
     * @ghidraAddress PAL: 0x00134660
     */
    void HandleInput(const BtnEvent<8> &event) override;

    /**
     * Pass a stick event of command 2 to its player while the song plays.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00133330
     * @ghidraAddress PAL: 0x00134b40
     */
    void HandleInput(const StickEvent<2> &event) override;

    /**
     * Pass a stick event of command 6 to its player while the song plays.
     *
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00133380
     * @ghidraAddress PAL: 0x00134b90
     */
    void HandleInput(const StickEvent<6> &event) override;

    /**
     * Pause or resume the song.
     *
     * Pausing opens the pause menu and, outside an online session, stops the song clock.
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
     * @return Whether mState is kStatePlaying.
     * @ghidraAddress NTSC-U/C: 0x0033f088
     * @ghidraAddress PAL: 0x003ac5c0
     */
    bool IsPlaying() const override {
        return mState == kStatePlaying;
    }

    /**
     * Turn looping of a player's track on or off.
     *
     * @param nPlayer The player index.
     * @param bLooping Loop the track.
     * @ghidraAddress NTSC-U/C: 0x00134ca0
     * @ghidraAddress PAL: 0x001364b0
     */
    void SetLooping(int nPlayer, bool bLooping);

    /**
     * Erase the gems of the section at the song position from every pitch track.
     *
     * @ghidraAddress NTSC-U/C: 0x00135510
     * @ghidraAddress PAL: 0x00136d10
     */
    void ClearGems();

    /**
     * Focus or release the remix menu of the first controller.
     *
     * @param bEnable Focus the menu.
     * @ghidraAddress NTSC-U/C: 0x001355c0
     * @ghidraAddress PAL: 0x00136dc0
     */
    void EnableMenu(bool bEnable);

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
     * Raise the tempo by one beat per minute.
     *
     * @ghidraAddress NTSC-U/C: 0x00135620
     * @ghidraAddress PAL: 0x00136e20
     */
    void RaiseTempo();

    /**
     * Lower the tempo by one beat per minute.
     *
     * @ghidraAddress NTSC-U/C: 0x00135648
     * @ghidraAddress PAL: 0x00136e48
     */
    void LowerTempo();

private:
    /**
     * Report the player of a session order.
     *
     * @param nNetOrder The session order.
     * @return The player index.
     * @ghidraAddress NTSC-U/C: 0x00131210
     * @ghidraAddress PAL: 0x00132a28
     */
    int GetPlayerFromNetOrder(int nNetOrder) const;

    /**
     * Act on the choice of the pause menu or of the menu at the end of the tutorial.
     *
     * Resume resumes the song, Quit records the quit and ends the remix, and End exits an online
     * session and ends the remix.
     *
     * @param action The choice.
     * @param pUserData The logic.
     * @ghidraAddress NTSC-U/C: 0x00132280
     * @ghidraAddress PAL: 0x00133a90
     */
    static void OnPauseDialog(Metagame::DialogAction action, void *pUserData);

    /**
     * Build a track for each track of the song.
     *
     * A pitch or a vocal track becomes a remix track, and a guitar or a turntable track becomes
     * mFreestyleTrack.
     *
     * @param pFreestyleType Receives the track type of the freestyle track.
     * @param pFreestyleInstrument Receives the instrument of the freestyle track.
     * @ghidraAddress NTSC-U/C: 0x00132358
     * @ghidraAddress PAL: 0x00133b68
     */
    void BuildTracks(int *pFreestyleType, int *pFreestyleInstrument);

    /**
     * Report whether the song position lies before mLoopBar, where input is refused.
     *
     * A refused press plays a sound, except a note of the intro or of a preview.
     *
     * @param nPressed Non-zero for a press.
     * @param bNote Whether the input is a gem button.
     * @return Whether the input is refused.
     * @ghidraAddress NTSC-U/C: 0x00132a48
     * @ghidraAddress PAL: 0x00134258
     */
    bool IsInputLocked(int nPressed, bool bNote);

    /**
     * End the remix after a quit, or else start the outro and fade the song out.
     *
     * @ghidraAddress NTSC-U/C: 0x00132ad0
     * @ghidraAddress PAL: 0x001342e0
     */
    void EndRemix();

    /**
     * Find the pitch track of a song track.
     *
     * @param nTrack The track's index in the song.
     * @return The track, or null.
     * @ghidraAddress NTSC-U/C: 0x00133450
     * @ghidraAddress PAL: 0x00134c60
     */
    PitchTrack *FindPitchTrack(int nTrack) const;

    /**
     * Find the vocal track of a song track.
     *
     * @param nTrack The track's index in the song.
     * @return The track, or null.
     * @ghidraAddress NTSC-U/C: 0x001334a0
     * @ghidraAddress PAL: 0x00134cb0
     */
    VoxTrack *FindVoxTrack(int nTrack) const;

    /**
     * Mark every remix menu active or inactive.
     *
     * @param nActive Non-zero for active.
     * @ghidraAddress NTSC-U/C: 0x001334f0
     * @ghidraAddress PAL: 0x00134d00
     */
    void SetHudsActive(int nActive);

    /**
     * Show the state of a player's track in the player's remix menu.
     *
     * @param nPlayer The player index.
     * @ghidraAddress NTSC-U/C: 0x00133588
     * @ghidraAddress PAL: 0x00134d98
     */
    void UpdateHud(int nPlayer);

    /**
     * Refresh the remix menu of every player, or only the first in an online session.
     *
     * mUpdateHudsCmd runs it.
     *
     * @ghidraAddress NTSC-U/C: 0x00133790
     * @ghidraAddress PAL: 0x00134fa0
     */
    void UpdateHuds();

    /**
     * Change to a playable section two bars from the song position, or three bars once a third of
     * the current bar has passed.
     *
     * @param nIndex The index in mPlayableSections.
     * @ghidraAddress NTSC-U/C: 0x00133838
     * @ghidraAddress PAL: 0x00135048
     */
    void QueueSectionChange(int nIndex);

    /**
     * Loop the song over a playable section from a bar on.
     *
     * @param nIndex The index in mPlayableSections.
     * @param nBar The bar the section plays from.
     * @ghidraAddress NTSC-U/C: 0x001338a8
     * @ghidraAddress PAL: 0x001350b8
     */
    void ChangeSection(int nIndex, int nBar);

    /**
     * Start the intro of the display and schedule OnTransition() half a bar later.
     *
     * @ghidraAddress NTSC-U/C: 0x00133998
     * @ghidraAddress PAL: 0x001351a8
     */
    void StartTransition();

    /**
     * Clear the remix tracks up to mLoopBar and schedule the section's tracks and menus there.
     *
     * mTransitionCmd runs it.
     *
     * @ghidraAddress NTSC-U/C: 0x00133a28
     * @ghidraAddress PAL: 0x00135238
     */
    void OnTransition();

    /**
     * Give the vocal tracks the current section, and outside a preview silence the remix tracks
     * and apply the effects.
     *
     * mSectionStartCmd runs it.
     *
     * @ghidraAddress NTSC-U/C: 0x00133c18
     * @ghidraAddress PAL: 0x00135428
     */
    void ApplySectionTracks();

    /**
     * Copy the gems of each section's pattern into the remix tracks, and the effects of each
     * pattern's set into the section's set.
     *
     * @ghidraAddress NTSC-U/C: 0x00133d48
     * @ghidraAddress PAL: 0x00135558
     */
    void RebuildPatterns();

    /**
     * Follow the section the song position plays during a preview, once a section.
     *
     * mSectionEndCmd runs it. It adds the checkpoints again once the song wraps.
     *
     * @ghidraAddress NTSC-U/C: 0x00133e38
     * @ghidraAddress PAL: 0x00135648
     */
    void OnSectionEnd();

    /**
     * Apply the effect set of the current section.
     *
     * mApplyEffectsCmd runs it.
     *
     * @ghidraAddress NTSC-U/C: 0x00134020
     * @ghidraAddress PAL: 0x00135830
     */
    void ApplySectionEffects();

    /**
     * Add a checkpoint labelled with its section for each section of the song.
     *
     * @param nFirstBar The bar the song starts at.
     * @ghidraAddress NTSC-U/C: 0x00134058
     * @ghidraAddress PAL: 0x00135868
     */
    void AddSectionCheckpoints(int nFirstBar);

    /**
     * Start a preview of the whole remix.
     *
     * @param nBar The bar the preview starts at, or -1 for the bar after the song position.
     * @param bTransition Whether the song fades into the preview through a transition.
     * @ghidraAddress NTSC-U/C: 0x001341c8
     * @ghidraAddress PAL: 0x001359d8
     */
    void StartPreview(int nBar, bool bTransition);

    /**
     * Play the intro of the song with every remix track active at full volume.
     *
     * mPlayIntroCmd runs it, and it schedules EndIntro() for the end of the intro.
     *
     * @ghidraAddress NTSC-U/C: 0x00134648
     * @ghidraAddress PAL: 0x00135e58
     */
    void PlayIntro();

    /**
     * Silence the remix tracks, restore the volume of each track, and apply the effects.
     *
     * mEndIntroCmd runs it.
     *
     * @ghidraAddress NTSC-U/C: 0x00134790
     * @ghidraAddress PAL: 0x00135fa0
     */
    void EndIntro();

    /**
     * Stop a preview and return to the current section.
     *
     * @param nBar The bar the section plays from, or -1 for the next bar QueueSectionChange()
     *        picks.
     * @ghidraAddress NTSC-U/C: 0x00134860
     * @ghidraAddress PAL: 0x00136070
     */
    void StopPreview(int nBar);

    /**
     * Loop a range of bars from a bar on and restart the background music.
     *
     * @param nStartBar The first bar of the loop.
     * @param nEndBar The bar after the loop.
     * @param nPlayBar The bar the loop plays from.
     * @param nLoopBar The new mLoopBar.
     * @ghidraAddress NTSC-U/C: 0x00134b50
     * @ghidraAddress PAL: 0x00136360
     */
    void SetLoop(int nStartBar, int nEndBar, int nPlayBar, int nLoopBar);

    /**
     * Report the tempo at the speed of the song clock.
     *
     * @return The tempo in beats per minute.
     * @ghidraAddress NTSC-U/C: 0x00134bb8
     * @ghidraAddress PAL: 0x001363c8
     */
    float GetTempo();

    /**
     * Change the tempo when the new speed lies between 0.8 and 1.2.
     *
     * @param fDelta The change in beats per minute.
     * @ghidraAddress NTSC-U/C: 0x00134bf0
     * @ghidraAddress PAL: 0x00136400
     */
    void AdjustTempo(float fDelta);

    /**
     * Switch the vocals of the current section of a track on or off.
     *
     * @param nTrack The track's index in the song.
     * @param bMute Mute the vocals.
     * @ghidraAddress NTSC-U/C: 0x00134d50
     * @ghidraAddress PAL: 0x00136560
     */
    void MuteVocals(int nTrack, bool bMute);

    /**
     * Set whether remote players silence every remix track.
     *
     * @param nMuteRemote Non-zero to silence.
     * @ghidraAddress NTSC-U/C: 0x00134db8
     * @ghidraAddress PAL: 0x001365c8
     */
    void SetMuteRemote(int nMuteRemote);

    /**
     * Switch the filter of a track in the current effect set.
     *
     * @param nTrack The track's index in the song.
     * @param bOn Switch the filter on.
     * @ghidraAddress NTSC-U/C: 0x00134e70
     * @ghidraAddress PAL: 0x00136680
     */
    void SetFilter(int nTrack, bool bOn);

    /**
     * Switch the stutter of a track in the current effect set.
     *
     * @param nTrack The track's index in the song.
     * @param bOn Switch the stutter on.
     * @ghidraAddress NTSC-U/C: 0x00134ed8
     * @ghidraAddress PAL: 0x001366e8
     */
    void SetStutter(int nTrack, bool bOn);

    /**
     * Switch the chorus of a track in the current effect set.
     *
     * @param nTrack The track's index in the song.
     * @param bOn Switch the chorus on.
     * @ghidraAddress NTSC-U/C: 0x00134f40
     * @ghidraAddress PAL: 0x00136750
     */
    void SetChorus(int nTrack, bool bOn);

    /**
     * Read the saved remix the game database stores, if any.
     *
     * @ghidraAddress NTSC-U/C: 0x00134fa8
     * @ghidraAddress PAL: 0x001367b8
     */
    void LoadRemix();

    /**
     * Store the remix in the game database, with its rating, packed.
     *
     * @ghidraAddress NTSC-U/C: 0x00135090
     * @ghidraAddress PAL: 0x001368a0
     */
    void SaveRemix();

    /**
     * Write the edits of the remix: a version byte, the speed, the effects, and the remix tracks.
     *
     * @param stream The stream.
     * @ghidraAddress NTSC-U/C: 0x00135320
     * @ghidraAddress PAL: 0x00136b30
     */
    void Save(BinStream &stream);

    /**
     * Read the edits of the remix that Save() wrote, and set the song speed.
     *
     * @param stream The stream.
     * @ghidraAddress NTSC-U/C: 0x00135418
     * @ghidraAddress PAL: 0x00136c20
     */
    void Load(BinStream &stream);

    /**
     * Report whether a tick plays the current section outside a preview.
     *
     * @param nTick The tick.
     * @return Whether the section at the tick is the current one.
     * @ghidraAddress NTSC-U/C: 0x00135670
     * @ghidraAddress PAL: 0x00136e70
     */
    bool IsTickInCurrentSection(int nTick);

    /**
     * Change the section the way the host reported it.
     *
     * @param pPacket The packet.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x001356e8
     * @ghidraAddress PAL: 0x00136ee8
     */
    int OnSectionChange(SectionChangePacket *pPacket);

    /**
     * Switch an effect the way another console reported it.
     *
     * @param pPacket The packet.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x00135768
     * @ghidraAddress PAL: 0x00136f68
     */
    int OnFX(FXPacket *pPacket);

    /**
     * Switch the vocals of a section the way another console reported it.
     *
     * @param pPacket The packet.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x00135830
     * @ghidraAddress PAL: 0x00137030
     */
    int OnMuteVocals(MuteVocalsPacket *pPacket);

    /**
     * Start or stop a preview the way the host reported it.
     *
     * @param pPacket The packet.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x001358a0
     * @ghidraAddress PAL: 0x001370a0
     */
    int OnPreview(PreviewPacket *pPacket);

    /**
     * Toggle a gem another console placed in the current section.
     *
     * @param pPacket The packet.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x001358e0
     * @ghidraAddress PAL: 0x001370e0
     */
    int OnEditGem(EditGemPacket *pPacket);

    /**
     * Erase a bar another console erased.
     *
     * @param pPacket The packet.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x00135968
     * @ghidraAddress PAL: 0x00137168
     */
    int OnEraseBar(EraseBarPacket *pPacket);

    /**
     * Erase a section another console erased.
     *
     * @param pPacket The packet.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x001359b8
     * @ghidraAddress PAL: 0x001371b8
     */
    int OnEraseSection(EraseSectionPacket *pPacket);

    /**
     * Take the track and the looping of a remote player from a newer update.
     *
     * @param pPacket The packet.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x00135a08
     * @ghidraAddress PAL: 0x00137208
     */
    int OnRemixerUpdate(RemixerUpdatePacket *pPacket);

    /**
     * Take the assignment of the players to the tracks the host settled.
     *
     * @param pPacket The packet.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x00135af0
     * @ghidraAddress PAL: 0x001372f0
     */
    int OnArbiter(ArbiterPacket *pPacket);

    /**
     * Take a player who quit the session out of the remix.
     *
     * @param pMsg The message.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x00135cc0
     * @ghidraAddress PAL: 0x001374c0
     */
    int OnPlayerAborted(PlayerAbortedMsg *pMsg);

    /**
     * Show why the session ended and end the remix 3.5 seconds later.
     *
     * @param pMsg The message.
     * @return 0.
     * @ghidraAddress NTSC-U/C: 0x00135db8
     * @ghidraAddress PAL: 0x001375b8
     */
    int OnGameEnded(GameEndedMsg *pMsg);

    /**
     * Pass a choice of a remix menu to HandleHudEvent().
     *
     * @param nPlayer The player whose menu reported the choice.
     * @param nEvent One of RemixHUD::Event.
     * @param pData The logic.
     * @ghidraAddress NTSC-U/C: 0x00135f38
     * @ghidraAddress PAL: 0x00137738
     */
    static void OnHudEvent(int nPlayer, int nEvent, void *pData);

    /**
     * Act on a choice of a player's remix menu.
     *
     * @param nPlayer The player index.
     * @param nEvent One of RemixHUD::Event.
     * @ghidraAddress NTSC-U/C: 0x00135f68
     * @ghidraAddress PAL: 0x00137768
     */
    void HandleHudEvent(int nPlayer, int nEvent);

    /**
     * Ignore a key of the keyboard. The body returns zero.
     *
     * The PAL body passes the message to WorldLogic::OnKeyboardKey() and returns 1.
     *
     * @param pMsg The message.
     * @return 0, or 1 in the PAL build.
     * @ghidraAddress NTSC-U/C: 0x00136388
     * @ghidraAddress PAL: 0x00137b88
     */
    int OnKeyboardKey(KeyboardKeyMsg *pMsg);

    Song *mSong;                  /*!< The song. Borrowed. */
    int mTicksPerBar;             /*!< The song ticks in one bar. */
    int mNumBars;                 /*!< The length of the song in bars. */
    int mState;                   /*!< One of State. */
    int mSavedState;              /*!< The state SetPaused() restores. */
    int mPaused;                  /*!< Whether SetPaused() paused the song. */
    PlayMap *mPlayMap;            /*!< The map of the song positions. Borrowed. */
    SectionBoundaries *mSections; /*!< The sections of the song. Borrowed. */
    std::vector<std::unique_ptr<RemixTrack>> mRemixTracks; /*!< The pitch and vocal tracks. */
    std::vector<PitchTrack *> mPitchTracks;                /*!< The pitch tracks. Borrowed. */
    std::vector<VoxTrack *> mVoxTracks;                    /*!< The vocal tracks. Borrowed. */
    std::unique_ptr<FreestyleTrack> mFreestyleTrack; /*!< The guitar or turntable track, or null. */
    std::vector<std::unique_ptr<Player>> mPlayers;   /*!< Every player, by index. */
    std::unique_ptr<GameTrackSelector> mTrackSelector; /*!< The assignment of the players. */
    WorldBeat mWorldBeat;                         /*!< The events of the track named "WORLD". */
    std::vector<int> mPlayableSections;           /*!< The sections no pattern fills, ascending. */
    int mSectionIndex;                            /*!< The index of the current section. */
    int mLoopBar;                                 /*!< The bar the current loop plays from. */
    int mMuteRemote;                              /*!< Whether remote players silence the tracks. */
    int mPreviewing;                              /*!< Whether a preview plays. */
    int mQuit;                                    /*!< Whether a player quit from a menu. */
    int mModified;                                /*!< Whether the remix was edited. */
    int mReadOnly;                                /*!< Whether the remix may not be edited. */
    float mBaseTempo;                             /*!< The tempo at normal speed, in BPM. */
    std::vector<int> mNetOrderPlayers;            /*!< The player of each session order. */
    std::unique_ptr<NetArbiter> mNetArbiter;      /*!< The arbitration of the host, or null. */
    unsigned int mArbiterVersion;                 /*!< The newest assignment taken. */
    std::vector<unsigned int> mUpdateVersions;    /*!< The newest update of each player. */
    std::vector<int> mPlayerTracks;               /*!< The track the host assigned each player. */
    std::vector<int> mTrackVolumes;               /*!< The volume of each track after the intro. */
    std::vector<std::unique_ptr<RemixHUD>> mHuds; /*!< The remix menu of each controller. */
    FXMgr mFx;                                    /*!< The effect sets of the sections. */
    Ptr<Command> mTransitionCmd;                  /*!< Runs OnTransition(). */
    Ptr<Command> mSectionStartCmd;                /*!< Runs ApplySectionTracks(). */
    Ptr<Command> mUpdateHudsCmd;                  /*!< Runs UpdateHuds(). */
    Ptr<Command> mSectionEndCmd;                  /*!< Runs OnSectionEnd(). */
    Ptr<Command> mApplyEffectsCmd;                /*!< Runs ApplySectionEffects(). */
    Ptr<Command> mPlayIntroCmd;                   /*!< Runs PlayIntro(). */
    Ptr<Command> mEndIntroCmd;                    /*!< Runs EndIntro(). */
    std::unique_ptr<RemixTutorial> mTutorial;     /*!< The tutorial the remix plays, or null. */
    int mSavedMuteRemote;                         /*!< mMuteRemote before the preview. */
    int mEndTick;                                 /*!< The tick the faded song ends at. */
    int mRemixLoaded;                             /*!< Whether LoadRemix() read a saved remix. */
};

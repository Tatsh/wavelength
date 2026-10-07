#pragma once

#include <vector>

#include "game/pitchtrackdisplay.h"
#include "game/pitchtrackgems.h"
#include "game/pitchtrackmusic.h"
#include "game/pitchtrackplaygems.h"
#include "game/pitchtrackriffdata.h"
#include "game/pitchtrackriffs.h"
#include "game/playmap.h"
#include "game/remixtrack.h"
#include "game/sectionboundaries.h"
#include "game/slotgrid.h"
#include "os/binstream.h"
#include "os/command.h"
#include "os/ptr.h"

/**
 * Track of a remixed song whose gems players place, each gem playing the riff of its gem button.
 *
 * The RTTI records the class as deriving from RemixTrack. A player edits the bars of the phrase
 * the track shows, two bars from an even bar of the section on, and the first player to place a
 * gem in a section owns the section until its last gem goes. The names of the members other than
 * the Track overrides are inferred.
 */
class PitchTrack : public RemixTrack {
public:
    /**
     * Construct the track for one song track.
     *
     * @param nTrack The index of the song track.
     * @param nLeadInBars The bars the song plays before bar 0.
     * @param nNumBars The length of the song in bars.
     * @param nTicksPerBar The song ticks in one bar.
     * @param pRiffData The riffs of the song track.
     * @param pPatterns The patterns of gems the track can select.
     * @param pPlayMap The map of the song positions.
     * @param pSections The sections of the song.
     * @param pPatternGrid The pattern each section repeats, or null.
     * @param bDisplayed Whether the track shows its bars and gems on the play field.
     * @ghidraAddress NTSC-U/C: 0x00128880
     * @ghidraAddress PAL: 0x0012a078
     */
    PitchTrack(int nTrack,
               int nLeadInBars,
               int nNumBars,
               int nTicksPerBar,
               PitchTrackRiffData *pRiffData,
               const std::vector<PitchTrackGems *> *pPatterns,
               PlayMap *pPlayMap,
               SectionBoundaries *pSections,
               SlotGrid *pPatternGrid,
               bool bDisplayed);

    /**
     * Release the track.
     *
     * @ghidraAddress NTSC-U/C: 0x00128ba0
     * @ghidraAddress PAL: 0x0012a398
     */
    ~PitchTrack() override;

    /**
     * Start the track with the song.
     *
     * @ghidraAddress NTSC-U/C: 0x00128d40
     * @ghidraAddress PAL: 0x0012a538
     */
    void Start() override;

    /**
     * Stop the track with the song.
     *
     * @ghidraAddress NTSC-U/C: 0x00128d88
     * @ghidraAddress PAL: 0x0012a580
     */
    void Stop() override;

    /**
     * Give the track to the player it plays for.
     *
     * A track a player departs hides its phrase and shows its own gems again.
     *
     * @param pPlayer The player, or null.
     * @ghidraAddress NTSC-U/C: 0x00128cb0
     * @ghidraAddress PAL: 0x0012a4a8
     */
    void SetPlayer(Player *pPlayer) override;

    /**
     * Place or remove the gem of a pressed gem button at the nearest sixteenth of a bar.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x001290f8
     * @ghidraAddress PAL: 0x0012a8f0
     */
    void HandleInput(Player *pPlayer, const PlayNoteEvent &event) override;

    /**
     * Erase the bar at the nearest sixteenth of a bar, or its section on a second press within
     * half a second.
     *
     * @param pPlayer The player.
     * @param event The event. It is unused.
     * @ghidraAddress NTSC-U/C: 0x00129330
     * @ghidraAddress PAL: 0x0012ab28
     */
    void HandleInput(Player *pPlayer, const BtnEvent<8> &event) override;

    /**
     * Ignore a stick event. The body is empty.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x0033c9d8
     * @ghidraAddress PAL: 0x003a9f10
     */
    void HandleInput([[maybe_unused]] Player *pPlayer,
                     [[maybe_unused]] const StickEvent<2> &event) override {
    }

    /**
     * Ignore a stick event. The body is empty.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x0033c9e0
     * @ghidraAddress PAL: 0x003a9f18
     */
    void HandleInput([[maybe_unused]] Player *pPlayer,
                     [[maybe_unused]] const StickEvent<6> &event) override {
    }

    /**
     * Ignore a button event. The body is empty.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x0033c9e8
     * @ghidraAddress PAL: 0x003a9f20
     */
    void HandleInput([[maybe_unused]] Player *pPlayer,
                     [[maybe_unused]] const BtnEvent<10> &event) override {
    }

    /**
     * Set whether the track is silent while a player on another console is on it.
     *
     * @param bMuteRemote Whether the track is silent.
     * @ghidraAddress NTSC-U/C: 0x00128ef0
     * @ghidraAddress PAL: 0x0012a6e8
     */
    void SetMuteRemote(bool bMuteRemote) override;

    /**
     * Report whether the track is silent while a player on another console is on it.
     *
     * @return mMuteRemote.
     * @ghidraAddress NTSC-U/C: 0x00128f10
     * @ghidraAddress PAL: 0x0012a708
     */
    bool GetMuteRemote() const override;

    /**
     * Show a range of bars empty, and start the phrases at the end of the range.
     *
     * @param nStartBar The first bar of the range.
     * @param nEndBar The bar after the range.
     * @ghidraAddress NTSC-U/C: 0x00128f40
     * @ghidraAddress PAL: 0x0012a738
     */
    void SkipBars(int nStartBar, int nEndBar) override;

    /**
     * Copy the gems of each section's pattern into the section.
     *
     * @ghidraAddress NTSC-U/C: 0x00128ed0
     * @ghidraAddress PAL: 0x0012a6c8
     */
    void RebuildGems() override;

    /**
     * Set whether the track is the one the song's player plays, which silences its own riffs.
     *
     * @param bActive Whether the track is active.
     * @ghidraAddress NTSC-U/C: 0x00128f18
     * @ghidraAddress PAL: 0x0012a710
     */
    void SetActive(bool bActive) override;

    /**
     * Report whether the track is the one the song's player plays.
     *
     * @return mActive.
     * @ghidraAddress NTSC-U/C: 0x00128f38
     * @ghidraAddress PAL: 0x0012a730
     */
    bool IsActive() const override;

    /**
     * Show the gems again from a bar on.
     *
     * @param nStartBar The first bar.
     * @ghidraAddress NTSC-U/C: 0x00128f90
     * @ghidraAddress PAL: 0x0012a788
     */
    void Redraw(int nStartBar) override;

    /**
     * Write the gems.
     *
     * @param stream The stream.
     * @ghidraAddress NTSC-U/C: 0x001295f8
     * @ghidraAddress PAL: 0x0012adf0
     */
    void Save(BinStream &stream) override;

    /**
     * Read the gems and show them again from the current bar on.
     *
     * @param stream The stream.
     * @ghidraAddress NTSC-U/C: 0x00129618
     * @ghidraAddress PAL: 0x0012ae10
     */
    void Load(BinStream &stream) override;

    /**
     * Copy the gems of a pattern over a range of bars, and play them.
     *
     * @param nPattern The pattern.
     * @param nStartBar The first bar of the range.
     * @param nEndBar The bar after the range.
     * @ghidraAddress NTSC-U/C: 0x00128e38
     * @ghidraAddress PAL: 0x0012a630
     */
    void SelectPattern(int nPattern, int nStartBar, int nEndBar);

    /**
     * Set whether gem edits repeat on every other bar of the section.
     *
     * @param bRepeat Whether edits repeat.
     * @ghidraAddress NTSC-U/C: 0x00128e88
     * @ghidraAddress PAL: 0x0012a680
     */
    void SetRepeat(bool bRepeat);

    /**
     * Set whether the remix editor is open, which hides the phrase from the player.
     *
     * @param bEditing Whether the editor is open.
     * @ghidraAddress NTSC-U/C: 0x00128ea8
     * @ghidraAddress PAL: 0x0012a6a0
     */
    void SetEditing(bool bEditing);

    /**
     * Place or remove the gem at a tick, and show the change.
     *
     * @param nSlot The gem button.
     * @param nTick The tick.
     * @param bRepeat Whether the edit repeats on every other bar of the section.
     * @param nOwner The player who edits the gem.
     * @return Whether a gem lies at the tick afterwards.
     * @ghidraAddress NTSC-U/C: 0x00128fb8
     * @ghidraAddress PAL: 0x0012a7b0
     */
    bool ToggleGem(signed char nSlot, int nTick, bool bRepeat, signed char nOwner);

    /**
     * Remove the gems of a bar, and of every other bar of the section while edits repeat.
     *
     * @param nBar The bar.
     * @ghidraAddress NTSC-U/C: 0x00129060
     * @ghidraAddress PAL: 0x0012a858
     */
    void EraseBar(int nBar);

    /**
     * Remove the gems of the section a bar lies in.
     *
     * @param nBar The bar.
     * @ghidraAddress NTSC-U/C: 0x001290b0
     * @ghidraAddress PAL: 0x0012a8a8
     */
    void EraseSection(int nBar);

private:
    /**
     * Show the gems of the track again in place of a previewed pattern.
     *
     * @ghidraAddress NTSC-U/C: 0x00128dd0
     * @ghidraAddress PAL: 0x0012a5c8
     */
    void EndPreview();

    /**
     * Show the current phrase to the player on the track, if any.
     *
     * @param bStyle The style of the display.
     * @ghidraAddress NTSC-U/C: 0x00129670
     * @ghidraAddress PAL: 0x0012ae68
     */
    void UpdatePhrase(bool bStyle);

    /**
     * Show the current phrase to the player on the track, and schedule the next phrase.
     *
     * @param bPlayable Whether the player edits the phrase.
     * @param bStyle The style of the display.
     * @ghidraAddress NTSC-U/C: 0x001296b0
     * @ghidraAddress PAL: 0x0012aea8
     */
    void ShowPhrase(bool bPlayable, bool bStyle);

    /**
     * Silence the riffs while the track is active, or while a player on another console or no
     * player is on it and such players silence it.
     *
     * @ghidraAddress NTSC-U/C: 0x00129898
     * @ghidraAddress PAL: 0x0012b090
     */
    void UpdateMute();

    int mTicksPerBar;                            /*!< The song ticks in one bar. */
    PlayMap *mPlayMap;                           /*!< The map of the song positions. */
    SectionBoundaries *mSections;                /*!< The sections of the song. */
    PitchTrackGems mGems;                        /*!< The gems the players placed. */
    PitchTrackPlayGems mPlayGems;                /*!< mGems by the bars the song plays. */
    std::vector<PitchTrackPlayGems *> mPatterns; /*!< The patterns the track can select. */
    PitchTrackRiffs mRiffs;                      /*!< The riffs of the gem buttons. */
    PitchTrackMusic mMusic;                      /*!< The player of the gems' riffs. */
    PitchTrackDisplay *mDisplay;                 /*!< The display of the gems, or null. */
    bool mRepeat;                                /*!< Whether edits repeat in the section. */
    bool mEditing;                               /*!< Whether the remix editor is open. */
    bool mMuteRemote;                            /*!< Whether remote players silence it. */
    bool mActive;                                /*!< Whether the song's player plays it. */
    float mLastEraseMs;                          /*!< The system time of the last erase. */
    int mStartBar;                               /*!< The first bar a phrase can start at. */
    int mPreviewPattern;                         // +0xb4, -1. No routine sets another value.
    Ptr<Command> mPhraseCommand;                 /*!< The command that calls UpdatePhrase(). */
};

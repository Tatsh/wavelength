#pragma once

#include "game/catchtrackdata.h"
#include "game/catchtrackstate.h"
#include "game/duelpatterntable.h"
#include "game/dueltrackpitcher.h"
#include "game/gemcatcher.h"
#include "game/gemcursor.h"
#include "game/netfaker.h"
#include "game/pitchtrackriffdata.h"
#include "game/player.h"
#include "game/playmap.h"
#include "game/sectionboundaries.h"
#include "game/track.h"
#include "game/trackreactor.h"
#include "gs/muse.h"
#include "os/command.h"
#include "os/ptr.h"

// DuelLogic creates and deletes its DuelTrack objects, and each track reports to its DuelLogic.
class DuelLogic;

/**
 * Track of one player in a duel, on which the player places a phrase or catches the phrase the
 * other player placed.
 *
 * The RTTI records the class as deriving from Track and TrackReactor. The object is 0x10c bytes.
 * The presses of a phrase to place go to a DuelTrackPitcher, and the presses of a phrase to
 * catch go to a GemCatcher. Each reports to the track through a nested receiver. The names of the
 * members other than the constructor, the destructor, and the Track and TrackReactor overrides are
 * inferred.
 */
class DuelTrack : public Track, public TrackReactor {
public:
    /** The values of SetMode() and ShowPhrase(). */
    enum Mode {
        kModeIdle = 0,  /*!< The track shows no phrase. */
        kModePitch = 1, /*!< The player places a phrase. */
        kModeCatch = 2, /*!< The player catches the phrase the other player placed. */
    };

    /**
     * Construct the track of one player.
     *
     * @param pLogic The rules the track reports to.
     * @param pPlayer The player.
     * @param pCatchGems The gems the other player places for the player to catch.
     * @param pPitchGems The gems the player places for the other player to catch.
     * @param pRiffData The riffs of the song track the duel plays first.
     * @param pPatterns The duel patterns of the song.
     * @param pTickDuration The duration of one tick on the song clock.
     * @param pPlayMap The map of the song positions.
     * @param pSections The sections of the song. It is unused.
     * @param nSide The side of the player.
     * @param nOpponentSide The side of the other player.
     * @param nLeadInBars One more than the bars the song plays before bar 0. It is unused.
     * @param nNumBars The length of the song in bars.
     * @param nTicksPerBar The song ticks in one bar.
     * @param nPhraseBars The length of a phrase in bars.
     * @param bEasiest Whether the song is at the easiest difficulty, at which the duel coaches the
     *                 players.
     * @ghidraAddress NTSC-U/C: 0x0010ab10
     * @ghidraAddress PAL: 0x0010c248
     */
    DuelTrack(DuelLogic *pLogic,
              Player *pPlayer,
              CatchTrackData *pCatchGems,
              CatchTrackData *pPitchGems,
              PitchTrackRiffData *pRiffData,
              DuelPatternTable *pPatterns,
              const float *pTickDuration,
              PlayMap *pPlayMap,
              SectionBoundaries *pSections,
              int nSide,
              int nOpponentSide,
              int nLeadInBars,
              int nNumBars,
              int nTicksPerBar,
              int nPhraseBars,
              bool bEasiest);

    /**
     * Release the track.
     *
     * @ghidraAddress NTSC-U/C: 0x0010adc0
     * @ghidraAddress PAL: 0x0010c4f8
     */
    ~DuelTrack() override;

    /**
     * Start the track with the song.
     *
     * @ghidraAddress NTSC-U/C: 0x0010aef8
     * @ghidraAddress PAL: 0x0010c630
     */
    void Start() override;

    /**
     * Stop the track with the song.
     *
     * @ghidraAddress NTSC-U/C: 0x0010af58
     * @ghidraAddress PAL: 0x0010c690
     */
    void Stop() override;

    /**
     * Hand a press of the player on the track to the pitcher or to the catcher.
     *
     * A press while the phrase is already decided plays the refusal sound.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x0010b470
     * @ghidraAddress PAL: 0x0010cba8
     */
    void HandleInput(Player *pPlayer, const PlayNoteEvent &event) override;

    /**
     * Ignore a button event. The body is empty.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x003352e8
     */
    void HandleInput([[maybe_unused]] Player *pPlayer,
                     [[maybe_unused]] const BtnEvent<8> &event) override {
    }

    /**
     * Ignore a stick event. The body is empty.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x0010b500
     */
    void HandleInput(Player *pPlayer, const StickEvent<2> &event) override;

    /**
     * Ignore a stick event. The body is empty.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x0010b508
     */
    void HandleInput(Player *pPlayer, const StickEvent<6> &event) override;

    /**
     * Ignore a button event. The body is empty.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x003352f0
     */
    void HandleInput([[maybe_unused]] Player *pPlayer,
                     [[maybe_unused]] const BtnEvent<10> &event) override {
    }

    /**
     * Report the song ticks in one bar.
     *
     * @return The ticks.
     * @ghidraAddress NTSC-U/C: 0x003352f8
     */
    int GetTicksPerBar() const override {
        return mTicksPerBar;
    }

    /**
     * Report the player of the side the track belongs to.
     *
     * @return The player.
     * @ghidraAddress NTSC-U/C: 0x00335300
     */
    Player *GetPlayer() const override {
        return mSidePlayer;
    }

    /**
     * Report that the phrase of every bar can be played.
     *
     * @param nBar The bar.
     * @return True.
     * @ghidraAddress NTSC-U/C: 0x0010bdc0
     */
    bool IsBarActive(int nBar) override;

    /**
     * Play the riff of a caught gem and show the catch.
     *
     * The gem counts towards the catch when it lies in the phrase to catch, and catching the last
     * gem of the phrase ends the catch as made.
     *
     * @param nTick The song tick of the press.
     * @param cursor The gem.
     * @param bRemote Whether the catch came from another console.
     * @ghidraAddress NTSC-U/C: 0x0010b5a0
     * @ghidraAddress PAL: 0x0010ccd8
     */
    void HitGem(int nTick, const GemCursor &cursor, bool bRemote) override;

    /**
     * End the catch as missed when a gem of the phrase to catch passes without a press.
     *
     * Only a player on this console misses a gem this way.
     *
     * @param nTick The song tick the gem was judged at.
     * @param cursor The gem.
     * @param bRemote Whether the miss came from another console.
     * @ghidraAddress NTSC-U/C: 0x0010b7c8
     * @ghidraAddress PAL: 0x0010cf00
     */
    void MissGem(int nTick, const GemCursor &cursor, bool bRemote) override;

    /**
     * Report a cursor at the first gem the player catches.
     *
     * @return The cursor.
     * @ghidraAddress NTSC-U/C: 0x0010bd90
     * @ghidraAddress PAL: 0x0010d4c8
     */
    GemCursor GetCursor() override;

    /**
     * Show the bars of the next phrase.
     *
     * @param nMode One of Mode. kModePitch also shows the steps the duel pattern allows.
     * @param nStartBar The first bar of the phrase.
     * @param nEndBar The bar after the phrase.
     * @ghidraAddress NTSC-U/C: 0x0010b088
     * @ghidraAddress PAL: 0x0010c7c0
     */
    void ShowPhrase(int nMode, int nStartBar, int nEndBar);

    /**
     * Change what the player does on the track.
     *
     * Catching a phrase with no gems ends the catch as made at once.
     *
     * @param nMode One of Mode.
     * @param nBar The bar the change takes effect at.
     * @ghidraAddress NTSC-U/C: 0x0010b0e0
     * @ghidraAddress PAL: 0x0010c818
     */
    void SetMode(int nMode, int nBar);

    /**
     * Count one more gem in the phrase to catch.
     *
     * @ghidraAddress NTSC-U/C: 0x0010b3a0
     * @ghidraAddress PAL: 0x0010cad8
     */
    void CountGem();

    /**
     * Disable a range of bars the song has passed and remove the gems the range has.
     *
     * @param nStartBar The first bar of the range.
     * @param nEndBar The bar after the range.
     * @ghidraAddress NTSC-U/C: 0x0010b3b0
     * @ghidraAddress PAL: 0x0010cae8
     */
    void DropBars(int nStartBar, int nEndBar);

    /**
     * Move the playback of the track to the current song position.
     *
     * @ghidraAddress NTSC-U/C: 0x0010b3d0
     * @ghidraAddress PAL: 0x0010cb08
     */
    void Rewind();

    /**
     * Withdraw the command that runs once per phrase.
     *
     * @ghidraAddress NTSC-U/C: 0x0010b438
     * @ghidraAddress PAL: 0x0010cb70
     */
    void CancelPhrase();

    /**
     * End the current catch as missed.
     *
     * In an online duel, the miss is sent to the other console first.
     *
     * @ghidraAddress NTSC-U/C: 0x0010ba20
     * @ghidraAddress PAL: 0x0010d158
     */
    void MissCatch();

    /**
     * End the current catch as made.
     *
     * In an online duel, the catch is sent to the other console.
     *
     * @param nBar The bar the catch ended in.
     * @ghidraAddress NTSC-U/C: 0x0010bbb0
     * @ghidraAddress PAL: 0x0010d2e8
     */
    void MakeCatch(int nBar);

    /**
     * Change the riffs the gems the player places play.
     *
     * @param pRiffData The riffs.
     * @ghidraAddress NTSC-U/C: 0x0010bd70
     * @ghidraAddress PAL: 0x0010d4a8
     */
    void SetRiffData(PitchTrackRiffData *pRiffData);

private:
    /**
     * Receiver that hands the judgements of the GemCatcher to the track.
     *
     * The RTTI includes the class name.
     */
    class CatchReceiver : public GemCatcher::Receiver {
    public:
        /**
         * Construct a receiver for a track.
         *
         * @param pOwner The track.
         */
        explicit CatchReceiver(DuelTrack *pOwner) : mOwner(pOwner) {
        }

        /**
         * Hand a caught gem to the track.
         *
         * @param nTick The tick of the press.
         * @param cursor The gem.
         * @ghidraAddress NTSC-U/C: 0x00335388
         * @ghidraAddress PAL: 0x003a2938
         */
        void OnHit(int nTick, const GemCursor &cursor) override {
            mOwner->HitGem(nTick, cursor, false);
        }

        /**
         * Hand a passed gem to the track.
         *
         * @param nTick The tick the gem was judged at.
         * @param cursor The gem.
         * @ghidraAddress NTSC-U/C: 0x003353c0
         * @ghidraAddress PAL: 0x003a2970
         */
        void OnPass(int nTick, const GemCursor &cursor) override {
            mOwner->MissGem(nTick, cursor, false);
        }

        /**
         * Hand a press that caught no gem to the track.
         *
         * @param nTick The tick of the press.
         * @param nLane The lane.
         * @ghidraAddress NTSC-U/C: 0x003353f8
         * @ghidraAddress PAL: 0x003a29a8
         */
        void OnMiss(int nTick, int nLane) override {
            mOwner->OnPressMissed(nTick, nLane);
        }

    private:
        DuelTrack *mOwner; /*!< The track. */
    };

    /**
     * Receiver that hands the judgements of the DuelTrackPitcher to the track.
     *
     * The RTTI includes the class name.
     */
    class PitchReceiver : public DuelTrackPitcher::Receiver {
    public:
        /**
         * Construct a receiver for a track.
         *
         * @param pOwner The track.
         */
        explicit PitchReceiver(DuelTrack *pOwner) : mOwner(pOwner) {
        }

        /**
         * Hand a refused press to the track.
         *
         * @param nTick The tick of the step.
         * @param nSlot The gem button.
         * @ghidraAddress NTSC-U/C: 0x00335498
         * @ghidraAddress PAL: 0x003a2a48
         */
        void OnRejected(int nTick, [[maybe_unused]] int nSlot) override {
            mOwner->OnPressRejected(nTick);
        }

        /**
         * Hand a gem placed to the track.
         *
         * @param nTick The tick of the step the gem was placed at.
         * @ghidraAddress NTSC-U/C: 0x003354b8
         * @ghidraAddress PAL: 0x003a2a68
         */
        void OnPlaced(int nTick) override {
            mOwner->OnGemPlaced(nTick);
        }

    private:
        DuelTrack *mOwner; /*!< The track. */
    };

    /**
     * Show each bar of a range.
     *
     * @param nMode One of Mode.
     * @param nStartBar The first bar.
     * @param nEndBar The bar after the range.
     * @ghidraAddress NTSC-U/C: 0x0010af80
     * @ghidraAddress PAL: 0x0010c6b8
     */
    void ShowBars(int nMode, int nStartBar, int nEndBar);

    /**
     * Report whether a gem is the last of its bar.
     *
     * @param cursor The gem.
     * @return Whether no later gem lies in the bar of the gem.
     * @ghidraAddress NTSC-U/C: 0x0010b510
     * @ghidraAddress PAL: 0x0010cc48
     */
    bool IsLastGemOfBar(const GemCursor &cursor);

    /**
     * Show how much of the phrase to catch the player has caught.
     *
     * @ghidraAddress NTSC-U/C: 0x0010b768
     * @ghidraAddress PAL: 0x0010cea0
     */
    void ShowCatchProgress();

    /**
     * Show a press that caught no gem, and end the catch as missed when the phrase is over.
     *
     * @param nTick The tick of the press.
     * @param nLane The lane.
     * @ghidraAddress NTSC-U/C: 0x0010b838
     * @ghidraAddress PAL: 0x0010cf70
     */
    void OnPressMissed(int nTick, int nLane);

    /**
     * Play the refusal of a press on a step the duel pattern does not allow.
     *
     * @param nTick The tick of the step.
     * @ghidraAddress NTSC-U/C: 0x0010b8f8
     * @ghidraAddress PAL: 0x0010d030
     */
    void OnPressRejected(int nTick);

    /**
     * Add the points of a gem placed to the phrase.
     *
     * The first gem of the phrase rewinds the track of the catcher.
     *
     * @param nTick The tick of the step the gem was placed at.
     * @ghidraAddress NTSC-U/C: 0x0010b978
     * @ghidraAddress PAL: 0x0010d0b0
     */
    void OnGemPlaced(int nTick);

    /**
     * End the catch as missed once a tick lies in the phrase to catch or later.
     *
     * @param nTick The tick.
     * @ghidraAddress NTSC-U/C: 0x0010b9d8
     * @ghidraAddress PAL: 0x0010d110
     */
    void CheckCatchMissed(int nTick);

    /**
     * End the catch as made once every gem of the phrase is caught.
     *
     * @param cursor The gem caught last.
     * @param bRemote Whether the catch came from another console. A remote catch ends nothing.
     * @ghidraAddress NTSC-U/C: 0x0010bb38
     * @ghidraAddress PAL: 0x0010d270
     */
    void CheckCatchMade(const GemCursor &cursor, bool bRemote);

    /**
     * Start counting the gems of the next phrase placed, and run again a phrase later while the
     * song lasts.
     *
     * @ghidraAddress NTSC-U/C: 0x0010bcf0
     * @ghidraAddress PAL: 0x0010d428
     */
    void OnPhraseTick();

    Player *mSidePlayer;           /*!< The player of the side the track belongs to. */
    CatchReceiver *mCatchReceiver; /*!< The receiver of mGemCatcher. */
    PitchReceiver *mPitchReceiver; /*!< The receiver of mPitcher. */
    DuelPatternTable *mPatterns;   /*!< The duel patterns of the song. */
    PlayMap *mPlayMap;             /*!< The map of the song positions. */
    DuelLogic *mLogic;             /*!< The rules the track reports to. */
    int mNumBars;                  /*!< The length of the song in bars. */
    int mTicksPerBar;              /*!< The song ticks in one bar. */
    int mPhraseBars;               /*!< The length of a phrase in bars. */
    int mFirstPhraseTick;          /*!< The tick OnPhraseTick() first runs at. */
    CatchTrackState mCatchState;   /*!< The bars of the gems the player catches. */
    CatchTrackState mPitchState;   /*!< The bars of the gems the player places. */
    int mMode;                     /*!< One of Mode. */
    GemCatcher mGemCatcher;        /*!< The judge of the presses of a catch. */
    DuelTrackPitcher mPitcher;     /*!< The judge of the presses of a phrase placed. */
    int mCatchMissed;              /*!< Whether the current catch ended as missed. */
    int mCatchMade;                /*!< Whether the current catch ended as made. */
    int mCatchPoints;              /*!< The points of the phrase to catch. */
    Ptr<Command> mPhraseCommand;   /*!< The command that calls OnPhraseTick(). */
    int mCaughtGems;               /*!< The gems of the phrase to catch caught so far. */
    int mPhraseGems;               /*!< The gems of the phrase to catch. */
    int mCatchBar;                 /*!< The first bar of the phrase to catch, or -1. */
    int mCatchEndBar;              /*!< The bar after the phrase to catch, or -1. */
    int mRejectEndTick;            /*!< The tick the mark of the last refused press ends at. */
    int mPhraseBonus;              /*!< The points a phrase to catch adds to its gems. */
    int mPitchPoints;              /*!< The points of the gems placed in the phrase. */
    int mPitchedGems;              /*!< The gems placed in the phrase. */
    Muse *mMuse;                   /*!< The riff of the gem caught last, or null. */
    NetFaker *mNetFaker;           /*!< The stand-in for a remote player, online only. */
    int mIgnoreMisses;             /*!< Whether a missed gem or an empty phrase ends nothing. */
};

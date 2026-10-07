#pragma once

#include <vector>

#include "game/catchtrackstate.h"
#include "game/playmap.h"
#include "gs/multimuse.h"
#include "gs/muse.h"
#include "os/command.h"
#include "os/ptr.h"
#include "os/scheduler.h"

/**
 * Player of the music of the captured bars of one catch track.
 *
 * The RTTI includes the class name and records Muse::NoteCB as the base. Each bar of the song as
 * written has a MultiMuse with the music of the gems of the bar. A bar plays when the song arrives
 * at it while its phrase is captured, or always when `play_all_gems` is set. When the bars stop
 * their previous notes, the first note of a bar stops the bar that played before it.
 */
class CatchTrackMusic : public Muse::NoteCB {
public:
    /**
     * Command that plays the bar the song arrives at and runs again a bar later.
     *
     * The RTTI includes the nested name and records Command as the base. Its members are inline.
     */
    class PlayBarCmd : public Command {
    public:
        /**
         * Construct the command of a player.
         *
         * @param pMusic The player.
         */
        explicit PlayBarCmd(CatchTrackMusic *pMusic) : mMusic(pMusic) {
        }

        /**
         * Release the command.
         *
         * @ghidraAddress NTSC-U/C: 0x0034add8
         * @ghidraAddress PAL: 0x003b8208
         */
        ~PlayBarCmd() override {
        }

        /**
         * Play the bar the song arrives at, and run again a bar later.
         *
         * @ghidraAddress NTSC-U/C: 0x0034ae50
         * @ghidraAddress PAL: 0x003b8280
         */
        void Execute() override {
            mMusic->PlayBar();
            TheSongScheduler.PostIn(this, mMusic->mTicksPerBar, false);
        }

    private:
        CatchTrackMusic *mMusic; /*!< The player. */
    };

    /**
     * Construct the player and collect the music of the gems bar by bar.
     *
     * @param pState The state of the bars of the track.
     * @param nTicksPerBar The song ticks in one bar.
     * @param nNumBars The length of the song in bars.
     * @param pPlayMap The map of the song positions.
     * @param nStopPrevious Nonzero for each bar to stop its previous note, and the bar before it.
     * @ghidraAddress NTSC-U/C: 0x0014d7d0
     * @ghidraAddress PAL: 0x0014f170
     */
    CatchTrackMusic(CatchTrackState *pState,
                    int nTicksPerBar,
                    int nNumBars,
                    PlayMap *pPlayMap,
                    int nStopPrevious);

    /**
     * Stop the music and release the bars.
     *
     * @ghidraAddress NTSC-U/C: 0x0014d930
     * @ghidraAddress PAL: 0x0014f2d0
     */
    ~CatchTrackMusic() override;

    /**
     * Stop the music and schedule the bars from the next bar boundary on.
     *
     * @ghidraAddress NTSC-U/C: 0x0014da08
     * @ghidraAddress PAL: 0x0014f3a8
     */
    void Start();

    /**
     * Withdraw the scheduled command and stop every bar.
     *
     * @ghidraAddress NTSC-U/C: 0x0014daa0
     * @ghidraAddress PAL: 0x0014f440
     */
    void Stop();

    /**
     * Play the rest of the current bar once its phrase is captured, or stop it while the phrase
     * is not captured.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0014dbd0
     * @ghidraAddress PAL: 0x0014f570
     */
    void Refresh();

    /**
     * Stop the bar that played before the current one.
     *
     * @param nNote The MIDI note number, which is not read.
     * @param nDuration The length of the note in ticks, which is not read.
     * @ghidraAddress NTSC-U/C: 0x0014dfb8
     * @ghidraAddress PAL: 0x0014f958
     */
    void OnNote(unsigned char nNote, int nDuration) override;

private:
    /**
     * Fill the MultiMuse of each bar with the music of the gems in the bar.
     *
     * The name is inferred.
     *
     * @param nNumBars The length of the song in bars.
     * @ghidraAddress NTSC-U/C: 0x0014dce0
     * @ghidraAddress PAL: 0x0014f680
     */
    void BuildBars(int nNumBars);

    /**
     * Play the bar the song arrives at when its phrase is captured or every gem plays. Past the
     * end of the song, the bars repeat from the start.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0014de80
     * @ghidraAddress PAL: 0x0014f820
     */
    void PlayBar();

    CatchTrackState *mState;           /*!< The state of the bars of the track. */
    Ptr<Command> mPlayBarCommand;      /*!< The PlayBarCmd of the player. */
    std::vector<Ptr<MultiMuse>> mBars; /*!< The music of each bar of the song as written. */
    int mTicksPerBar;                  /*!< The song ticks in one bar. */
    PlayMap *mPlayMap;                 /*!< The map of the song positions. */
    int mStopPrevious;                 /*!< Whether a bar stops the bar before it. */
    Muse *mCurrent;                    /*!< The bar that plays, or null. */
    Muse *mPrevious;                   /*!< The bar that played before mCurrent, or null. */
};

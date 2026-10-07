#pragma once

#include "game/player.h"
#include "game/playmap.h"
#include "game/sectionboundaries.h"
#include "game/track.h"
#include "os/command.h"
#include "os/ptr.h"
#include "os/timer.h"

/**
 * Track the player plays freely with the analog sticks rather than by hitting gems.
 *
 * The RTTI records the class as deriving from Track. AxeTrack and ScratchTrack derive from it. In
 * a game the player earns pending points for the time an active note sounds in each bar, and the
 * points are committed at the end of the bar. Online, the button and the stick positions are sent
 * to the other consoles.
 */
class FreestyleTrack : public Track {
public:
    /**
     * Construct a track.
     *
     * @param pSections The section boundaries of the song.
     * @param pPlayMap The map of the song positions.
     * @param pfMsPerTick The length of a tick, in milliseconds.
     * @param nIndex The track's index in the song.
     * @param nNumBars The length of the song, in bars.
     * @param nTicksPerBar The length of a bar, in ticks.
     * @ghidraAddress NTSC-U/C: 0x0014ef80
     * @ghidraAddress PAL: 0x001508e0
     */
    FreestyleTrack(const SectionBoundaries *pSections,
                   PlayMap *pPlayMap,
                   const float *pfMsPerTick,
                   int nIndex,
                   int nNumBars,
                   int nTicksPerBar);

    /**
     * Release the track.
     *
     * @ghidraAddress NTSC-U/C: 0x0014f078
     * @ghidraAddress PAL: 0x001509d8
     */
    ~FreestyleTrack() override;

    /**
     * Stop the track with the song. The member does nothing in this class.
     *
     * @ghidraAddress NTSC-U/C: 0x0014f0e8
     * @ghidraAddress PAL: 0x00150a48
     */
    void Stop() override;

    /**
     * Commit the points of the player that leaves, assign the player, and start the bars of the
     * player that arrives.
     *
     * @param pPlayer The player, or null.
     * @ghidraAddress NTSC-U/C: 0x0014f0f0
     * @ghidraAddress PAL: 0x00150a50
     */
    void SetPlayer(Player *pPlayer) override;

    using Track::HandleInput;

    /**
     * Send a note the track's player played, and start or stop the scoring of the time it sounds.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x0014f258
     * @ghidraAddress PAL: 0x00150bb8
     */
    void HandleInput(Player *pPlayer, const PlayNoteEvent &event) override;

    /**
     * Ignore the event.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x0034b6d8
     * @ghidraAddress PAL: 0x003b8b08
     */
    void HandleInput([[maybe_unused]] Player *pPlayer,
                     [[maybe_unused]] const BtnEvent<8> &event) override {
    }

    /**
     * Pass a stick position of the track's player to the graphics and the other consoles.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x0014f330
     * @ghidraAddress PAL: 0x00150c90
     */
    void HandleInput(Player *pPlayer, const StickEvent<2> &event) override;

    /**
     * Pass a stick position of the track's player to the graphics.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x0014f3b8
     * @ghidraAddress PAL: 0x00150d18
     */
    void HandleInput(Player *pPlayer, const StickEvent<6> &event) override;

    /**
     * Report whether the track has an active note.
     *
     * @return True while a note sounds.
     */
    virtual bool IsActive() = 0;

    /** Rebuild what the track plays from the song position. */
    virtual void Refresh() = 0;

    /**
     * Wrap a tick into the repeat period of the track.
     *
     * @param nTick The tick.
     * @return The tick within the period.
     * @ghidraAddress NTSC-U/C: 0x0014f930
     * @ghidraAddress PAL: 0x00151290
     */
    int WrapTick(int nTick);

    /**
     * Report the tick at which the span that starts at a tick ends.
     *
     * @param nTick The tick.
     * @param nStart The first tick of the span.
     * @param nEnd The tick after the span, or -1 when the span has no end.
     * @return The end tick.
     * @ghidraAddress NTSC-U/C: 0x0014f980
     * @ghidraAddress PAL: 0x001512e0
     */
    int SpanEnd(int nTick, int nStart, int nEnd);

    /**
     * Award the player pending points for the time the active note sounded in the bar, and run
     * again a twentieth of a bar later.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x0014f400
     * @ghidraAddress PAL: 0x00150d60
     */
    void UpdatePoints();

    /**
     * Commit the pending points of the bar that ends and restart the timing of the active note.
     *
     * The name is inferred.
     *
     * @param bSchedule Whether to run again at the end of the next bar.
     * @ghidraAddress NTSC-U/C: 0x0014f610
     * @ghidraAddress PAL: 0x00150f70
     */
    void StartBar(bool bSchedule);

    /**
     * Send the button and a stick position of a local player to the other consoles, at most once
     * every 150 milliseconds.
     *
     * The name is inferred.
     *
     * @param pPlayer The player.
     * @param fX The horizontal stick position.
     * @param fY The vertical stick position.
     * @ghidraAddress NTSC-U/C: 0x0014f7c8
     * @ghidraAddress PAL: 0x00151128
     */
    void SendUpdate(Player *pPlayer, float fX, float fY);

    int mTickOffset;                    /*!< The offset of the game tick from the song tick. */
    const SectionBoundaries *mSections; /*!< The section boundaries of the song. */
    PlayMap *mPlayMap;                  /*!< The map of the song positions. */
    const float *mMsPerTick;            /*!< The length of a tick, in milliseconds. */
    int mNumBars;                       /*!< The length of the song, in bars. */
    int mTicksPerBar;                   /*!< The length of a bar, in ticks. */
    int mButton;                        /*!< The button of the last note the player played. */
    float mLastSendMs;                  /*!< The system time SendUpdate() last sent at. */
    Timer mNoteTimer;                   /*!< The time the active note sounded in the bar. */
    Ptr<Command> mBarCommand;           /*!< Calls StartBar() with true. */
    Ptr<Command> mPointsCommand;        /*!< Calls UpdatePoints(). */
};

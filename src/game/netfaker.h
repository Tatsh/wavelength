#pragma once

#include "game/gemcursor.h"
#include "game/trackreactor.h"
#include "os/command.h"
#include "os/memfuncommand.h"
#include "os/ptr.h"

/**
 * Player of the gems of a remote player's track, standing in for the remote player's input.
 *
 * The RTTI records the command the class schedules. The class is not polymorphic. DuelTrack and
 * ClickTrack build one over their gems. At each gem's tick the faker reports the gem as hit to
 * its TrackReactor, or as missed when the remote player has stopped catching within the bar of
 * the previous gem and the faker counts such misses.
 */
class NetFaker {
public:
    /**
     * Construct a faker at the first gem the reactor reports.
     *
     * Inline. The constructors of DuelTrack and ClickTrack expand it.
     *
     * @param pReactor The rules the gems are reported to.
     */
    explicit NetFaker(TrackReactor *pReactor)
        : mReactor(pReactor), mCursor(pReactor->GetCursor()), mLastBar(-1),
          mUpdateCommand(NewMemFunCommand(this, &NetFaker::Update)), mCountMisses(0) {
    }

    /**
     * Schedule the gem the cursor is at.
     *
     * @ghidraAddress NTSC-U/C: 0x00128350
     * @ghidraAddress PAL: 0x00129b48
     */
    void Start();

    /**
     * Withdraw the scheduled gem.
     *
     * @ghidraAddress NTSC-U/C: 0x001283c0
     * @ghidraAddress PAL: 0x00129bb8
     */
    void Stop();

    /**
     * Move to a gem and schedule it.
     *
     * @param cursor The gem.
     * @ghidraAddress NTSC-U/C: 0x001283f8
     * @ghidraAddress PAL: 0x00129bf0
     */
    void SetCursor(const GemCursor &cursor);

    /**
     * Report the gem of the current tick and schedule the next gem.
     *
     * A gem is reported only for a remote player who has not left the song, in a bar the reactor
     * reports active.
     *
     * @ghidraAddress NTSC-U/C: 0x00128498
     * @ghidraAddress PAL: 0x00129c90
     */
    void Update();

private:
    TrackReactor *mReactor;      /*!< The rules the gems are reported to. */
    GemCursor mCursor;           /*!< The next gem. */
    int mLastBar;                /*!< The bar of the last gem reported, or -1. */
    Ptr<Command> mUpdateCommand; /*!< The command that calls Update(). */
    int mCountMisses;            /*!< Whether a remote player not catching misses gems. */
};

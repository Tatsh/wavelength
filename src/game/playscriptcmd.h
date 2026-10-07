#pragma once

#include "game/scripttrackdata.h"
#include "os/command.h"
#include "os/scheduler.h"
#include "os/task.h"
#include "os/taskdonenotifier.h"
#include "script/scriptfunction.h"

/**
 * Command that runs the commands of a track named "SCRIPT" of the MIDI file, each at its tick.
 *
 * The RTTI includes the class name and records Command as the base. The command posts itself on
 * the song scheduler at the tick of each command in turn. Once the last command ran, the task
 * waiting on the track is told the track is done. TutorialGameLogic builds the command inline, and
 * the members are defined in the class.
 */
class PlayScriptCmd : public Command {
public:
    /**
     * Construct a command at the first command of a track.
     *
     * @param pTrack The commands.
     * @param pTickDuration The duration of one song tick. The command does not read it.
     */
    PlayScriptCmd(ScriptTrackData *pTrack, float *pTickDuration)
        : mTrack(pTrack), mTickDuration(pTickDuration), mNextCommand(0), mStartTick(0),
          mWaitingTask(nullptr) {
    }

    /**
     * Release the command.
     *
     * @ghidraAddress NTSC-U/C: 0x00343ce8
     * @ghidraAddress PAL: 0x003b1220
     */
    ~PlayScriptCmd() override {
    }

    /**
     * Run the next command of the track and post the one after it.
     *
     * @ghidraAddress NTSC-U/C: 0x00343d60
     * @ghidraAddress PAL: 0x003b1298
     */
    void Execute() override {
        // Yes, the binary discards the result.
        (void)ScriptFunction::Dispatch(mTrack->GetCommand(mNextCommand)->mValue);
        ++mNextCommand;
        ScheduleNext();
    }

    /**
     * Post the command at the tick of the next command of the track, or report the end of the
     * track to the waiting task.
     *
     * The name is inferred.
     *
     * @ghidraAddress NTSC-U/C: 0x00343da8
     * @ghidraAddress PAL: 0x003b12e0
     */
    void ScheduleNext() {
        if (mNextCommand < mTrack->NumCommands()) {
            TheSongScheduler.PostAt(
                this, mStartTick + mTrack->GetCommand(mNextCommand)->mPosition.mTick, false);
        } else if (mWaitingTask != nullptr) {
            TaskDoneNotifier::ReportDone(mWaitingTask);
        }
    }

    ScriptTrackData *mTrack; /*!< The commands. */
    float *mTickDuration;    /*!< The duration of one song tick, from Song::GetMsPerTick(). */
    int mNextCommand;        /*!< The index of the command that runs next. */
    int mStartTick;          /*!< The song tick the ticks of the commands count from. */
    Task *mWaitingTask;      /*!< The task waiting for the end of the track, or null. */
};

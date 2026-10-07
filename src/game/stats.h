#pragma once

#include <vector>

#include "memcard/memcarduser.h"
#include "os/string.h"

/**
 * Log of the gameplay events of one session, written as text to a file or a memory card when the
 * session ends.
 *
 * The class is not polymorphic. The RTTI of the nested classes records the name. The one instance
 * is the function-local static of shared(), and TheStats addresses it. The constructor
 * clears mActive and mLogName, and no routine of the class sets either of them.
 */
class Stats {
public:
    /**
     * One recorded event.
     *
     * The RTTI records the class as nested in Stats. The name field precedes the vptr.
     */
    class Event {
    public:
        /**
         * Construct an event.
         *
         * @param pszName The name the log writes ahead of the fields.
         */
        explicit Event(const char *pszName) : mName(pszName) {
        }

        /**
         * Release the event.
         *
         * @ghidraAddress NTSC-U/C: 0x00343698
         * @ghidraAddress PAL: 0x003b0bd0
         */
        virtual ~Event() {
        }

        /**
         * Format the fields of the event.
         *
         * @return The fields as text.
         */
        virtual String ToString() const = 0;

        const char *mName; /*!< The name the log writes ahead of the fields. */
    };

    /**
     * A gem caught by a player.
     *
     * The RTTI records the class as nested in Stats.
     */
    class GemHitEvent : public Event {
    public:
        /**
         * Construct the event.
         *
         * @param nPlayer The player.
         * @param nTrack The track.
         * @param nGemTick The tick of the gem.
         * @param nButtonTick The tick of the button press.
         * @param fSlopMs The distance of the press from the gem.
         */
        GemHitEvent(int nPlayer, int nTrack, int nGemTick, int nButtonTick, float fSlopMs)
            : Event("gem_hit"), mPlayer(nPlayer), mTrack(nTrack), mGemTick(nGemTick),
              mButtonTick(nButtonTick), mSlopMs(fSlopMs) {
        }

        /**
         * Release the event.
         *
         * @ghidraAddress NTSC-U/C: 0x00341ee0
         * @ghidraAddress PAL: 0x003af418
         */
        ~GemHitEvent() override {
        }

        /**
         * Format the player, the track, the two ticks, and the slop.
         *
         * @return The fields as text.
         * @ghidraAddress NTSC-U/C: 0x00341f60
         * @ghidraAddress PAL: 0x003af498
         */
        String ToString() const override;

        unsigned char mPlayer; /*!< The player. */
        unsigned char mTrack;  /*!< The track. */
        int mGemTick;          /*!< The tick of the gem. */
        int mButtonTick;       /*!< The tick of the button press. */
        float mSlopMs;         /*!< The distance of the press from the gem. */
    };

    /**
     * A button press that caught no gem.
     *
     * The RTTI records the class as nested in Stats.
     */
    class GemMissEvent : public Event {
    public:
        /**
         * Construct the event.
         *
         * @param nPlayer The player.
         * @param nTrack The track.
         * @param nGemTick The tick of the nearest gem, or 0.
         * @param nButtonTick The tick of the button press.
         * @param fSlopMs The distance of the press from the nearest gem, or 0.
         */
        GemMissEvent(int nPlayer, int nTrack, int nGemTick, int nButtonTick, float fSlopMs)
            : Event("gem_miss"), mPlayer(nPlayer), mTrack(nTrack), mGemTick(nGemTick),
              mButtonTick(nButtonTick), mSlopMs(fSlopMs) {
        }

        /**
         * Release the event.
         *
         * @ghidraAddress NTSC-U/C: 0x003420a8
         * @ghidraAddress PAL: 0x003af5e0
         */
        ~GemMissEvent() override {
        }

        /**
         * Format the player, the track, and the press, and the nearest gem when the slop is not 0.
         *
         * @return The fields as text.
         * @ghidraAddress NTSC-U/C: 0x00342128
         * @ghidraAddress PAL: 0x003af660
         */
        String ToString() const override;

        unsigned char mPlayer; /*!< The player. */
        unsigned char mTrack;  /*!< The track. */
        int mGemTick;          /*!< The tick of the nearest gem, or 0. */
        int mButtonTick;       /*!< The tick of the button press. */
        float mSlopMs;         /*!< The distance of the press from the nearest gem, or 0. */
    };

    /**
     * A gem that passed without a press.
     *
     * The RTTI records the class as nested in Stats.
     */
    class GemPassEvent : public Event {
    public:
        /**
         * Construct the event.
         *
         * @param nPlayer The player.
         * @param nTrack The track.
         * @param nTick The tick of the gem.
         */
        GemPassEvent(int nPlayer, int nTrack, int nTick)
            : Event("gem_pass"), mPlayer(nPlayer), mTrack(nTrack), mTick(nTick) {
        }

        /**
         * Release the event.
         *
         * @ghidraAddress NTSC-U/C: 0x00342288
         * @ghidraAddress PAL: 0x003af7c0
         */
        ~GemPassEvent() override {
        }

        /**
         * Format the player, the track, and the tick.
         *
         * @return The fields as text.
         * @ghidraAddress NTSC-U/C: 0x00342308
         * @ghidraAddress PAL: 0x003af840
         */
        String ToString() const override;

        unsigned char mPlayer; /*!< The player. */
        signed char mTrack;    /*!< The track. */
        int mTick;             /*!< The tick of the gem. */
    };

    /**
     * A player moving to another track.
     *
     * The RTTI records the class as nested in Stats.
     */
    class ChangeTrackEvent : public Event {
    public:
        /**
         * Construct the event.
         *
         * @param nPlayer The player.
         * @param nOldTrack The track the player left.
         * @param nTick The song tick.
         * @param nNewTrack The new track.
         */
        ChangeTrackEvent(int nPlayer, int nOldTrack, int nTick, int nNewTrack)
            : Event("change_track"), mPlayer(nPlayer), mOldTrack(nOldTrack), mNewTrack(nNewTrack),
              mTick(nTick) {
        }

        /**
         * Release the event.
         *
         * @ghidraAddress NTSC-U/C: 0x00342400
         * @ghidraAddress PAL: 0x003af938
         */
        ~ChangeTrackEvent() override {
        }

        /**
         * Format the player, the two tracks, and the tick.
         *
         * @return The fields as text.
         * @ghidraAddress NTSC-U/C: 0x00342480
         * @ghidraAddress PAL: 0x003af9b8
         */
        String ToString() const override;

        unsigned char mPlayer; /*!< The player. */
        signed char mOldTrack; /*!< The track the player left. */
        signed char mNewTrack; /*!< The new track. */
        int mTick;             /*!< The song tick. */
    };

    /**
     * A phrase captured by a player.
     *
     * The RTTI records the class as nested in Stats.
     */
    class CaptureTrackEvent : public Event {
    public:
        /**
         * Construct the event.
         *
         * @param nPlayer The player.
         * @param nTrack The track.
         * @param nTick The song tick.
         * @param nNumBars The bars the capture clears.
         */
        CaptureTrackEvent(int nPlayer, int nTrack, int nTick, int nNumBars)
            : Event("capture_track"), mPlayer(nPlayer), mTrack(nTrack), mTick(nTick),
              mNumBars(nNumBars) {
        }

        /**
         * Release the event.
         *
         * @ghidraAddress NTSC-U/C: 0x003425a0
         * @ghidraAddress PAL: 0x003afad8
         */
        ~CaptureTrackEvent() override {
        }

        /**
         * Format the player, the track, the tick, and the bars.
         *
         * @return The fields as text.
         * @ghidraAddress NTSC-U/C: 0x00342620
         * @ghidraAddress PAL: 0x003afb58
         */
        String ToString() const override;

        unsigned char mPlayer;  /*!< The player. */
        unsigned char mTrack;   /*!< The track. */
        int mTick;              /*!< The song tick. */
        unsigned char mNumBars; /*!< The bars the capture clears. */
    };

    /**
     * The end of a section.
     *
     * The RTTI records the class as nested in Stats.
     */
    class CompleteStageEvent : public Event {
    public:
        /**
         * Construct the event.
         *
         * @param nTick The song tick.
         */
        explicit CompleteStageEvent(int nTick) : Event("complete_stage"), mTick(nTick) {
        }

        /**
         * Release the event.
         *
         * @ghidraAddress NTSC-U/C: 0x00342740
         * @ghidraAddress PAL: 0x003afc78
         */
        ~CompleteStageEvent() override {
        }

        /**
         * Format the tick.
         *
         * @return The fields as text.
         * @ghidraAddress NTSC-U/C: 0x003427c0
         * @ghidraAddress PAL: 0x003afcf8
         */
        String ToString() const override;

        int mTick; /*!< The song tick. */
    };

    /**
     * The start or end of a guitar freestyle.
     *
     * The RTTI records the class as nested in Stats.
     */
    class AxeEvent : public Event {
    public:
        /**
         * Construct the event.
         *
         * @param nPlayer The player.
         * @param nTrack The track.
         * @param nTick The song tick.
         * @param nStart Non-zero at the start of the freestyle.
         */
        AxeEvent(int nPlayer, int nTrack, int nTick, int nStart)
            : Event("axe"), mPlayer(nPlayer), mTrack(nTrack), mTick(nTick), mStart(nStart) {
        }

        /**
         * Release the event.
         *
         * @ghidraAddress NTSC-U/C: 0x00342858
         * @ghidraAddress PAL: 0x003afd90
         */
        ~AxeEvent() override {
        }

        /**
         * Format the player, the track, the tick, and whether the freestyle starts.
         *
         * @return The fields as text.
         * @ghidraAddress NTSC-U/C: 0x003428d8
         * @ghidraAddress PAL: 0x003afe10
         */
        String ToString() const override;

        unsigned char mPlayer; /*!< The player. */
        unsigned char mTrack;  /*!< The track. */
        int mTick;             /*!< The song tick. */
        int mStart;            /*!< Non-zero at the start of the freestyle. */
    };

    /**
     * The start or end of a scratch.
     *
     * The RTTI records the class as nested in Stats.
     */
    class ScratchEvent : public Event {
    public:
        /**
         * Construct the event.
         *
         * @param nPlayer The player.
         * @param nTrack The track.
         * @param nTick The song tick.
         * @param nStart Non-zero at the start of the scratch.
         */
        ScratchEvent(int nPlayer, int nTrack, int nTick, int nStart)
            : Event("scratch"), mPlayer(nPlayer), mTrack(nTrack), mTick(nTick), mStart(nStart) {
        }

        /**
         * Release the event.
         *
         * @ghidraAddress NTSC-U/C: 0x00342a10
         * @ghidraAddress PAL: 0x003aff48
         */
        ~ScratchEvent() override {
        }

        /**
         * Format the player, the track, the tick, and whether the scratch starts.
         *
         * @return The fields as text.
         * @ghidraAddress NTSC-U/C: 0x00342a90
         * @ghidraAddress PAL: 0x003affc8
         */
        String ToString() const override;

        unsigned char mPlayer; /*!< The player. */
        unsigned char mTrack;  /*!< The track. */
        int mTick;             /*!< The song tick. */
        int mStart;            /*!< Non-zero at the start of the scratch. */
    };

    /**
     * A power-up caught by a player.
     *
     * The RTTI records the class as nested in Stats.
     */
    class CatchPowerupEvent : public Event {
    public:
        /**
         * Construct the event.
         *
         * @param nPlayer The player.
         * @param nTick The song tick.
         * @param nType The kind of power-up.
         */
        CatchPowerupEvent(int nPlayer, int nTick, int nType)
            : Event("catch_powerup"), mPlayer(nPlayer), mTick(nTick), mType(nType) {
        }

        /**
         * Release the event.
         *
         * @ghidraAddress NTSC-U/C: 0x00342bc8
         * @ghidraAddress PAL: 0x003b0100
         */
        ~CatchPowerupEvent() override {
        }

        /**
         * Format the player, the tick, and the kind of power-up.
         *
         * @return The fields as text.
         * @ghidraAddress NTSC-U/C: 0x00342c48
         * @ghidraAddress PAL: 0x003b0180
         */
        String ToString() const override;

        unsigned char mPlayer; /*!< The player. */
        int mTick;             /*!< The song tick. */
        unsigned char mType;   /*!< The kind of power-up. */
    };

    /**
     * A power-up deployed by a player.
     *
     * The RTTI records the class as nested in Stats.
     */
    class DeployPowerupEvent : public Event {
    public:
        /**
         * Construct the event.
         *
         * @param nPlayer The player.
         * @param nTick The song tick.
         * @param nType The kind of power-up.
         */
        DeployPowerupEvent(int nPlayer, int nTick, int nType)
            : Event("deploy_powerup"), mPlayer(nPlayer), mTick(nTick), mType(nType) {
        }

        /**
         * Release the event.
         *
         * @ghidraAddress NTSC-U/C: 0x00342d40
         * @ghidraAddress PAL: 0x003b0278
         */
        ~DeployPowerupEvent() override {
        }

        /**
         * Format the player, the tick, and the kind of power-up.
         *
         * @return The fields as text.
         * @ghidraAddress NTSC-U/C: 0x00342dc0
         * @ghidraAddress PAL: 0x003b02f8
         */
        String ToString() const override;

        unsigned char mPlayer; /*!< The player. */
        int mTick;             /*!< The song tick. */
        unsigned char mType;   /*!< The kind of power-up. */
    };

    /**
     * A lost solo song.
     *
     * The RTTI records the class as nested in Stats.
     */
    class LoseSoloGameEvent : public Event {
    public:
        /**
         * Construct the event.
         *
         * @param nTick The song tick.
         * @param nScore The score.
         * @param fProgress The fraction of the song played.
         */
        LoseSoloGameEvent(int nTick, int nScore, float fProgress)
            : Event("lose_solo_game"), mTick(nTick), mScore(nScore), mProgress(fProgress) {
        }

        /**
         * Release the event.
         *
         * @ghidraAddress NTSC-U/C: 0x00342eb8
         * @ghidraAddress PAL: 0x003b03f0
         */
        ~LoseSoloGameEvent() override {
        }

        /**
         * Format the tick, the score, and the fraction played.
         *
         * @return The fields as text.
         * @ghidraAddress NTSC-U/C: 0x00342f38
         * @ghidraAddress PAL: 0x003b0470
         */
        String ToString() const override;

        int mTick;       /*!< The song tick. */
        int mScore;      /*!< The score. */
        float mProgress; /*!< The fraction of the song played. */
    };

    /**
     * A won solo song.
     *
     * The RTTI records the class as nested in Stats.
     */
    class WinSoloGameEvent : public Event {
    public:
        /**
         * Construct the event.
         *
         * @param nScore The score.
         * @param fEnergized The fraction of the possible capture bars captured.
         * @param nFullMixBars The bars played with every track captured.
         * @param nBestStreak The longest streak.
         */
        WinSoloGameEvent(int nScore, float fEnergized, int nFullMixBars, int nBestStreak)
            : Event("win_solo_game"), mScore(nScore), mEnergized(fEnergized),
              mFullMixBars(nFullMixBars), mBestStreak(nBestStreak) {
        }

        /**
         * Release the event.
         *
         * @ghidraAddress NTSC-U/C: 0x00343030
         * @ghidraAddress PAL: 0x003b0568
         */
        ~WinSoloGameEvent() override {
        }

        /**
         * Format the score, the fraction energized, the full mix bars, and the longest streak.
         *
         * @return The fields as text.
         * @ghidraAddress NTSC-U/C: 0x003430b0
         * @ghidraAddress PAL: 0x003b05e8
         */
        String ToString() const override;

        int mScore;       /*!< The score. */
        float mEnergized; /*!< The fraction of the possible capture bars captured. */
        int mFullMixBars; /*!< The bars played with every track captured. */
        int mBestStreak;  /*!< The longest streak. */
    };

    /**
     * The end of a multiplayer song.
     *
     * The RTTI records the class as nested in Stats.
     */
    class EndMultiGameEvent : public Event {
    public:
        /** Number of scores the event records. */
        static constexpr int kNumScores = 4;

        /**
         * Construct the event.
         *
         * @param nScore0 The score of the first player.
         * @param nScore1 The score of the second player.
         * @param nScore2 The score of the third player.
         * @param nScore3 The score of the fourth player.
         */
        EndMultiGameEvent(int nScore0, int nScore1, int nScore2, int nScore3)
            : Event("end_multi_game"), mScores{nScore0, nScore1, nScore2, nScore3} {
        }

        /**
         * Release the event.
         *
         * @ghidraAddress NTSC-U/C: 0x003431d0
         * @ghidraAddress PAL: 0x003b0708
         */
        ~EndMultiGameEvent() override {
        }

        /**
         * Format the four scores.
         *
         * @return The fields as text.
         * @ghidraAddress NTSC-U/C: 0x00343250
         * @ghidraAddress PAL: 0x003b0788
         */
        String ToString() const override;

        int mScores[kNumScores]; /*!< The score of each player. */
    };

    /**
     * A player who left the song.
     *
     * The RTTI records the class as nested in Stats.
     */
    class PlayerAbortedEvent : public Event {
    public:
        /**
         * Construct the event.
         *
         * @param nPlayer The player.
         * @param nTick The song tick.
         */
        PlayerAbortedEvent(int nPlayer, int nTick)
            : Event("player_aborted"), mPlayer(nPlayer), mTick(nTick) {
        }

        /**
         * Release the event.
         *
         * @ghidraAddress NTSC-U/C: 0x00343340
         * @ghidraAddress PAL: 0x003b0878
         */
        ~PlayerAbortedEvent() override {
        }

        /**
         * Format the player and the tick.
         *
         * @return The fields as text.
         * @ghidraAddress NTSC-U/C: 0x003433c0
         * @ghidraAddress PAL: 0x003b08f8
         */
        String ToString() const override;

        unsigned char mPlayer; /*!< The player. */
        int mTick;             /*!< The song tick. */
    };

    /**
     * A new value of the juice meter.
     *
     * The RTTI records the class as nested in Stats.
     */
    class JuiceEvent : public Event {
    public:
        /**
         * Construct the event.
         *
         * @param fValue The juice.
         * @param nTick The song tick.
         */
        JuiceEvent(float fValue, int nTick) : Event("juice"), mValue(fValue), mTick(nTick) {
        }

        /**
         * Release the event.
         *
         * @ghidraAddress NTSC-U/C: 0x00343488
         * @ghidraAddress PAL: 0x003b09c0
         */
        ~JuiceEvent() override {
        }

        /**
         * Format the juice and the tick.
         *
         * @return The fields as text.
         * @ghidraAddress NTSC-U/C: 0x00343508
         * @ghidraAddress PAL: 0x003b0a40
         */
        String ToString() const override;

        float mValue; /*!< The juice. */
        int mTick;    /*!< The song tick. */
    };

    /**
     * Receiver of the result of saving the log to a memory card.
     *
     * The RTTI records the class as nested in Stats and as deriving from MemcardUser.
     */
    class StatsMemcardUser : public MemcardUser {
    public:
        /**
         * Empty the log once the memory card task ends.
         *
         * @param nStatus The result of the task. The body does not read it.
         * @ghidraAddress NTSC-U/C: 0x00341eb8
         * @ghidraAddress PAL: 0x003af3f0
         */
        void OnFileSaved(int nStatus) override;
    };

    /**
     * Construct an inactive log.
     *
     * @ghidraAddress NTSC-U/C: 0x0013bf70
     * @ghidraAddress PAL: 0x0013d840
     */
    Stats();

    /**
     * Release the log.
     *
     * @ghidraAddress NTSC-U/C: 0x0013c030
     * @ghidraAddress PAL: 0x0013d900
     */
    ~Stats();

    /**
     * Report the single instance, constructing it on first use.
     *
     * @return The instance.
     * @ghidraAddress NTSC-U/C: 0x0013c120
     * @ghidraAddress PAL: 0x0013d9f0
     */
    static Stats *shared();

    /**
     * Start the log of a song.
     *
     * The body is empty.
     *
     * @param nNumTracks The number of tracks.
     * @param nNumBars The length of the song in bars.
     * @ghidraAddress NTSC-U/C: 0x0013c178
     * @ghidraAddress PAL: 0x0013da48
     */
    void Begin([[maybe_unused]] int nNumTracks, [[maybe_unused]] int nNumBars) {
    }

    /**
     * End the session, write the log, and release the events.
     *
     * @ghidraAddress NTSC-U/C: 0x0013c180
     * @ghidraAddress PAL: 0x0013da50
     */
    void End();

    /**
     * Record a caught gem.
     *
     * @param nTrack The track.
     * @param nGemTick The tick of the gem.
     * @param nButtonTick The tick of the button press.
     * @param fSlopMs The distance of the press from the gem.
     * @ghidraAddress NTSC-U/C: 0x0013c338
     * @ghidraAddress PAL: 0x0013dc08
     */
    void GemHit(int nTrack, int nGemTick, int nButtonTick, float fSlopMs);

    /**
     * Record a press that missed the nearest gem.
     *
     * @param nTrack The track.
     * @param nGemTick The tick of the nearest gem.
     * @param nButtonTick The tick of the button press.
     * @param fSlopMs The distance of the press from the nearest gem.
     * @ghidraAddress NTSC-U/C: 0x0013c508
     * @ghidraAddress PAL: 0x0013ddd8
     */
    void GemMiss(int nTrack, int nGemTick, int nButtonTick, float fSlopMs);

    /**
     * Record a press with no gem near it.
     *
     * @param nTrack The track.
     * @param nButtonTick The tick of the button press.
     * @ghidraAddress NTSC-U/C: 0x0013c6d8
     * @ghidraAddress PAL: 0x0013dfa8
     */
    void GemMiss(int nTrack, int nButtonTick);

    /**
     * Record a gem that passed without a press on a track that has a player.
     *
     * @param nTrack The track.
     * @param nTick The tick of the gem.
     * @ghidraAddress NTSC-U/C: 0x0013c890
     * @ghidraAddress PAL: 0x0013e160
     */
    void GemPass(int nTrack, int nTick);

    /**
     * Record which player plays a track.
     *
     * @param nTrack The track index.
     * @param nPlayer The player index, or -1 for none.
     * @ghidraAddress NTSC-U/C: 0x0013ca50
     * @ghidraAddress PAL: 0x0013e320
     */
    void SetTrackPlayer(int nTrack, int nPlayer);

    /**
     * Record a player moving to another track.
     *
     * @param nPlayer The player's index.
     * @param nTrack The index of the new track.
     * @param nTick The song tick.
     * @ghidraAddress NTSC-U/C: 0x0013ca70
     * @ghidraAddress PAL: 0x0013e340
     */
    void ChangeTrack(int nPlayer, int nTrack, int nTick);

    /**
     * Record a captured phrase.
     *
     * @param nTrack The track.
     * @param nTick The song tick.
     * @param nNumBars The bars the capture clears.
     * @ghidraAddress NTSC-U/C: 0x0013cc40
     * @ghidraAddress PAL: 0x0013e510
     */
    void CaptureTrack(int nTrack, int nTick, int nNumBars);

    /**
     * Record the end of a section.
     *
     * @param nTick The song tick.
     * @ghidraAddress NTSC-U/C: 0x0013ce00
     * @ghidraAddress PAL: 0x0013e6d0
     */
    void CompleteStage(int nTick);

    /**
     * Record the start of a guitar freestyle.
     *
     * @param nTrack The track.
     * @param nTick The song tick.
     * @ghidraAddress NTSC-U/C: 0x0013cf90
     * @ghidraAddress PAL: 0x0013e860
     */
    void AxeBegin(int nTrack, int nTick);

    /**
     * Record the end of a guitar freestyle.
     *
     * @param nTrack The track.
     * @param nTick The song tick.
     * @ghidraAddress NTSC-U/C: 0x0013d148
     * @ghidraAddress PAL: 0x0013ea18
     */
    void AxeEnd(int nTrack, int nTick);

    /**
     * Record the start of a scratch.
     *
     * @param nTrack The track index.
     * @param nTick The song tick.
     * @ghidraAddress NTSC-U/C: 0x0013d300
     * @ghidraAddress PAL: 0x0013ebd0
     */
    void ScratchBegin(int nTrack, int nTick);

    /**
     * Record the end of a scratch.
     *
     * @param nTrack The track index.
     * @param nTick The song tick.
     * @ghidraAddress NTSC-U/C: 0x0013d4b8
     * @ghidraAddress PAL: 0x0013ed88
     */
    void ScratchEnd(int nTrack, int nTick);

    /**
     * Record a deployed power-up.
     *
     * @param nPlayer The player.
     * @param nTick The song tick.
     * @param nType The kind of power-up.
     * @ghidraAddress NTSC-U/C: 0x0013d670
     * @ghidraAddress PAL: 0x0013ef40
     */
    void DeployPowerup(int nPlayer, int nTick, int nType);

    /**
     * Record a caught power-up.
     *
     * @param nPlayer The player.
     * @param nTick The song tick.
     * @param nType The kind of power-up.
     * @ghidraAddress NTSC-U/C: 0x0013d818
     * @ghidraAddress PAL: 0x0013f0e8
     */
    void CatchPowerup(int nPlayer, int nTick, int nType);

    /**
     * Record a lost solo song.
     *
     * @param nTick The song tick of the loss.
     * @param nScore The score.
     * @param fProgress The fraction of the song played.
     * @ghidraAddress NTSC-U/C: 0x0013d9c0
     * @ghidraAddress PAL: 0x0013f290
     */
    void LoseSoloGame(int nTick, int nScore, float fProgress);

    /**
     * Record a won solo song.
     *
     * @param nScore The score.
     * @param nFullMixBars The bars played with every track captured.
     * @param nBestStreak The longest streak.
     * @param fEnergized The fraction of the possible capture bars captured.
     * @ghidraAddress NTSC-U/C: 0x0013db68
     * @ghidraAddress PAL: 0x0013f438
     */
    void WinSoloGame(int nScore, int nFullMixBars, int nBestStreak, float fEnergized);

    /**
     * Record the end of a multiplayer song.
     *
     * @param nScore0 The score of the first player.
     * @param nScore1 The score of the second player.
     * @param nScore2 The score of the third player.
     * @param nScore3 The score of the fourth player.
     * @ghidraAddress NTSC-U/C: 0x0013dd20
     * @ghidraAddress PAL: 0x0013f5f0
     */
    void EndMultiGame(int nScore0, int nScore1, int nScore2, int nScore3);

    /**
     * Record a player who left the song.
     *
     * @param nPlayer The player.
     * @param nTick The song tick.
     * @ghidraAddress NTSC-U/C: 0x0013ded8
     * @ghidraAddress PAL: 0x0013f7a8
     */
    void PlayerAborted(int nPlayer, int nTick);

    /**
     * Record a new value of the juice meter.
     *
     * @param fValue The juice.
     * @param nTick The song tick.
     * @ghidraAddress NTSC-U/C: 0x0013e070
     * @ghidraAddress PAL: 0x0013f940
     */
    void Juice(float fValue, int nTick);

    std::vector<int> mPlayerTracks; /*!< The track of each player. */
    std::vector<int> mTrackPlayers; /*!< The player of each track, or -1. */
    int mSongBars;                  /*!< The "song_bars" value of the log, initially -1. */
    String mStartTime;              /*!< The "start_time" value of the log. */
    std::vector<Event *> mEvents;   /*!< The recorded events, in order. */
    const char *mLogName;           /*!< The name of the log file, or null to record nothing. */
    int mMemcardSlot;               /*!< The memory card slot of the log, or -1 for a file. */
    String mLog;                    /*!< The text of the log. */
    StatsMemcardUser *mMemcardUser; /*!< Receives the result of saving the log. */
    int mActive;                    /*!< Non-zero while the session records events. */

private:
    /**
     * Format one field of the log as `(name value)` and a line break.
     *
     * @param name The name of the field.
     * @param pszValue The value.
     * @param bQuoted Enclose the value in double quotes.
     * @return The line.
     * @ghidraAddress NTSC-U/C: 0x0013bd20
     * @ghidraAddress PAL: 0x0013d5f0
     */
    static String FormatField(const String &name, const char *pszValue, bool bQuoted);

    /**
     * Format one integer field of the log.
     *
     * @param name The name of the field.
     * @param nValue The value.
     * @return The line.
     * @ghidraAddress NTSC-U/C: 0x0013be30
     * @ghidraAddress PAL: 0x0013d700
     */
    static String FormatField(const String &name, int nValue);

    /**
     * Format one field of the log as `TRUE` or `FALSE`.
     *
     * @param name The name of the field.
     * @param bValue The value.
     * @return The line.
     * @ghidraAddress NTSC-U/C: 0x0013beb0
     * @ghidraAddress PAL: 0x0013d780
     */
    static String FormatField(const String &name, bool bValue);

    /**
     * Append the date and time of the console clock to a file name.
     *
     * @param text The file name.
     * @ghidraAddress NTSC-U/C: 0x0013bef8
     * @ghidraAddress PAL: 0x0013d7c8
     */
    static void AppendTimestamp(String &text);

    /**
     * Write the settings of the game and every event to mLog.
     *
     * @ghidraAddress NTSC-U/C: 0x0013e210
     * @ghidraAddress PAL: 0x0013fae0
     */
    void WriteSessionLog();
};

/**
 * The session log, Stats::shared() as the unit's static initialiser stored it.
 *
 * @ghidraAddress NTSC-U/C: 0x004361d8
 */
extern Stats *TheStats;

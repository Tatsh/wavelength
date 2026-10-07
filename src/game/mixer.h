#pragma once

#include <vector>

#include "os/command.h"
#include "os/ptr.h"
#include "os/ramp.h"
#include "script/dataarray.h"

/**
 * Volume control of the song's tracks.
 *
 * The RTTI includes the class name. The one instance is the function-local static of shared(),
 * and TheMixer addresses it. Each of the sixteen synthesiser channels has a VolumeRamp that sends
 * the channel's volume as MIDI controller 17. The last channel is the effects channel.
 *
 * While the mixer plays (kModePlaying), a track a player is on plays at mHighVolume, and any
 * other track plays at the entry of its volume list that the number of held tracks selects. With
 * an instrument track whose last track is occupied, every other track uses
 * mInstrumentVolumes instead. The `track_solo` cheat mutes the tracks nobody is on.
 */
class Mixer {
public:
    /** What the mixer is doing, the values of mMode. */
    enum Mode {
        kModeOff = 0,        /*!< The volumes are left alone. */
        kModePlaying = 1,    /*!< The volumes follow the players. */
        kModeFullMix = 2,    /*!< Every track plays at mHighVolume. */
        kModeVictoryLap = 3, /*!< The song is ending after a win. */
    };

    /** The instrument tracks Init() configures the mixer for. */
    enum Instrument {
        kInstrumentNone = 0,    /*!< The song has no instrument track. */
        kInstrumentAxe = 1,     /*!< The last track is played with the axe. */
        kInstrumentScratch = 2, /*!< The last track is played with the scratcher. */
    };

    /** The number of synthesiser channels the mixer controls. */
    static constexpr int kChannelCount = 16;

    /** The channel of the effects. */
    static constexpr int kFxChannel = kChannelCount - 1;

    /** The state of one of the song's tracks. */
    struct TrackData {
        int mPlayers = 0;                      /*!< The number of players on the track. */
        bool mHeld = false;                    /*!< Whether HoldTrack() holds the track. */
        Ptr<Command> mReleaseCommand{nullptr}; /*!< The command that calls ReleaseTrack(). */
    };

    /**
     * Ramp that sends one synthesiser channel's volume.
     *
     * The RTTI records the class as deriving from Ramp.
     */
    class VolumeRamp : public Ramp {
    public:
        /**
         * Construct a ramp at full volume for a channel.
         *
         * Inline. The Mixer constructor expands it.
         *
         * @param pScheduler The scheduler the steps are queued on.
         * @param fValue The starting volume.
         * @param nChannel The channel.
         */
        VolumeRamp(Scheduler *pScheduler, float fValue, unsigned char nChannel)
            : Ramp(pScheduler, fValue), mChannel(nChannel) {
        }

        /**
         * Send a volume to the channel as MIDI controller 17.
         *
         * @param fValue The volume.
         * @param nTick Unused.
         * @ghidraAddress NTSC-U/C: 0x0033bd18
         * @ghidraAddress PAL: 0x003a9250
         */
        void Apply(float fValue, int nTick) override;

    private:
        unsigned char mChannel; /*!< The channel. */
    };

    /**
     * Construct a mixer with no tracks and one ramp per channel.
     *
     * @ghidraAddress NTSC-U/C: 0x00123870
     * @ghidraAddress PAL: 0x00124ff0
     */
    Mixer();

    /**
     * Destroy the ramps.
     *
     * @ghidraAddress NTSC-U/C: 0x00123988
     * @ghidraAddress PAL: 0x00125108
     */
    ~Mixer();

    /**
     * Report the single instance, constructing it on first use.
     *
     * @return The instance.
     * @ghidraAddress NTSC-U/C: 0x00123ee8
     * @ghidraAddress PAL: 0x00125668
     */
    static Mixer *shared();

    /**
     * Set the mixer up for a song and read the "mixer" section of the configuration.
     *
     * Every track starts with no players and unheld. Each channel's volume list is the
     * configuration's `low_volumes`, except the effects channel's, which is `fx_volumes`. Every
     * channel then moves at once to the first entry of its list.
     *
     * @param nTracks The number of tracks in the song.
     * @param nInstrument One of Instrument.
     * @ghidraAddress NTSC-U/C: 0x00123ad0
     * @ghidraAddress PAL: 0x00125250
     */
    void Init(int nTracks, int nInstrument);

    /**
     * Report whether a track is the instrument track.
     *
     * @param nTrack The track's index.
     * @return Whether the song has an instrument track and the track is the last one.
     * @ghidraAddress NTSC-U/C: 0x00123e38
     * @ghidraAddress PAL: 0x001255b8
     */
    bool IsInstrumentTrack(int nTrack) const;

    /**
     * Replace a channel's volume list.
     *
     * @param nChannel The channel.
     * @param volumes The volumes.
     * @ghidraAddress NTSC-U/C: 0x00123e78
     * @ghidraAddress PAL: 0x001255f8
     */
    void SetVolumes(int nChannel, const std::vector<unsigned char> &volumes);

    /**
     * Replace the volume list the other tracks use while the instrument track is occupied.
     *
     * @param volumes The volumes.
     * @ghidraAddress NTSC-U/C: 0x00123ea0
     * @ghidraAddress PAL: 0x00125620
     */
    void SetInstrumentVolumes(const std::vector<unsigned char> &volumes);

    /**
     * Count one more player on a track and refresh the track's volume.
     *
     * @param nTrack The track's index.
     * @ghidraAddress NTSC-U/C: 0x00123f40
     * @ghidraAddress PAL: 0x001256c0
     */
    void AddPlayer(int nTrack);

    /**
     * Count one player fewer on a track and refresh the track's volume.
     *
     * @param nTrack The track's index.
     * @ghidraAddress NTSC-U/C: 0x00123fc8
     * @ghidraAddress PAL: 0x00125748
     */
    void RemovePlayer(int nTrack);

    /**
     * Hold a track until the tick before a song tick, and refresh every other channel.
     *
     * Nothing happens once the song has passed the tick.
     *
     * @param nTrack The track's index.
     * @param nUntilTick The tick the hold ends at.
     * @ghidraAddress NTSC-U/C: 0x00124070
     * @ghidraAddress PAL: 0x001257f0
     */
    void HoldTrack(int nTrack, int nUntilTick);

    /**
     * End the hold on a track and refresh every channel.
     *
     * @param nTrack The track's index.
     * @ghidraAddress NTSC-U/C: 0x00124150
     * @ghidraAddress PAL: 0x001258d0
     */
    void ReleaseTrack(int nTrack);

    /**
     * Play every track but the effects at mHighVolume.
     *
     * @ghidraAddress NTSC-U/C: 0x001241c0
     * @ghidraAddress PAL: 0x00125940
     */
    void PlayFullMix();

    /**
     * Move every channel to the victory lap volume, and an occupied instrument track to
     * mHighVolume, over 1920 ticks.
     *
     * @ghidraAddress NTSC-U/C: 0x00124220
     * @ghidraAddress PAL: 0x001259a0
     */
    void StartVictoryLap();

    /**
     * Start following the players and register the `track_solo` cheat.
     *
     * A mixer that is not off is unchanged.
     *
     * @ghidraAddress NTSC-U/C: 0x001242d0
     * @ghidraAddress PAL: 0x00125a50
     */
    void Activate();

    /**
     * Stop following the players, unregister the cheat, and return every track to full volume.
     *
     * A mixer that is off is unchanged.
     *
     * @ghidraAddress NTSC-U/C: 0x00124328
     * @ghidraAddress PAL: 0x00125aa8
     */
    void Deactivate();

    /**
     * Fade every track but the effects to silence.
     *
     * @param nTicks The ticks the fade lasts.
     * @ghidraAddress NTSC-U/C: 0x001243b0
     * @ghidraAddress PAL: 0x00125b30
     */
    void FadeOut(int nTicks);

    /**
     * Move a channel to a volume.
     *
     * @param nChannel The channel.
     * @param nVolume The volume.
     * @param nTicks The ticks the move lasts.
     * @ghidraAddress NTSC-U/C: 0x001244c8
     * @ghidraAddress PAL: 0x00125c48
     */
    void SetVolume(int nChannel, unsigned char nVolume, int nTicks);

    /**
     * Report a channel's volume.
     *
     * @param nChannel The channel.
     * @return The volume.
     * @ghidraAddress NTSC-U/C: 0x00124508
     * @ghidraAddress PAL: 0x00125c88
     */
    unsigned char GetVolume(int nChannel);

    /**
     * Move every channel to the volume its track's state selects.
     *
     * @param nTicks The ticks the moves last.
     * @ghidraAddress NTSC-U/C: 0x00124640
     * @ghidraAddress PAL: 0x00125dc0
     */
    void RefreshAll(int nTicks);

    /**
     * Toggle muting the tracks nobody is on, the `track_solo` script command.
     *
     * @param pCommand The command.
     * @param pUserData Unused.
     * @ghidraAddress NTSC-U/C: 0x001246a0
     * @ghidraAddress PAL: 0x00125e20
     */
    static void ToggleMuteUnoccupied(DataArray *pCommand, void *pUserData);

private:
    /**
     * Read a list of volumes that follows a key in the configuration.
     *
     * @param pConfig The configuration section.
     * @param pszKey The key.
     * @param volumes Receives the volumes, resized to the length of the list.
     * @ghidraAddress NTSC-U/C: 0x00123748
     * @ghidraAddress PAL: 0x00124ec8
     */
    static void
    ReadVolumes(const DataArray *pConfig, const char *pszKey, std::vector<unsigned char> &volumes);

    /**
     * Read the integer that follows a key in the configuration.
     *
     * @param pszKey The key.
     * @param pConfig The configuration section.
     * @return The integer.
     * @ghidraAddress NTSC-U/C: 0x00123800
     * @ghidraAddress PAL: 0x00124f80
     */
    static int ReadInt(const char *pszKey, const DataArray *pConfig);

    /**
     * Read the integer that follows a key in the configuration as a volume.
     *
     * @param pszKey The key.
     * @param pConfig The configuration section.
     * @return The volume.
     * @ghidraAddress NTSC-U/C: 0x00123838
     * @ghidraAddress PAL: 0x00124fb8
     */
    static unsigned char ReadByte(const char *pszKey, const DataArray *pConfig);

    /**
     * Move a channel to the entry of a volume list that the number of held tracks selects.
     *
     * The muting cheat selects silence, and two or more controllers on this console select
     * mHighVolume.
     *
     * @param nChannel The channel.
     * @param volumes The volume list.
     * @param nTicks The ticks the move lasts.
     * @ghidraAddress NTSC-U/C: 0x00124420
     * @ghidraAddress PAL: 0x00125ba0
     */
    void ApplyVolume(int nChannel, const std::vector<unsigned char> &volumes, int nTicks);

    /**
     * Move a channel to the volume its track's state selects.
     *
     * @param nChannel The channel.
     * @param nTicks The ticks the move lasts.
     * @ghidraAddress NTSC-U/C: 0x00124538
     * @ghidraAddress PAL: 0x00125cb8
     */
    void RefreshTrack(int nChannel, int nTicks);

    int mRampTicks;                                     /*!< `ramp_ticks`. */
    std::vector<TrackData> mTracks;                     /*!< The song's tracks. */
    unsigned char mHighVolume;                          /*!< `high_volume`. */
    std::vector<unsigned char> mVolumes[kChannelCount]; /*!< Each channel's volume list. */
    std::vector<unsigned char> mInstrumentVolumes;      /*!< The instrument's `low_volumes`. */
    unsigned char mVictoryLapVolume;                    /*!< The instrument's victory lap volume. */
    int mHeldCount;                                     /*!< The number of held tracks. */
    int mMode;                                          /*!< One of Mode. */
    bool mHasInstrumentTrack;          /*!< Whether there is an instrument track. */
    bool mMuteUnoccupied;              /*!< The `track_solo` cheat. */
    VolumeRamp *mRamps[kChannelCount]; /*!< Each channel's volume. */
};

/**
 * The track mixer, Mixer::shared() as the unit's static initialiser stored it.
 *
 * @ghidraAddress NTSC-U/C: 0x00436144
 */
extern Mixer *TheMixer;

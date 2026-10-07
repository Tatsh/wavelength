#pragma once

#include <vector>

#include "game/catchtrackdata.h"
#include "game/playmap.h"
#include "game/remixtrack.h"
#include "game/sectionboundaries.h"
#include "game/slotgrid.h"
#include "game/voxtrackdisplay.h"
#include "game/voxtrackmusic.h"
#include "os/binstream.h"

/**
 * Vocal track of a remixed song, whose players switch its sections on and off.
 *
 * The RTTI records the class as deriving from RemixTrack. The track plays its gems unless it is
 * active, a player on another console silences it, or the section that plays is switched off. The
 * names of the members other than the Track and RemixTrack overrides are inferred.
 */
class VoxTrack : public RemixTrack {
public:
    /**
     * Construct the track for one song track.
     *
     * @param pGems The gems of the song track.
     * @param pPlayMap The map of the song positions.
     * @param pSections The sections of the song.
     * @param pPatternGrid The pattern each section repeats.
     * @param nTrack The index of the song track.
     * @param nLeadInBars The bars the song plays before bar 0.
     * @param nNumBars The length of the song in bars.
     * @param nTicksPerBar The song ticks in one bar.
     * @ghidraAddress NTSC-U/C: 0x00142880
     * @ghidraAddress PAL: 0x00144220
     */
    VoxTrack(CatchTrackData *pGems,
             PlayMap *pPlayMap,
             SectionBoundaries *pSections,
             SlotGrid *pPatternGrid,
             int nTrack,
             int nLeadInBars,
             int nNumBars,
             int nTicksPerBar);

    /**
     * Release the track.
     *
     * @ghidraAddress NTSC-U/C: 0x00142aa8
     * @ghidraAddress PAL: 0x00144448
     */
    ~VoxTrack() override;

    /**
     * Start the track with the song.
     *
     * @ghidraAddress NTSC-U/C: 0x00142b38
     * @ghidraAddress PAL: 0x001444d8
     */
    void Start() override;

    /**
     * Stop the track with the song.
     *
     * @ghidraAddress NTSC-U/C: 0x00142b70
     * @ghidraAddress PAL: 0x00144510
     */
    void Stop() override;

    /**
     * Give the track to the player it plays for, and show the player an empty phrase on it.
     *
     * @param pPlayer The player, or null.
     * @ghidraAddress NTSC-U/C: 0x00142ba0
     * @ghidraAddress PAL: 0x00144540
     */
    void SetPlayer(Player *pPlayer) override;

    /**
     * Play the sound of an edit the track refuses when a gem button is pressed.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00143068
     * @ghidraAddress PAL: 0x00144a08
     */
    void HandleInput(Player *pPlayer, const PlayNoteEvent &event) override;

    /**
     * Play the sound of an edit the track refuses.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x00143090
     * @ghidraAddress PAL: 0x00144a30
     */
    void HandleInput(Player *pPlayer, const BtnEvent<8> &event) override;

    /**
     * Ignore a stick event. The body is empty.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x003444b0
     */
    void HandleInput([[maybe_unused]] Player *pPlayer,
                     [[maybe_unused]] const StickEvent<2> &event) override {
    }

    /**
     * Ignore a stick event. The body is empty.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x003444b8
     */
    void HandleInput([[maybe_unused]] Player *pPlayer,
                     [[maybe_unused]] const StickEvent<6> &event) override {
    }

    /**
     * Ignore a button event. The body is empty.
     *
     * @param pPlayer The player.
     * @param event The event.
     * @ghidraAddress NTSC-U/C: 0x003444c0
     */
    void HandleInput([[maybe_unused]] Player *pPlayer,
                     [[maybe_unused]] const BtnEvent<10> &event) override {
    }

    /**
     * Set whether the track is silent while a player on another console is on it.
     *
     * @param bMuteRemote Whether the track is silent.
     * @ghidraAddress NTSC-U/C: 0x00142c10
     * @ghidraAddress PAL: 0x001445b0
     */
    void SetMuteRemote(bool bMuteRemote) override;

    /**
     * Report whether the track is silent while a player on another console is on it.
     *
     * @return mMuteRemote.
     * @ghidraAddress NTSC-U/C: 0x00142c30
     */
    bool GetMuteRemote() const override;

    /**
     * Show a range of bars empty.
     *
     * @param nStartBar The first bar of the range.
     * @param nEndBar The bar after the range.
     * @ghidraAddress NTSC-U/C: 0x00142e30
     * @ghidraAddress PAL: 0x001447d0
     */
    void SkipBars(int nStartBar, int nEndBar) override;

    /**
     * Copy the switch of each section's pattern into the section.
     *
     * @ghidraAddress NTSC-U/C: 0x00142e50
     * @ghidraAddress PAL: 0x001447f0
     */
    void RebuildGems() override;

    /**
     * Set whether the track is the one the song's player plays, which silences it.
     *
     * @param bActive Whether the track is active.
     * @ghidraAddress NTSC-U/C: 0x00142c58
     * @ghidraAddress PAL: 0x001445f8
     */
    void SetActive(bool bActive) override;

    /**
     * Report whether the track is the one the song's player plays.
     *
     * @return mActive.
     * @ghidraAddress NTSC-U/C: 0x00142c78
     */
    bool IsActive() const override;

    /**
     * Show the track again from a bar on.
     *
     * @param nStartBar The first bar.
     * @ghidraAddress NTSC-U/C: 0x00143048
     * @ghidraAddress PAL: 0x001449e8
     */
    void Redraw(int nStartBar) override;

    /**
     * Write the switch of each section, one byte per section.
     *
     * @param stream The stream.
     * @ghidraAddress NTSC-U/C: 0x001430b0
     * @ghidraAddress PAL: 0x00144a50
     */
    void Save(BinStream &stream) override;

    /**
     * Read the switch of each section, one byte per section.
     *
     * @param stream The stream.
     * @ghidraAddress NTSC-U/C: 0x00143268
     * @ghidraAddress PAL: 0x00144c00
     */
    void Load(BinStream &stream) override;

    /**
     * Set the section the song plays.
     *
     * @param nSection The section.
     * @ghidraAddress NTSC-U/C: 0x00142c38
     * @ghidraAddress PAL: 0x001445d8
     */
    void SetSection(int nSection);

    /**
     * Switch a section off or on.
     *
     * @param nSection The section.
     * @param bMuted Whether the section is silent.
     * @ghidraAddress NTSC-U/C: 0x00142c80
     * @ghidraAddress PAL: 0x00144620
     */
    void SetSectionMuted(int nSection, bool bMuted);

    /**
     * Report whether a section is switched off.
     *
     * @param nSection The section.
     * @return Whether the section is silent.
     * @ghidraAddress NTSC-U/C: 0x00142d70
     * @ghidraAddress PAL: 0x00144710
     */
    bool IsSectionMuted(int nSection) const;

private:
    /**
     * Silence the music while the track is active, while a player on another console or no player
     * is on it and such players silence it, or while the section that plays is switched off.
     *
     * @ghidraAddress NTSC-U/C: 0x00143420
     * @ghidraAddress PAL: 0x00144db0
     */
    void UpdateMute();

    CatchTrackData *mGems;           /*!< The gems of the song track. */
    VoxTrackDisplay mDisplay;        /*!< The display of the gems. */
    VoxTrackMusic mMusic;            /*!< The player of the gems. */
    int mMuteRemote;                 /*!< Nonzero while remote players silence the track. */
    int mActive;                     /*!< Nonzero while the song's player plays the track. */
    std::vector<bool> mSectionMuted; /*!< Whether each section is switched off. */
    int mSection;                    /*!< The section the song plays. */
    SectionBoundaries *mSections;    /*!< The sections of the song. */
    SlotGrid *mPatternGrid;          /*!< The pattern each section repeats. */
};

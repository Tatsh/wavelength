#pragma once

#include "game/track.h"
#include "os/binstream.h"

/**
 * Track of a song that players edit in a remix.
 *
 * The RTTI includes the class name and records Track as the base. The class is abstract and has no
 * vtable of its own. PitchTrack and VoxTrack derive from it and fill the members it adds in the
 * same order. The member names are inferred.
 */
class RemixTrack : public Track {
public:
    /**
     * Construct a track with no player.
     *
     * Inline. The constructors of the derived classes expand it.
     *
     * @param nIndex The track's index in the song.
     */
    explicit RemixTrack(int nIndex) : Track(nIndex) {
    }

    /**
     * Set whether the track is silent while a player on another console is on it.
     *
     * @param bMuteRemote Whether the track is silent.
     */
    virtual void SetMuteRemote(bool bMuteRemote) = 0;

    /**
     * Report whether the track is silent while a player on another console is on it.
     *
     * @return Whether the track is silent.
     */
    virtual bool GetMuteRemote() const = 0;

    /**
     * Show a range of bars empty, and start the phrases at the end of the range.
     *
     * @param nStartBar The first bar of the range.
     * @param nEndBar The bar after the range.
     */
    virtual void SkipBars(int nStartBar, int nEndBar) = 0;

    /** Copy the gems of each section's pattern into the section. */
    virtual void RebuildGems() = 0;

    /**
     * Set whether the track is the one the song's player plays, which silences its own riffs.
     *
     * @param bActive Whether the track is active.
     */
    virtual void SetActive(bool bActive) = 0;

    /**
     * Report whether the track is the one the song's player plays.
     *
     * @return Whether the track is active.
     */
    virtual bool IsActive() const = 0;

    /**
     * Show the track again from a bar on.
     *
     * @param nStartBar The first bar.
     */
    virtual void Redraw(int nStartBar) = 0;

    /**
     * Write the edits of the track.
     *
     * @param stream The stream.
     */
    virtual void Save(BinStream &stream) = 0;

    /**
     * Read the edits of the track.
     *
     * @param stream The stream.
     */
    virtual void Load(BinStream &stream) = 0;
};

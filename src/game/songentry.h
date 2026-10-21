#pragma once

#include <vector>

#include "script/dataarray.h"

/**
 * View of the entry of one song in the "songs" section of the configuration.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. The object is the one
 * pointer to the entry.
 */
class SongEntry {
public:
    SongEntry() = default;

    /**
     * Construct the entry of a song.
     *
     * @param pData The entry, or null.
     */
    explicit SongEntry(DataArray *pData) : mData(pData) {
    }

    /** Values of GetType(). */
    enum Type {
        kTypeNormal = 0,   /*!< A song whose entry does not specify a type. */
        kTypeBoss = 1,     /*!< The boss song of an arena. */
        kTypeBonus = 2,    /*!< A bonus song. */
        kTypeSecret = 3,   /*!< A secret song. */
        kTypeTutorial = 4, /*!< The tutorial. */
    };

    /**
     * Report the `type` of the song.
     *
     * @return The type, or 0 when the entry does not specify one.
     * @ghidraAddress NTSC-U/C: 0x0027d320
     * @ghidraAddress PAL: 0x00286c38
     */
    int GetType() const;

    /**
     * Report the name of the song, the first symbol of the entry.
     *
     * @return The name.
     * @ghidraAddress NTSC-U/C: 0x0027cee0
     * @ghidraAddress PAL: 0x002867f8
     */
    const char *GetName() const;

    /**
     * Report the number of the song, the `id` of the entry.
     *
     * @return The number.
     * @ghidraAddress NTSC-U/C: 0x0027ce40
     * @ghidraAddress PAL: 0x00286758
     */
    int GetId() const;

    /**
     * Report the localised title of the song. The name is inferred.
     *
     * @return The title.
     * @ghidraAddress NTSC-U/C: 0x0027cf00
     * @ghidraAddress PAL: 0x00286818
     */
    const char *GetTitle() const;

    /**
     * Report the localised artist, the token `<name>_ARTIST`.
     *
     * @return The artist.
     * @ghidraAddress NTSC-U/C: 0x0027d090
     * @ghidraAddress PAL: 0x002869a8
     */
    const char *GetArtist() const;

    /**
     * Report the localised record label, the token `<name>_LABEL`.
     *
     * The name is inferred.
     *
     * @return The record label.
     * @ghidraAddress NTSC-U/C: 0x0027cf48
     * @ghidraAddress PAL: 0x00286860
     */
    const char *GetLabel() const;

    /**
     * Report the localised short form of the artist, the token `<name>_ARTIST_SHORT`.
     *
     * @return The short artist.
     * @ghidraAddress NTSC-U/C: 0x0027d0d8
     * @ghidraAddress PAL: 0x002869f0
     */
    const char *GetArtistShort() const;

    /**
     * Report the localised shortest form of the artist, the token `<name>_ARTIST_SHORTEST`, or
     * GetArtistShort() when the locale lacks the token.
     *
     * @return The shortest artist.
     * @ghidraAddress NTSC-U/C: 0x0027d120
     * @ghidraAddress PAL: 0x00286a38
     */
    const char *GetArtistShortest() const;

    /**
     * Report the localised biography, the token `<name>_BIO`.
     *
     * @return The biography.
     * @ghidraAddress NTSC-U/C: 0x0027d1a0
     * @ghidraAddress PAL: 0x00286ab8
     */
    const char *GetBio() const;

    /**
     * Report the localised web address, the token `<name>_WWW`.
     *
     * @return The web address.
     * @ghidraAddress NTSC-U/C: 0x0027d1e8
     * @ghidraAddress PAL: 0x00286b00
     */
    const char *GetWww() const;

    /**
     * Report the localised genre, the token `<name>_GENRE`, with the tempo when the genre is not
     * empty.
     *
     * @return The genre.
     * @ghidraAddress NTSC-U/C: 0x0027d230
     * @ghidraAddress PAL: 0x00286b48
     */
    const char *GetGenre() const;

    /**
     * Report the localised genre, the token `<name>_GENRE`, without the tempo.
     *
     * @return The genre.
     * @ghidraAddress NTSC-U/C: 0x0027d2d8
     * @ghidraAddress PAL: 0x00286bf0
     */
    const char *GetGenreName() const;

    /**
     * Report the localised name of a remix of the song, the token `<name>_REMIX_TITLE`.
     *
     * Without a translation of the token, the routine reports the result of the routine at
     * `0x0027cf90`.
     *
     * @return The name.
     * @ghidraAddress NTSC-U/C: 0x0027d010
     * @ghidraAddress PAL: 0x00286928
     */
    const char *GetRemixTitle() const;

    /**
     * Report the `bpm` of the song.
     *
     * @return The tempo in beats per minute.
     * @ghidraAddress NTSC-U/C: 0x0027d510
     * @ghidraAddress PAL: 0x00286e28
     */
    float GetBpm() const;

    /**
     * Report the `tunnel_scale` of the song.
     *
     * @return The scale, or 1 when the entry does not specify one.
     * @ghidraAddress NTSC-U/C: 0x0027cea0
     * @ghidraAddress PAL: 0x002867b8
     */
    float GetTunnelScale() const;

    /**
     * Report the `song_bars` of the song.
     *
     * @return The number of bars.
     * @ghidraAddress NTSC-U/C: 0x0027d448
     * @ghidraAddress PAL: 0x00286d60
     */
    int GetBars() const;

    /**
     * Report the `remix_song_bars` of the song.
     *
     * @return The number of bars of a remix.
     * @ghidraAddress NTSC-U/C: 0x0027d478
     * @ghidraAddress PAL: 0x00286d90
     */
    int GetRemixBars() const;

    /**
     * Report the `duel_song_bars` of the song, or the `remix_song_bars` when the entry does not
     * specify it.
     *
     * @return The number of bars of a duel.
     * @ghidraAddress NTSC-U/C: 0x0027d4a8
     * @ghidraAddress PAL: 0x00286dc0
     */
    int GetDuelBars() const;

    /**
     * Report the short title of the song.
     *
     * The title is the localized `<name>_TITLE_SHORT`, or the full title when the text table has
     * no short title.
     *
     * @return The title.
     * @ghidraAddress NTSC-U/C: 0x0027cf90
     * @ghidraAddress PAL: 0x002868a8
     */
    const char *GetTitleShort() const;

    /**
     * Report the `arena_path` of the entry.
     *
     * The name is inferred.
     *
     * @return The path.
     * @ghidraAddress NTSC-U/C: 0x0027ce70
     * @ghidraAddress PAL: 0x00286788
     */
    const char *GetArenaPath() const;

    /**
     * Report whether the song is available at a skill level, which its `skill` sets the lowest
     * of.
     *
     * The name is inferred.
     *
     * @param nSkillLevel The skill level.
     * @return Whether `skill` is at most nSkillLevel.
     * @ghidraAddress NTSC-U/C: 0x0027d358
     * @ghidraAddress PAL: 0x00286c70
     */
    bool IsAvailable(int nSkillLevel) const;

    /**
     * Report whether a duel can play the song, from its `duel`.
     *
     * The name is inferred.
     *
     * @return Whether `duel` is non-zero.
     * @ghidraAddress NTSC-U/C: 0x0027d3a0
     * @ghidraAddress PAL: 0x00286cb8
     */
    bool IsDuel() const;

    /**
     * Report whether a remix can be made of the song, from its `not_remix`.
     *
     * The name is inferred.
     *
     * @return Whether `not_remix` is zero.
     * @ghidraAddress NTSC-U/C: 0x0027d3d8
     * @ghidraAddress PAL: 0x00286cf0
     */
    bool IsRemixable() const;

    /**
     * Report whether a game can play the song, from its `not_game`.
     *
     * The name is inferred.
     *
     * @return Whether `not_game` is zero.
     * @ghidraAddress NTSC-U/C: 0x0027d410
     * @ghidraAddress PAL: 0x00286d28
     */
    bool IsGameable() const;

    /**
     * Find a song in a list by name.
     *
     * The name is inferred.
     *
     * @param songs The list.
     * @param pszName The name of the song.
     * @return The entry of the song, or null when the list does not include it.
     * @ghidraAddress NTSC-U/C: 0x0027d540
     * @ghidraAddress PAL: 0x00286e58
     */
    static DataArray *Find(const std::vector<SongEntry> &songs, const char *pszName);

    /** @param pData The entry. */
    void SetData(DataArray *pData) {
        mData = pData;
    }

private:
    DataArray *mData; /*!< The entry. */
};

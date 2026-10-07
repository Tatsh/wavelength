#pragma once

#include "script/dataarray.h"

/**
 * View of the entry of one song in the "songs" section of the configuration.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred. The object is the one
 * pointer to the entry. Only the members GameLogic uses are declared.
 */
class SongEntry {
public:
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
     * Report the localised artist, the token `<name>_ARTIST`.
     *
     * @return The artist.
     * @ghidraAddress NTSC-U/C: 0x0027d090
     * @ghidraAddress PAL: 0x002869a8
     */
    const char *GetArtist() const;

    /**
     * Report the localised short form of the artist, the token `<name>_ARTIST_SHORT`.
     *
     * @return The short artist.
     * @ghidraAddress NTSC-U/C: 0x0027d0d8
     * @ghidraAddress PAL: 0x002869f0
     */
    const char *GetArtistShort() const;

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

    DataArray *mData; /*!< The entry. */
};

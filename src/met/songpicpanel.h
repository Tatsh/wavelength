#pragma once

#include "met/freqpanel.h"
#include "os/filepath.h"
#include "rnd/mesh.h"
#include "script/dataarray.h"

/**
 * Panel that shows the picture of a band or of an arena on the mesh `<name>.mesh`.
 *
 * The RTTI records the class as deriving from FreqPanel. The object is 0x110 bytes and its vtable
 * is at `0x003cc838`. The metagame registers the class for the panel type `song_pic_panel`, and
 * the song screen's `s_g_sel_song_pic` panel is one. A picture is a texture file that loads in the
 * background. Poll() places it on the mesh once it has loaded. The destructor at `0x00357140` is
 * compiler-generated and has no declaration here.
 */
class SongPicPanel : public FreqPanel {
public:
    /**
     * Construct a panel from its script description, with no picture.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @ghidraAddress NTSC-U/C: 0x0016f338
     * @ghidraAddress PAL: 0x00172588
     */
    SongPicPanel(DataArray *pData, const char *pszDir);

    /**
     * Create a panel from its script description.
     *
     * The metagame registers the routine for the panel type `song_pic_panel`.
     *
     * @param pData The script description.
     * @param pszDir The directory of the description file.
     * @return The new panel.
     * @ghidraAddress NTSC-U/C: 0x003571e8
     * @ghidraAddress PAL: 0x003c4448
     */
    static UIPanel *New(DataArray *pData, const char *pszDir) {
        return new SongPicPanel(pData, pszDir);
    }

    /**
     * Start the exit and release the picture.
     *
     * @param bForce Whether the panel skips its animation.
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0016f860
     * @ghidraAddress PAL: 0x00172b68
     */
    void Exit(bool bForce, float fTime) override;

    /**
     * Advance the panel, and place a picture that finished loading on the picture mesh.
     *
     * @param fTime The front-end time in milliseconds.
     * @ghidraAddress NTSC-U/C: 0x0016f668
     * @ghidraAddress PAL: 0x00172970
     * @stub
     */
    void Poll(float fTime) override;

    /**
     * Finish the load, and find and hide the picture mesh `<name>.mesh`.
     *
     * @ghidraAddress NTSC-U/C: 0x0016f3b8
     * @ghidraAddress PAL: 0x00172608
     */
    void FinishLoad() override;

    /**
     * Start loading the picture of a song's band.
     *
     * The picture is `Songs\<song>\<song>_encrypt.bmp` for an encrypted song,
     * `Songs\<song>\<song>_band_sm.bmp` for the small picture, and `Songs\<song>\<song>_band.bmp`
     * otherwise.
     *
     * @param pszSong The song.
     * @param bSmall Whether the small picture loads.
     * @param bUnlocked Whether the song is unlocked.
     * @param bEncrypted Whether the song is locked and hidden.
     * @ghidraAddress NTSC-U/C: 0x0016f458
     */
    void SetBandPicture(const char *pszSong, bool bSmall, bool bUnlocked, bool bEncrypted);

    /**
     * Start loading the picture of an arena, `Metagame\image\<arena>_band.bmp`.
     *
     * @param pszArena The arena.
     * @ghidraAddress NTSC-U/C: 0x0016f550
     */
    void SetArenaPicture(const char *pszArena);

    /**
     * Show or hide the picture mesh.
     *
     * A shown picture appears only once the panel is loaded and the picture is placed.
     *
     * @param bShowing Whether the picture shows.
     * @ghidraAddress NTSC-U/C: 0x0016f5f8
     * @ghidraAddress PAL: 0x00172900
     */
    void SetPictureShowing(bool bShowing);

    FilePath mPicturePath;   /*!< The texture file of the picture, or an empty path. +0xe4 */
    Rnd::Mesh *mPictureMesh; /*!< The mesh `<name>.mesh` the picture shows on. */
    int mUnlocked;           /*!< Whether the band's song is unlocked. */
    const char *mSong;       /*!< The song of the band picture, or an empty string. */
    int mPictureShowing;     /*!< Whether the picture shows. */
    int mPictureReady;       /*!< Whether the loaded picture is placed on the mesh. */
    int mEncrypted;          /*!< Whether the band's song is locked and hidden. */
};

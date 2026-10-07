#pragma once

/**
 * The front end music: the tracks of the current menu song and the mixes that switch them on and
 * off on the bar.
 *
 * The class is not polymorphic and has no RTTI, so its name is inferred. Metagame builds one for
 * each visit to the front end. Only the members the metagame uses are declared.
 */
class MetaMusic {
public:
    /**
     * Build the music with no tracks and no mix.
     *
     * @ghidraAddress NTSC-U/C: 0x00169380
     * @ghidraAddress PAL: 0x0016c508
     */
    MetaMusic();

    /**
     * Stop the tracks and release them.
     *
     * @ghidraAddress NTSC-U/C: 0x00169698
     * @ghidraAddress PAL: 0x0016c820
     */
    ~MetaMusic();

    /**
     * Switch to a mix at the music's current position.
     *
     * @param nMix The mix, an index of the `mix` entry of the menu song.
     * @ghidraAddress NTSC-U/C: 0x001699c0
     * @ghidraAddress PAL: 0x0016cb48
     */
    void ChangeMix(int nMix);

    /**
     * Switch to a mix at a tick of the front end clock. The current mix is torn down first.
     *
     * @param nMix The mix, an index of the `mix` entry of the menu song.
     * @param nTick The tick the new mix starts at.
     * @ghidraAddress NTSC-U/C: 0x001699e0
     * @ghidraAddress PAL: 0x0016cb68
     */
    void SwitchMix(int nMix, int nTick);

    /**
     * Start the sound effect of the tube the camera flies through.
     *
     * @ghidraAddress NTSC-U/C: 0x00169a48
     * @ghidraAddress PAL: 0x0016cbd0
     */
    void EnterTube();

    /**
     * Stop the sound effect of the tube.
     *
     * @param bSpeedingUp Whether the front end runs faster than real time. The body does not read
     * it.
     * @ghidraAddress NTSC-U/C: 0x00169af0
     * @ghidraAddress PAL: 0x0016cc78
     */
    void ExitTube(bool bSpeedingUp);

    /**
     * Start the music at the last bar of the front end clock with a mix, at full volume.
     *
     * @param nMix The mix, an index of the `mix` entry of the menu song.
     * @ghidraAddress NTSC-U/C: 0x00169b98
     * @ghidraAddress PAL: 0x0016cd20
     */
    void Start(int nMix);

    /**
     * Fade every track out over one bar and stop the music after it.
     *
     * @ghidraAddress NTSC-U/C: 0x0016a028
     * @ghidraAddress PAL: 0x0016d1b0
     */
    void Stop();
};

#pragma once

/**
 * Build the tracks shared by every menu song from the `music_shared_midi_file` entry of the
 * metagame configuration.
 *
 * The routines of the shared music have no receiver and act on the file-level tracks and lag ramp
 * of their unit. No class provides them. Each track of the file whose name is
 * `TRANSITION 1`, `TRANSITION 3`, or `TRANSITION 4` becomes a MixTrack. The names are inferred.
 *
 * @ghidraAddress NTSC-U/C: 0x00197068
 * @ghidraAddress PAL: 0x0019e588
 */
void LoadSharedMusic();

/**
 * Release the shared tracks and the lag ramp.
 *
 * @ghidraAddress NTSC-U/C: 0x00197118
 * @ghidraAddress PAL: 0x0019e638
 */
void UnloadSharedMusic();

/**
 * Fade each shared track in from the start of its loop over 1440 ticks, and move the lag of the
 * sound output to 100 milliseconds.
 *
 * @ghidraAddress NTSC-U/C: 0x001971f0
 * @ghidraAddress PAL: 0x0019e710
 */
void FadeInSharedMusic();

/**
 * Fade each shared track out over 1440 ticks, and move the lag of the sound output back to the
 * `lag_ms` entry of the `synth` section.
 *
 * @ghidraAddress NTSC-U/C: 0x001972c8
 * @ghidraAddress PAL: 0x0019e7e8
 */
void FadeOutSharedMusic();

/**
 * Report whether a shared track still plays, as one does until its fade out has ended.
 *
 * @return Whether a shared track plays.
 * @ghidraAddress NTSC-U/C: 0x001973c8
 * @ghidraAddress PAL: 0x0019e8e8
 */
bool IsSharedMusicFading();

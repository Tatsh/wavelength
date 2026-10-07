#pragma once

/**
 * Build the tracks shared by every menu song from the `music_shared_midi_file` entry of the
 * metagame configuration.
 *
 * The names in this header are inferred. The tracks are file-level state of their unit.
 *
 * @ghidraAddress NTSC-U/C: 0x00197068
 * @ghidraAddress PAL: 0x0019e588
 */
void LoadSharedMusic();

/**
 * Release the shared tracks.
 *
 * @ghidraAddress NTSC-U/C: 0x00197118
 * @ghidraAddress PAL: 0x0019e638
 */
void UnloadSharedMusic();

/**
 * Start the ramp of each shared track that brings it in, over 1440 ticks.
 *
 * @ghidraAddress NTSC-U/C: 0x001971f0
 * @ghidraAddress PAL: 0x0019e710
 */
void FadeInSharedMusic();

/**
 * Start the ramp of each shared track that takes it out, over 1440 ticks.
 *
 * @ghidraAddress NTSC-U/C: 0x001972c8
 * @ghidraAddress PAL: 0x0019e7e8
 */
void FadeOutSharedMusic();

/**
 * Report whether the ramp of a shared track still runs.
 *
 * @return Whether a ramp runs.
 * @ghidraAddress NTSC-U/C: 0x001973c8
 * @ghidraAddress PAL: 0x0019e8e8
 */
bool IsSharedMusicFading();

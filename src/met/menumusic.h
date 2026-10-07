#pragma once

/**
 * Fade out the tracks of the front-end music over three seconds.
 *
 * The routines of the front-end music have no receiver and act on the music's file-scope track
 * list. The name is inferred.
 *
 * @ghidraAddress NTSC-U/C: 0x001972c8
 * @ghidraAddress PAL: 0x0019e7e8
 */
void FadeOutMenuMusic();

/**
 * Report whether a track of the front-end music is still changing its volume.
 *
 * The name is inferred.
 *
 * @return True while a fade runs.
 * @ghidraAddress NTSC-U/C: 0x001973c8
 * @ghidraAddress PAL: 0x0019e8e8
 */
bool IsMenuMusicFading();

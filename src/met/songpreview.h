#pragma once

/**
 * Advance the preview clip the song screens play, fading its volume and starting or stopping its
 * stream as requested.
 *
 * The preview is file-level state of its unit, and the name is inferred.
 *
 * @ghidraAddress NTSC-U/C: 0x001967f0
 * @ghidraAddress PAL: 0x0019dd10
 */
void PollSongPreview();

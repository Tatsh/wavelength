#pragma once

#include "game/avatarpartset.h"
#include "game/remixinfo.h"
#include "game/songentry.h"
#include "os/string.h"
#include "rnd/font.h"
#include "rnd/mat.h"
#include "rnd/mesh.h"

/**
 * The `freq_winner.mat` material FindWinnerMaterial() keeps for the result screens.
 *
 * @ghidraAddress NTSC-U/C: 0x003af8b4
 */
extern Rnd::Mat *g_pWinnerMat;

/**
 * Find the `freq_winner.mat` material the result screens draw the winner's character with, and
 * keep it for them.
 *
 * The routine belongs to a unit of front end helpers that keeps its objects in file-level state.
 * The name is inferred.
 *
 * @ghidraAddress NTSC-U/C: 0x001993e0
 * @ghidraAddress PAL: 0x001a0900
 */
void FindWinnerMaterial();

/**
 * Find the font of a face in a player colour, `<face>_1_<colour>.font`.
 *
 * `purple` uses the `pink` font, and a colour other than `red`, `green`, `purple`, or `yellow`
 * uses the `white` font. The name is inferred.
 *
 * @param pszFace The face, such as `lucida`.
 * @param pszColor The colour, as GetPlayerColorName() reports it.
 * @return The font, or null.
 * @ghidraAddress NTSC-U/C: 0x00199450
 * @ghidraAddress PAL: 0x001a0970
 */
Rnd::Font *FindColorFont(const char *pszFace, const char *pszColor);

/**
 * Find the `lucida` font in a player colour.
 *
 * @param pszColor The colour, as GetPlayerColorName() reports it.
 * @return The font, or null.
 * @ghidraAddress NTSC-U/C: 0x001995a0
 * @ghidraAddress PAL: 0x001a0ac0
 */
Rnd::Font *FindLucidaFont(const char *pszColor);

/**
 * Find the material of a player colour, `fcolor_<n>.mat`.
 *
 * A colour other than `red`, `green`, `purple`, or `yellow` uses the material of `green`. The
 * name is inferred.
 *
 * @param pszColor The colour, as GetPlayerColorName() reports it.
 * @return The material, or null.
 * @ghidraAddress NTSC-U/C: 0x001995c8
 * @ghidraAddress PAL: 0x001a0ae8
 */
Rnd::Mat *FindColorMaterial(const char *pszColor);

/**
 * Report the colour of a player.
 *
 * @param nPlayer The player, from 0 to 3, or -1 for no player.
 * @return `green`, `purple`, `red`, or `yellow`, or `white` for no player.
 * @ghidraAddress NTSC-U/C: 0x001996f0
 * @ghidraAddress PAL: 0x001a0c10
 */
const char *GetPlayerColorName(int nPlayer);

/**
 * Report the localised name of a kind of network connection.
 *
 * @param nType The kind, 1 for a modem, 2 for broadband, and 3 for offline.
 * @return The name, or the name of no connection for another kind.
 * @ghidraAddress NTSC-U/C: 0x00199758
 * @ghidraAddress PAL: 0x001a0c78
 */
const char *GetConnectionTypeName(int nType);

/**
 * Report the localised abbreviation of a kind of network connection.
 *
 * @param nType The kind, as GetConnectionTypeName() takes it.
 * @return The abbreviation.
 * @ghidraAddress NTSC-U/C: 0x001997e8
 */
const char *GetConnectionTypeAbbrev(int nType);

/**
 * Report the localised abbreviation of a duel difficulty.
 *
 * @param nDifficulty The difficulty, 1 for easy, 2 for medium, and 3 for hard.
 * @return The abbreviation, or an empty string for another difficulty.
 * @ghidraAddress NTSC-U/C: 0x00199878
 * @ghidraAddress PAL: 0x001a0d08
 */
const char *GetDuelDifficultyAbbrev(int nDifficulty);

/**
 * Find the material of a rank, from `rank_01.mat` for rank -1 to `rank_05.mat` for rank 3.
 *
 * A rank outside that range uses `rank_01.mat`. The name is inferred.
 *
 * @param nRank The rank.
 * @return The material, or null.
 * @ghidraAddress NTSC-U/C: 0x00199910
 * @ghidraAddress PAL: 0x001a0e50
 */
Rnd::Mat *FindRankMaterial(int nRank);

/**
 * Find the material of a grade, `grade_0<grade>.mat`, or `grade_end_0<grade>.mat` for the end of
 * a song.
 *
 * The routine reads no object. The name is inferred.
 *
 * @param nGrade The grade.
 * @param bEnd Whether the material of the end of a song is found.
 * @return The material, or null.
 * @ghidraAddress NTSC-U/C: 0x00199a08
 * @ghidraAddress PAL: 0x001a0f48
 */
Rnd::Mat *FindGradeMaterial(int nGrade, bool bEnd);

/**
 * Render an avatar into the part of the screen a mesh covers, and draw the mesh with the winner's
 * material while the avatar plays a `win` animation.
 *
 * The routine belongs to the unit of front end helpers. The name is inferred.
 *
 * @param pAvatar The avatar.
 * @param pMesh The mesh whose bounding box sets the part of the screen.
 * @ghidraAddress NTSC-U/C: 0x00199ab8
 * @ghidraAddress PAL: 0x001a0ff8
 */
void DrawAvatarOnMesh(AvatarPartSet *pAvatar, Rnd::Mesh *pMesh);

/**
 * Remove the spaces at the start and at the end of a string.
 *
 * The routine belongs to the unit of front end helpers. The name is inferred.
 *
 * @param pText The string.
 * @return True when spaces were removed.
 * @ghidraAddress NTSC-U/C: 0x00199cb0
 * @ghidraAddress PAL: 0x001a11f0
 */
bool TrimSpaces(String *pText);

/**
 * Format the genre of a song and the tempo of a remix with the token `GENRE_BPM`.
 *
 * The name is inferred.
 *
 * @param song The song.
 * @param pInfo The remix.
 * @return The text, in the shared FormatString() buffer.
 * @ghidraAddress NTSC-U/C: 0x00199d90
 * @ghidraAddress PAL: 0x001a12d0
 */
const char *FormatGenreTempo(const SongEntry &song, const RemixInfo *pInfo);

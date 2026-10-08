#pragma once

/**
 * Move the camera that frames the avatar to one of the `cams` of the `avatar` entry of the "db"
 * section, blending from the current one.
 *
 * The routine works on the camera the avatar players share, not on an object. The name is
 * inferred.
 *
 * @param pszCam The camera, such as `f_maker` or `bust`.
 * @ghidraAddress NTSC-U/C: 0x00271440
 */
void SetAvatarCam(const char *pszCam);

/**
 * Turn the avatar in front of the camera the avatar players share.
 *
 * The name is inferred.
 *
 * @param fTilt The angle about the horizontal axis, in radians.
 * @param fTurn The angle about the vertical axis, in radians.
 * @ghidraAddress NTSC-U/C: 0x00270860
 * @ghidraAddress PAL: 0x0027a400
 */
void SetAvatarViewAngles(float fTilt, float fTurn);

/**
 * Move the camera that frames the avatar to one of the `cams` at once, with no blend.
 *
 * The name is inferred.
 *
 * @param pszCam The camera, such as `bust`.
 * @ghidraAddress NTSC-U/C: 0x00271488
 * @ghidraAddress PAL: 0x0027b038
 */
void SnapAvatarCam(const char *pszCam);

/**
 * Choose the camera that frames the avatar for a size of the avatar display.
 *
 * The name is inferred.
 *
 * @param nFreqSize The size, one of GameOptions::FreqSize.
 * @param bSnap Whether to move the camera at once rather than blend.
 * @ghidraAddress NTSC-U/C: 0x00270a40
 * @ghidraAddress PAL: 0x0027a5e0
 */
void SetAvatarFreqSize(int nFreqSize, bool bSnap);

/**
 * Report whether the avatar display is shown at all.
 *
 * The name is inferred.
 *
 * @return Whether the size SetAvatarFreqSize() last chose is not GameOptions::kFreqSizeHidden.
 * @ghidraAddress NTSC-U/C: 0x00270ae8
 * @ghidraAddress PAL: 0x0027a688
 */
bool IsAvatarShown();

/**
 * Report the depth range the camera that frames the avatar draws into.
 *
 * The name is inferred.
 *
 * @param pfNear Receives the near end of the range.
 * @param pfFar Receives the far end of the range.
 * @ghidraAddress NTSC-U/C: 0x00270810
 * @ghidraAddress PAL: 0x0027a3b0
 */
void GetAvatarDepthRange(float *pfNear, float *pfFar);

/**
 * Set the depth range the camera that frames the avatar draws into.
 *
 * The name is inferred.
 *
 * @param fNear The near end of the range.
 * @param fFar The far end of the range.
 * @ghidraAddress NTSC-U/C: 0x00270830
 * @ghidraAddress PAL: 0x0027a3d0
 */
void SetAvatarDepthRange(float fNear, float fFar);

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

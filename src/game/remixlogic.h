#pragma once

/**
 * Game rules of a remix session.
 *
 * The RTTI includes the class name and records WorldLogic as the base. This header declares only
 * the members a remix tutorial calls.
 */
class RemixLogic {
public:
    /**
     * Turn looping of a lane's track on or off.
     *
     * @param nLane The lane.
     * @param bLooping Loop the track.
     * @ghidraAddress NTSC-U/C: 0x00134ca0
     * @ghidraAddress PAL: 0x001364b0
     */
    void SetLooping(int nLane, bool bLooping);

    /**
     * Clear the gems of every section.
     *
     * @ghidraAddress NTSC-U/C: 0x00135510
     * @ghidraAddress PAL: 0x00136d10
     */
    void ClearGems();

    /**
     * Open the remix menu.
     *
     * @ghidraAddress NTSC-U/C: 0x001355c0
     * @ghidraAddress PAL: 0x00136dc0
     */
    void OpenMenu();

    /**
     * Open the tempo control.
     *
     * @ghidraAddress NTSC-U/C: 0x001355e0
     * @ghidraAddress PAL: 0x00136de0
     */
    void OpenTempo();

    /**
     * Close the tempo control.
     *
     * @ghidraAddress NTSC-U/C: 0x00135600
     * @ghidraAddress PAL: 0x00136e00
     */
    void CloseTempo();

    /**
     * Raise the tempo by one step.
     *
     * @ghidraAddress NTSC-U/C: 0x00135620
     * @ghidraAddress PAL: 0x00136e20
     */
    void RaiseTempo();

    /**
     * Lower the tempo by one step.
     *
     * @ghidraAddress NTSC-U/C: 0x00135648
     * @ghidraAddress PAL: 0x00136e48
     */
    void LowerTempo();
};

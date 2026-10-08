#pragma once

#include <vector>

#include "gfx/particlearmsassembly.h"
#include "gfx/tnlgeom.h"
#include "script/dataarray.h"

/**
 * The particle arms of the tunnel, and the `move_particle_arms` script command that moves them.
 *
 * The class is not polymorphic, and the name is inferred. The object is 0x14 bytes.
 */
class ParticleArms {
public:
    /**
     * Register the script command and create the arms.
     *
     * @param pData The configuration, a tag followed by one entry for each assembly.
     * @param pGeom The geometry of the tunnel.
     * @ghidraAddress NTSC-U/C: 0x001f6fd0
     * @ghidraAddress PAL: 0x001ffd70
     */
    ParticleArms(DataArray *pData, TnlGeom *pGeom);

    /**
     * Unregister the script command and delete the arms.
     *
     * @ghidraAddress NTSC-U/C: 0x001f7188
     * @ghidraAddress PAL: 0x001fff28
     */
    ~ParticleArms();

    /**
     * Load each assembly from its entry.
     *
     * @param pData The configuration, a tag followed by one entry for each assembly.
     * @ghidraAddress NTSC-U/C: 0x001f70f0
     * @ghidraAddress PAL: 0x001ffe90
     */
    void LoadConfig(DataArray *pData);

    /**
     * Advance each assembly to a tick.
     *
     * @param flTick The tick of the song.
     * @ghidraAddress NTSC-U/C: 0x001f7258
     * @ghidraAddress PAL: 0x001ffff8
     */
    void Poll(float flTick);

    /**
     * Report the use of the particles of each assembly through the debug output.
     *
     * @ghidraAddress NTSC-U/C: 0x001f72e0
     * @ghidraAddress PAL: 0x00200080
     */
    void Report();

    std::vector<ParticleArmsAssembly *> mAssemblies; /*!< The assemblies. */
    TnlGeom *mGeom;                                  /*!< The geometry of the tunnel. */

private:
    /**
     * The `move_particle_arms` command, which moves the assembly its first argument names.
     *
     * The command is `move_particle_arms <name> <from ticks> <to ticks> <length>`.
     *
     * @param pCommand The command.
     * @param pUserData The arms.
     * @ghidraAddress NTSC-U/C: 0x001f7358
     * @ghidraAddress PAL: 0x002000f8
     */
    static void MoveCommand(DataArray *pCommand, void *pUserData);
};

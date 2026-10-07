#pragma once

#include <vector>

#include "mid/tickobj.h"
#include "os/string.h"
#include "script/dataarray.h"

/**
 * The script commands of a song's SCRIPT track, each at its tick.
 *
 * The class is not polymorphic and has no RTTI. The name is inferred.
 */
class ScriptTrackData {
public:
    /**
     * Construct data with no command.
     *
     * @param pszName The name of the data.
     * @ghidraAddress NTSC-U/C: 0x00138c58
     * @ghidraAddress PAL: 0x0013a4b8
     */
    explicit ScriptTrackData(const char *pszName);

    /**
     * Release every command.
     *
     * @ghidraAddress NTSC-U/C: 0x00138c90
     * @ghidraAddress PAL: 0x0013a4f0
     */
    ~ScriptTrackData();

    /**
     * Add a command after every command at or before its tick.
     *
     * @param nTick The tick.
     * @param pCommand The command, whose reference the data takes over.
     * @ghidraAddress NTSC-U/C: 0x00138d78
     * @ghidraAddress PAL: 0x0013a5d8
     */
    void AddCommand(int nTick, DataArray *pCommand);

    /**
     * Report the number of commands.
     *
     * @return The number of commands.
     * @ghidraAddress NTSC-U/C: 0x00139030
     * @ghidraAddress PAL: 0x0013a890
     */
    int NumCommands() const;

    /**
     * Report a command and its tick.
     *
     * @param nIndex The command index.
     * @return The command and its tick.
     * @ghidraAddress NTSC-U/C: 0x00139048
     * @ghidraAddress PAL: 0x0013a8a8
     */
    TickObj<DataArray *> *GetCommand(int nIndex);

    String mName;                                 /*!< The name of the data. */
    std::vector<TickObj<DataArray *> > mCommands; /*!< The commands in ascending tick order. */
};

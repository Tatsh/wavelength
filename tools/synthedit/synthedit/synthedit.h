#pragma once

#include <afxctl.h>

/**
 * The control module.
 *
 * The class name is from the runtime class information. The compiler generates the constructor
 * (0x10002120) and the destructor (0x10002100).
 */
class CSyntheditApp : public COleControlModule {
public:
    /**
     * Start the module.
     *
     * @return Whether it started.
     * @ghidraAddress 0x10001fdf
     */
    BOOL InitInstance();

    /**
     * Stop the module.
     *
     * @return The exit code.
     * @ghidraAddress 0x10001ffa
     */
    int ExitInstance();
};

/** The type library. */
extern const GUID CDECL _tlid;

/** The major version of the type library. */
extern const WORD _wVerMajor;

/** The minor version of the type library. */
extern const WORD _wVerMinor;

#pragma once

#include <map>

#include "os/Debug.h"
#include "synth/BankWin.h"
#include "utl/Str.h"

/**
 * Look up an object of an identifier map, failing when it is missing.
 *
 * @param iMap The map.
 * @param iKey The identifier.
 * @return The object.
 * @ghidraAddress 0x10007ef0
 * @ghidraAddress 0x10007f90
 * @ghidraAddress 0x100080a0
 * @ghidraAddress 0x10024aa0
 */
template <class T>
T *GetFromMap(std::map<int, T *> &iMap, int iKey) {
    typename std::map<int, T *>::iterator it = iMap.find(iKey);
    ASSERT(it != iMap.end());
    ASSERT(it->second != NULL);
    return it->second;
}

/**
 * Owner of every bank open in the editor.
 *
 * The class has no RTTI, and its name comes from its source file. The object is the map of banks.
 * The control has the one instance.
 */
class BankManager {
public:
    /**
     * Create a manager with no bank.
     *
     * @ghidraAddress 0x1001c455
     */
    BankManager();

    /**
     * Delete every bank.
     *
     * @ghidraAddress 0x1001c4a0
     */
    ~BankManager();

    /**
     * Delete a bank.
     *
     * @param bankID The identifier of the BankWin.
     * @return Zero.
     * @ghidraAddress 0x1001d909
     */
    int RetireBank(int bankID);

    /**
     * Open a bank, loading it when a file is given. A file already open is refused with a
     * notice.
     *
     * @param filename The path of the bank file, or an empty string for a new bank.
     * @param bankID The bank identifier a bank select requests.
     * @return The identifier of the BankWin, or -1 when the file is already open.
     * @ghidraAddress 0x1001d928
     */
    int NewBank(const String &filename, unsigned short bankID);

    /**
     * Look up a bank.
     *
     * @param bankID The identifier of the BankWin.
     * @return The bank.
     * @ghidraAddress 0x1001f3bd
     */
    BankWin *GetBankWin(int bankID);

private:
    std::map<int, BankWin *> mBanks; /*!< Open banks by identifier. */
};

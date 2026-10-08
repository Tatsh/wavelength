#include "utl/DataFunc.h"

#include <vector>

#include "os/Debug.h"

namespace {

// The node of a command that holds its name.
const int kCommandName = 0;

} // namespace

// 0x100c3db8
static std::vector<FuncDesc> gDataFuncs;

void DataRegisterFunc(DataFunc func, const char *name, void *data) {
    const FuncDesc desc = { String(name), func, data };
    gDataFuncs.push_back(desc);
}

bool DataExecute(DataArray *args) {
    const char *name = args->Sym(kCommandName);
    std::vector<FuncDesc>::const_iterator it;
    for (it = gDataFuncs.begin(); it != gDataFuncs.end(); ++it) {
        const FuncDesc &desc = *it;
        if (desc.mName == name) {
            desc.mFunc(args, desc.mData);
            return true;
        }
    }
    TheDebug.Printf("Error: function %s has not been registered\n", name);
    return false;
}

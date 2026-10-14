#include "game/showingcondition.h"

#include "os/debug.h"
#include "rnd/rndmanager.h"

ShowingCondition::ShowingCondition(DataArray *pCondition) {
    mDrawable = dynamic_cast<RndDrawable *>(TheManager.Find(pCondition->Sym(1)));
    if (mDrawable == nullptr) {
        DebugWarn("Couldn't find '%s' object (file %s, line %d)",
                  pCondition->Sym(1),
                  pCondition->mFile,
                  pCondition->mLine);
    }
}

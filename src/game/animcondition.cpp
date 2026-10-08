#include "game/animcondition.h"

#include "os/debug.h"
#include "rnd/manager.h"

AnimCondition::AnimCondition(DataArray *pCondition) {
    mAnim = dynamic_cast<Rnd::Animatable *>(Rnd::TheManager.Find(pCondition->Sym(1)));
    if (mAnim == nullptr) {
        DebugWarn("Couldn't find '%s' object (file %s, line %d)",
                  pCondition->Sym(1),
                  pCondition->mFile,
                  pCondition->mLine);
    }
    mFrame = pCondition->Float(2);
}

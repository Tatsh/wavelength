#include "game/animcondition.h"

#include "os/debug.h"
#include "rnd/rndmanager.h"

AnimCondition::AnimCondition(DataArray *pCondition) {
    mAnim = dynamic_cast<RndAnimatable *>(TheManager.Find(pCondition->Sym(1)));
    if (mAnim == nullptr) {
        DebugWarn("Couldn't find '%s' object (file %s, line %d)",
                  pCondition->Sym(1),
                  pCondition->GetFile(),
                  pCondition->GetLine());
    }
    mFrame = pCondition->Float(2);
}

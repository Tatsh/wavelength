#include "app/ovyassembly.h"

#include "os/hxstr.h"
#include "rnd/rndanimatable.h"
#include "rnd/rndmanager.h"

OvyAssembly::OvyAssembly(const HxStr &name, float flFrom, float flTo, float flDuration) {
    mAnim = dynamic_cast<RndAnimatable *>(TheManager.Find(name.mStr));
    mAnim->SetFrame(0.0f);
    mRamp.SetParams(flFrom, flTo, flDuration);
}

void OvyAssembly::SetTarget(float flTarget) {
    mRamp.SetTarget(flTarget);
}

void OvyAssembly::Jump(float flTarget) {
    mRamp.Jump(flTarget);
}

void OvyAssembly::Execute(float flTime) {
    if (mRamp.Execute(flTime) != 0) {
        mAnim->SetFrame(mRamp.Val());
    }
}

#include "gfx/spritegroup.h"

#include "os/string.h"
#include "rnd/rndmanager.h"

namespace {

// The pool takes this many particles beyond the most gems placed at once.
constexpr int kSpareCount = 64;

} // namespace

SpriteGroup::SpriteGroup(const char *pszName, int nMaxGems) {
    mParticles = dynamic_cast<RndParticleSys *>(TheManager.Find(FormatString("%s.ps", pszName)));
    mParticles->FreeAllParticles();
    mParticles->mParticlesOwner->SetNumParticles(nMaxGems + kSpareCount);
}

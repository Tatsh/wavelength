#include "gfx/particlearms.h"

#include <cstring>

#include "os/debug.h"
#include "script/scriptfunction.h"

namespace {

const char *const kMoveCommandName = "move_particle_arms";

// The arguments of the move command after its name.
enum MoveArgument {
    kMoveArgumentName = 1,
    kMoveArgumentFromTicks = 2,
    kMoveArgumentToTicks = 3,
    kMoveArgumentTicks = 4,
};

} // namespace

ParticleArms::ParticleArms(DataArray *pData, TnlGeom *pGeom) : mGeom(pGeom) {
    ScriptFunction::Register(MoveCommand, kMoveCommandName, this);
    mAssemblies.resize(pData->mSize - 1, nullptr);
    for (int i = 1; i < pData->mSize; ++i) {
        mAssemblies[i - 1] = new ParticleArmsAssembly(pData->Array(i));
    }
}

ParticleArms::~ParticleArms() {
    ScriptFunction::Unregister(MoveCommand);
    for (unsigned i = 0; i < mAssemblies.size(); ++i) {
        delete mAssemblies[i];
    }
}

void ParticleArms::LoadConfig(DataArray *pData) {
    for (int i = 1; i < pData->mSize; ++i) {
        mAssemblies[i - 1]->LoadConfig(pData->Array(i));
    }
}

void ParticleArms::Poll(float flTick) {
    for (unsigned i = 0; i < mAssemblies.size(); ++i) {
        mAssemblies[i]->Poll(mGeom, flTick);
    }
}

void ParticleArms::Report() {
    for (unsigned i = 0; i < mAssemblies.size(); ++i) {
        mAssemblies[i]->Report();
    }
}

void ParticleArms::MoveCommand(DataArray *pCommand, void *pUserData) {
    const ParticleArms *pArms = static_cast<ParticleArms *>(pUserData);
    const char *pszName = pCommand->Sym(kMoveArgumentName);
    for (unsigned i = 0; i < pArms->mAssemblies.size(); ++i) {
        if (std::strcmp(pszName, pArms->mAssemblies[i]->Name()) == 0) {
            const float flFromTicks = pCommand->Float(kMoveArgumentFromTicks);
            const float flToTicks = pCommand->Float(kMoveArgumentToTicks);
            pArms->mAssemblies[i]->Move(
                flFromTicks, flToTicks, pCommand->Float(kMoveArgumentTicks));
            return;
        }
    }
    DebugWarn("could not find particle arm %s", pszName);
}

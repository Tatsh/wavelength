#include "game/triggeraction.h"

#include <cstring>

#include "game/animateaction.h"
#include "game/animatetoaction.h"
#include "game/animchildaction.h"
#include "game/avatarbodyanimateaction.h"
#include "game/avatargemcatchaction.h"
#include "game/avatarinstrumentaction.h"
#include "game/backgroundaction.h"
#include "game/bluraction.h"
#include "game/changelyricaction.h"
#include "game/changescreenaction.h"
#include "game/datafuncaction.h"
#include "game/generateaction.h"
#include "game/setanimaction.h"
#include "game/setbluraction.h"
#include "game/setcamaction.h"
#include "game/setenvaction.h"
#include "game/setgeneratoraction.h"
#include "game/setmataction.h"
#include "game/setmeshaction.h"
#include "game/setparticlesaction.h"
#include "game/showaction.h"
#include "game/triggermgr.h"
#include "os/debug.h"

TriggerAction *TriggerAction::New(DataArray *pAction) {
    const char *pszName = pAction->Sym(0);
    TriggerAction *pNew = nullptr;
    if (std::strcmp(pszName, "avatar_gem_catch") == 0) {
        pNew = new AvatarGemCatchAction(pAction);
    } else if (std::strcmp(pszName, "avatar_instrument") == 0) {
        pNew = new AvatarInstrumentAction(pAction);
    } else if (std::strcmp(pszName, "avatar_body_animate") == 0) {
        pNew = new AvatarBodyAnimateAction(pAction);
    } else if (std::strcmp(pszName, "background") == 0) {
        pNew = new BackgroundAction(pAction);
    } else if (std::strcmp(pszName, "blur") == 0) {
        pNew = new BlurAction(pAction);
    } else if (std::strcmp(pszName, "set_anim") == 0) {
        pNew = new SetAnimAction(pAction);
    } else if (std::strcmp(pszName, "set_mesh") == 0) {
        pNew = new SetMeshAction(pAction);
    } else if (std::strcmp(pszName, "set_env") == 0) {
        pNew = new SetEnvAction(pAction);
    } else if (std::strcmp(pszName, "set_cam") == 0) {
        pNew = new SetCamAction(pAction);
    } else if (std::strcmp(pszName, "set_mat") == 0) {
        pNew = new SetMatAction(pAction);
    } else if (std::strcmp(pszName, "set_blur") == 0) {
        pNew = new SetBlurAction(pAction);
    } else if (std::strcmp(pszName, "set_generator") == 0) {
        pNew = new SetGeneratorAction(pAction);
    } else if (std::strcmp(pszName, "set_particles") == 0) {
        pNew = new SetParticlesAction(pAction);
    } else if (std::strcmp(pszName, "generate") == 0) {
        pNew = new GenerateAction(pAction);
    } else if (std::strcmp(pszName, "animate") == 0) {
        pNew = new AnimateAction(pAction);
    } else if (std::strcmp(pszName, "animate_to") == 0) {
        pNew = new AnimateToAction(pAction);
    } else if (std::strcmp(pszName, "add_anim") == 0) {
        pNew = new AnimChildAction(pAction, true);
    } else if (std::strcmp(pszName, "remove_anim") == 0) {
        pNew = new AnimChildAction(pAction, false);
    } else if (std::strcmp(pszName, "show") == 0) {
        pNew = new ShowAction(pAction, true);
    } else if (std::strcmp(pszName, "hide") == 0) {
        pNew = new ShowAction(pAction, false);
    } else if (std::strcmp(pszName, "change_screen") == 0) {
        pNew = new ChangeScreenAction(pAction);
    } else if (std::strcmp(pszName, "change_lyric") == 0) {
        pNew = new ChangeLyricAction(pAction);
    } else {
        pNew = new DataFuncAction(pAction);
    }
    if (pNew == nullptr) {
        DebugWarn("Don't recognize %s action (file %s, line %d)",
                  pszName,
                  pAction->mFile,
                  pAction->mLine);
    }
    return pNew;
}

void TriggerAction::Start() {
    TheTriggerMgr.mClock = mClock;
    if (mDelay <= 0.0f) {
        Exec();
    } else {
        TheTriggerMgr.Schedule(this, mDelay);
    }
}

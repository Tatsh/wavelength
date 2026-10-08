#include "game/triggercondition.h"

#include <cstring>

#include "game/animcondition.h"
#include "game/barcondition.h"
#include "game/buttoncondition.h"
#include "game/capturedcondition.h"
#include "game/componentcondition.h"
#include "game/distancecondition.h"
#include "game/dyingcondition.h"
#include "game/gamestatecondition.h"
#include "game/gemposcondition.h"
#include "game/healthcondition.h"
#include "game/instrumentcondition.h"
#include "game/leadercondition.h"
#include "game/musicmodecondition.h"
#include "game/notecondition.h"
#include "game/oldcomponentcondition.h"
#include "game/oldpanelcondition.h"
#include "game/oldscreencondition.h"
#include "game/panelcondition.h"
#include "game/pathalreadyunlockedcondition.h"
#include "game/pathexistscondition.h"
#include "game/playercondition.h"
#include "game/powerupcondition.h"
#include "game/randomcondition.h"
#include "game/rulesetcondition.h"
#include "game/scorecondition.h"
#include "game/screencondition.h"
#include "game/showingcondition.h"
#include "game/streakcondition.h"
#include "game/timecondition.h"
#include "game/trackcondition.h"
#include "os/debug.h"

TriggerCondition *TriggerCondition::New(DataArray *pCondition) {
    const char *pszName = pCondition->Sym(0);
    TriggerCondition *pNew = nullptr;
    if (std::strcmp(pszName, "note") == 0) {
        pNew = new NoteCondition(pCondition);
    } else if (std::strcmp(pszName, "showing") == 0) {
        pNew = new ShowingCondition(pCondition);
    } else if (std::strcmp(pszName, "random") == 0) {
        pNew = new RandomCondition(pCondition);
    } else if (std::strcmp(pszName, "captured") == 0) {
        pNew = new CapturedCondition();
    } else if (std::strcmp(pszName, "component") == 0) {
        pNew = new ComponentCondition(pCondition);
    } else if (std::strcmp(pszName, "old_component") == 0) {
        pNew = new OldComponentCondition(pCondition);
    } else if (std::strcmp(pszName, "panel") == 0) {
        pNew = new PanelCondition(pCondition);
    } else if (std::strcmp(pszName, "old_panel") == 0) {
        pNew = new OldPanelCondition(pCondition);
    } else if (std::strcmp(pszName, "screen") == 0) {
        pNew = new ScreenCondition(pCondition);
    } else if (std::strcmp(pszName, "old_screen") == 0) {
        pNew = new OldScreenCondition(pCondition);
    } else if (std::strcmp(pszName, "distance") == 0) {
        pNew = new DistanceCondition(pCondition);
    } else if (std::strcmp(pszName, "anim") == 0) {
        pNew = new AnimCondition(pCondition);
    } else if (std::strcmp(pszName, "streak") == 0) {
        pNew = new StreakCondition(pCondition);
    } else if (std::strcmp(pszName, "gem_pos") == 0) {
        pNew = new GemPosCondition(pCondition);
    } else if (std::strcmp(pszName, "music_mode") == 0) {
        pNew = new MusicModeCondition(pCondition);
    } else if (std::strcmp(pszName, "instrument") == 0) {
        pNew = new InstrumentCondition(pCondition);
    } else if (std::strcmp(pszName, "button") == 0) {
        pNew = new ButtonCondition(pCondition);
    } else if (std::strcmp(pszName, "player") == 0) {
        pNew = new PlayerCondition(pCondition);
    } else if (std::strcmp(pszName, "bar") == 0) {
        pNew = new BarCondition(pCondition);
    } else if (std::strcmp(pszName, "time") == 0) {
        pNew = new TimeCondition(pCondition);
    } else if (std::strcmp(pszName, "score") == 0) {
        pNew = new ScoreCondition(pCondition);
    } else if (std::strcmp(pszName, "track") == 0) {
        pNew = new TrackCondition(pCondition);
    } else if (std::strcmp(pszName, "powerup") == 0) {
        pNew = new PowerupCondition(pCondition);
    } else if (std::strcmp(pszName, "leader") == 0) {
        pNew = new LeaderCondition();
    } else if (std::strcmp(pszName, "health") == 0) {
        pNew = new HealthCondition(pCondition);
    } else if (std::strcmp(pszName, "dying") == 0) {
        pNew = new DyingCondition();
    } else if (std::strcmp(pszName, "game_state") == 0) {
        pNew = new GameStateCondition(pCondition);
    } else if (std::strcmp(pszName, "rule_set") == 0) {
        pNew = new RuleSetCondition(pCondition);
    } else if (std::strcmp(pszName, "path_already_unlocked") == 0) {
        pNew = new PathAlreadyUnlockedCondition(pCondition);
    } else if (std::strcmp(pszName, "path_exists") == 0) {
        pNew = new PathExistsCondition(pCondition);
    }
    if (pNew == nullptr) {
        DebugWarn("Don't recognize %s condition (file %s, line %d)",
                  pszName,
                  pCondition->mFile,
                  pCondition->mLine);
    }
    return pNew;
}

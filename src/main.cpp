#include "game/gamedb.h"
#include "game/triggermgr.h"
#include "game/worldmgr.h"
#include "met/metagame.h"
#include "netflow/net.h"
#include "os/debug.h"
#include "os/joypad.h"
#include "os/locale.h"
#include "os/mem.h"
#include "os/system.h"
#include "rnd/manager.h"
#include "rnd/object.h"
#include "rnd/rndrenderer.h"
#include "script/scriptfunction.h"
#include "synth/synth.h"

namespace {

constexpr char kConfigFile[] = "freq2_config.txt";
constexpr char kExitAppCommand[] = "exit_app";
constexpr char kRndHeapName[] = "rnd";

// Time the pending renderer loads may take each frame.
constexpr float kLoaderBudgetMs = 10.0f;

// Bytes the renderer heap compaction may move each frame. A world that has finished loading gets
// the smaller budget, and a world that has been unloaded restores the larger one.
constexpr int kDefaultRndHeapCompactBytes = 100000;
constexpr int kLoadedRndHeapCompactBytes = 40000;

// NTSC-U/C: 0x003ae480
bool g_bQuitRequested = false;

// NTSC-U/C: 0x003ae484
int g_nRndHeapCompactBytes = kDefaultRndHeapCompactBytes;

// NTSC-U/C: 0x003ae488
bool g_bRndHeapCompactStrict = false;

// NTSC-U/C: 0x00100200, PAL: 0x00100200
void CheckLeftoverObjects() {
    bool bLeftover = false;
    for (const auto &entry : Rnd::TheManager.mObjects) {
        if (entry.second->mInternal == 0) {
            bLeftover = true;
            break;
        }
    }
    if (!bLeftover) {
        return;
    }

    TheDebug << "Freq2 main: Flushing " << static_cast<int>(Rnd::TheManager.mObjects.size())
             << " objects.\n";
    TheDebug << "THIS IS BAD.  THERE SHOULD BE NONE,\n"
             << "aside from internal default objects,\n"
             << "such as \"[default cam\"].\nLeftovers:\n    " << Rnd::TheManager.mObjects << "\n";
    Rnd::TheManager.DeleteLoadedObjects();
}

// NTSC-U/C: 0x00100318, PAL: 0x00100318
void QuitApp([[maybe_unused]] DataArray *pCommand, [[maybe_unused]] void *pUserData) {
    g_bQuitRequested = true;
}

// NTSC-U/C: 0x00100328, PAL: 0x00100328
void HandleMetagameFrontEnd(int nMetagameEvent) {
    (void)TheWorldMgr->GetState(); // Yes, the binary discards this call's result.
    if (nMetagameEvent == Metagame::kEventStartGame) {
        TheMetagame.EnterLoading();
        (void)SystemMs(); // Yes, the binary discards the elapsed time.
        TheRnd->BeginFrame();
        TheRnd->EndFrame();
        TheWorldMgr->Load();
    } else if (nMetagameEvent == Metagame::kEventQuit) {
        g_bQuitRequested = true;
    }
}

// NTSC-U/C: 0x00100408, PAL: 0x00100408
void HandleMetagameLoading([[maybe_unused]] int nMetagameEvent) {
    (void)TheWorldMgr->GetState(); // Yes, the binary discards this call's result.
}

// NTSC-U/C: 0x00100430, PAL: 0x00100430
void HandleMetagamePlaying(int nMetagameEvent) {
    // A world in any other state is queried a second time and the answer discarded.
    if (TheWorldMgr->GetState() != WorldMgr::kStatePlaying) {
        (void)TheWorldMgr->GetState();
    }
    if (nMetagameEvent == Metagame::kEventQuit) {
        g_bQuitRequested = true;
    } else if (nMetagameEvent == Metagame::kEventLeaveGame) {
        TheWorldMgr->Unload();
        TheMetagame.EnterLeaving();
    }
}

// NTSC-U/C: 0x001004b0, PAL: 0x001004b0
void HandleMetagameLeaving([[maybe_unused]] int nMetagameEvent) {
    (void)TheWorldMgr->GetState(); // Yes, the binary discards this call's result.
}

// NTSC-U/C: 0x001004d8, PAL: 0x001004d8
void HandleMetagameRestarting([[maybe_unused]] int nMetagameEvent) {
    // A world in any other state is queried a second time and the answer discarded.
    if (TheWorldMgr->GetState() != WorldMgr::kStateLoading) {
        (void)TheWorldMgr->GetState();
    }
}

// NTSC-U/C: 0x00100678, PAL: 0x00100678
void HandleWorldLoading(int nWorldEvent) {
    // A metagame in any other state is queried a second time and the answer discarded.
    if (TheMetagame.GetState() != Metagame::kStateLoading) {
        (void)TheMetagame.GetState();
    }
    if (nWorldEvent == WorldMgr::kEventLoaded) {
        TheMetagame.EnterPlaying();
        TheWorldMgr->Start();
    }
}

// NTSC-U/C: 0x00100518, PAL: 0x00100518
void HandleWorldPlaying(int nWorldEvent) {
    // A metagame in any other state is queried a second time and the answer discarded.
    if (TheMetagame.GetState() != Metagame::kStatePlaying) {
        (void)TheMetagame.GetState();
    }
    if (nWorldEvent == WorldMgr::kEventFinished) {
        TheWorldMgr->Unload();
        TheRnd->BeginFrame();
        TheRnd->EndFrame();
        TheMetagame.EnterLeaving();
    } else if (nWorldEvent == WorldMgr::kEventRestart) {
        TheWorldMgr->Unload();
        TheMetagame.EnterRestarting();
    }
}

// NTSC-U/C: 0x001005e0, PAL: 0x001005e0
void HandleWorldUnloaded(int nWorldEvent, bool bReturnToFrontEnd) {
    if (TheMetagame.GetState() == Metagame::kStateRestarting) {
        if (nWorldEvent == WorldMgr::kEventUnloaded) {
            TheWorldMgr->Load();
        }
    } else if (bReturnToFrontEnd && nWorldEvent == WorldMgr::kEventUnloaded) {
        TheMetagame.EnterFrontEnd();
        TheWorldMgr->Reset();
    }
}

// NTSC-U/C: 0x001006e8, PAL: 0x001006e8
// The body is empty in the shipped build. The call is made all the same.
void TraceStateEvents([[maybe_unused]] int nMetagameEvent,
                      [[maybe_unused]] int nMetagameState,
                      [[maybe_unused]] int nWorldEvent,
                      [[maybe_unused]] int nWorldState) {
}

// NTSC-U/C: 0x001006f0, PAL: 0x001006f0
void DispatchStateEvents(int nMetagameEvent, int nWorldEvent) {
    const int nMetagameState = TheMetagame.GetState();
    const int nWorldState = TheWorldMgr->GetState();
    TraceStateEvents(nMetagameEvent, nMetagameState, nWorldEvent, nWorldState);

    switch (nMetagameState) {
    case Metagame::kStateFrontEnd:
        HandleMetagameFrontEnd(nMetagameEvent);
        break;
    case Metagame::kStateLoading:
        HandleMetagameLoading(nMetagameEvent);
        break;
    case Metagame::kStatePlaying:
        HandleMetagamePlaying(nMetagameEvent);
        break;
    case Metagame::kStateLeaving:
        HandleMetagameLeaving(nMetagameEvent);
        break;
    case Metagame::kStateRestarting:
        HandleMetagameRestarting(nMetagameEvent);
        break;
    default:
        break;
    }

    // The world handlers see the state read before the metagame handler ran.
    switch (nWorldState) {
    case WorldMgr::kStateLoading:
        HandleWorldLoading(nWorldEvent);
        break;
    case WorldMgr::kStatePlaying:
        HandleWorldPlaying(nWorldEvent);
        break;
    case WorldMgr::kStateUnloaded:
        HandleWorldUnloaded(nWorldEvent, nMetagameEvent == Metagame::kEventReturnToFrontEnd);
        break;
    default:
        break;
    }
}

// NTSC-U/C: 0x00100828, PAL: 0x00100828
void UpdateAppFrame() {
    TheWorldMgr->UpdateTime();
    TheSynth->UpdateTime();
    SystemPoll();
    Rnd::TheManager.PollLoaders(kLoaderBudgetMs);
    PollNetSubsystem();

    const int nMetagameEvent = TheMetagame.Update();
    const int nWorldEvent = TheWorldMgr->Poll();
    if (nWorldEvent == WorldMgr::kEventLoaded) {
        g_nRndHeapCompactBytes = kLoadedRndHeapCompactBytes;
        g_bRndHeapCompactStrict = true;
    } else if (nWorldEvent == WorldMgr::kEventUnloaded) {
        g_bRndHeapCompactStrict = false;
        g_nRndHeapCompactBytes = kDefaultRndHeapCompactBytes;
    }

    TheGameDb->Poll();
    TheSynth->Poll();

    TheRnd->BeginFrame();
    MemCompact(MemFindHeap(kRndHeapName), g_nRndHeapCompactBytes, g_bRndHeapCompactStrict);
    TheMetagame.Draw();
    TheWorldMgr->Draw();
    TheMetagame.DrawOverlay();
    TheRnd->EndFrame();

    DispatchStateEvents(nMetagameEvent, nWorldEvent);
}

// NTSC-U/C: 0x001009e0
void InitializeAppSubsystems(int argc, char **argv) {
    SystemInit(argc, argv, kConfigFile);
    Rnd::TheManager.Init();
    TheRnd->Init();
    TheMetagame.ShowLoadingScreen();
    Synth::Create();
    TheGameDb->Init();
    InitializeNetSubsystem();
    TheLocale.Init(GetSystemLanguage());
    TheTriggerMgr.Init();
    TheWorldMgr->Init();
    TheMetagame.Init();
    ScriptFunction::Register(QuitApp, kExitAppCommand, nullptr);
}

// NTSC-U/C: 0x00100ab0, PAL: 0x00101ac8
void TerminateAppSubsystems() {
    ScriptFunction::Unregister(QuitApp);
    TheMetagame.Terminate();
    TheWorldMgr->Terminate();
    TheTriggerMgr.Terminate();
    TheGameDb->Terminate();
    TheLocale.Terminate();
    TerminateNetSubsystem();
    CheckLeftoverObjects();
    TheRnd->Terminate();
    Rnd::TheManager.DeleteLoadedObjects();
    Synth::Destroy();
}

} // namespace

// NTSC-U/C: 0x00100b50
int main(int argc, char **argv) {
    InitializeAppSubsystems(argc, argv);
    JoypadSetStickMessages(true);

    while (!g_bQuitRequested) {
        UpdateAppFrame();
    }

    TerminateAppSubsystems();
    SystemTerminate();
    DebugPrint("exiting...\n");
    return 0;
}

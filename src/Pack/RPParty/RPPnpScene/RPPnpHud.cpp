// Table Tennis HUD: owns the eleven pingpong.arc layouts and forwards the scene steps to them.
// Unit 0x802327AC-0x80232C74 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPPnpScene/RPPnpHud.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPParty/RPPnpScene/RPPnpGame.h>

RPPnpHud* RPPnpHud::sInstance;

void RPPnpHud::CreateInstance() {
    sInstance = new RPPnpHud();
}

void RPPnpHud::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPPnpHud::~RPPnpHud() {
    mpBalloonLyt = NULL;
    mpScoreLyt = NULL;
    mpServerLyt = NULL;
    mpArrowLyt = NULL;
    mpCountLyt = NULL;
    mpResultLyt = NULL;
    mpStartMsgLyt = NULL;
    mpHelpMsgLyt = NULL;
    mpFaultMsgLyt = NULL;
    mpServeMsgLyt = NULL;
    mpPointMsgLyt = NULL;
}

void RPPnpHud::Create() {
    mpBalloonLyt->Create();
    mpScoreLyt->Create();
    mpServerLyt->Create();
    mpArrowLyt->Create();
    mpCountLyt->Create();
    mpResultLyt->Create();
    mpStartMsgLyt->Create();
    mpHelpMsgLyt->Create();
    mpFaultMsgLyt->Create();
    mpServeMsgLyt->Create();
    mpPointMsgLyt->Create();
}

void RPPnpHud::Load() {
    mpBalloonLyt->Load();
    mpScoreLyt->Load();
    mpServerLyt->Load();
    mpArrowLyt->Load();
    mpCountLyt->Load();
    mpResultLyt->Load();
    mpStartMsgLyt->Load();
    mpHelpMsgLyt->Load();
    mpFaultMsgLyt->Load();
    mpServeMsgLyt->Load();
    mpPointMsgLyt->Load();
}

void RPPnpHud::Reset() {
    mpBalloonLyt->Reset();
    mpScoreLyt->Reset();
    mpServerLyt->Reset();
    mpArrowLyt->Reset();
    mpCountLyt->Reset();
    mpResultLyt->Reset();
    mpStartMsgLyt->Reset();
    mpHelpMsgLyt->Reset();
    mpFaultMsgLyt->Reset();
    mpServeMsgLyt->Reset();
    mpPointMsgLyt->Reset();
}

void RPPnpHud::Calculate() {
    mpBalloonLyt->Calculate();
    mpScoreLyt->Calculate();
    mpServerLyt->Calculate();
    mpArrowLyt->Calculate();
    mpCountLyt->Calculate();
    mpResultLyt->Calculate();
    mpStartMsgLyt->Calculate();
    mpHelpMsgLyt->Calculate();
    mpFaultMsgLyt->Calculate();
    mpServeMsgLyt->Calculate();
    mpPointMsgLyt->Calculate();
}

void RPPnpHud::Exit() {
    mpBalloonLyt->Exit();
    mpScoreLyt->Exit();
    mpServerLyt->Exit();
    mpArrowLyt->Exit();
    mpCountLyt->Exit();
    mpResultLyt->Exit();
    mpStartMsgLyt->Exit();
    mpHelpMsgLyt->Exit();
    mpFaultMsgLyt->Exit();
    mpServeMsgLyt->Exit();
    mpPointMsgLyt->Exit();
}

void RPPnpHud::Draw() {
    if (RPGrpRenderer::GetDrawStep() != 5) {
        return;
    }

    switch (RPGrpRenderer::GetCurrentViewNo()) {
    case 1:
        mpBalloonLyt->Draw();
        if (RPPnpGame::GetInstance()->IsMultiPlayer()) {
            mpServerLyt->Draw();
        }
        mpArrowLyt->Draw();
        mpCountLyt->Draw();
        mpResultLyt->Draw();
        mpStartMsgLyt->Draw();
        mpHelpMsgLyt->Draw();
        mpFaultMsgLyt->Draw();
        mpServeMsgLyt->Draw();
        mpPointMsgLyt->Draw();
        break;
    case 2:
        break;
    case 3:
        mpScoreLyt->Draw();
        break;
    }
}

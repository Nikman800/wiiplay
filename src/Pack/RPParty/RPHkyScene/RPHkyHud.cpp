// Laser Hockey HUD: owns the seven airHockey.arc layouts and forwards the scene steps to them.
// Unit 0x80221D6C-0x80222118 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPHkyScene/RPHkyHud.h>

#include <Pack/RPGraphics.h>

RPHkyHud* RPHkyHud::sInstance;

void RPHkyHud::CreateInstance() {
    sInstance = new RPHkyHud();
}

void RPHkyHud::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPHkyHud::~RPHkyHud() {
    mpStartMsgLyt = NULL;
    mpBalloonLyt = NULL;
    mpTimeLyt = NULL;
    mpScoreLyt = NULL;
    mpGoalMsgLyt = NULL;
    mpTimeUpMsgLyt = NULL;
    mpMsg03Lyt = NULL;
}

void RPHkyHud::Create() {
    mpStartMsgLyt->Create();
    mpBalloonLyt->Create();
    mpTimeLyt->Create();
    mpScoreLyt->Create();
    mpGoalMsgLyt->Create();
    mpTimeUpMsgLyt->Create();
    mpMsg03Lyt->Create();
}

void RPHkyHud::Load() {
    mpStartMsgLyt->Load();
    mpBalloonLyt->Load();
    mpTimeLyt->Load();
    mpScoreLyt->Load();
    mpGoalMsgLyt->Load();
    mpTimeUpMsgLyt->Load();
    mpMsg03Lyt->Load();
}

void RPHkyHud::Reset() {
    mpStartMsgLyt->Reset();
    mpBalloonLyt->Reset();
    mpTimeLyt->Reset();
    mpScoreLyt->Reset();
    mpGoalMsgLyt->Reset();
    mpTimeUpMsgLyt->Reset();
    mpMsg03Lyt->Reset();
}

void RPHkyHud::Calculate() {
    mpStartMsgLyt->Calculate();
    mpBalloonLyt->Calculate();
    mpTimeLyt->Calculate();
    mpScoreLyt->Calculate();
    mpGoalMsgLyt->Calculate();
    mpTimeUpMsgLyt->Calculate();
    mpMsg03Lyt->Calculate();
}

void RPHkyHud::Exit() {
    mpStartMsgLyt->Exit();
    mpBalloonLyt->Exit();
    mpTimeLyt->Exit();
    mpScoreLyt->Exit();
    mpGoalMsgLyt->Exit();
    mpTimeUpMsgLyt->Exit();
    mpMsg03Lyt->Exit();
}

void RPHkyHud::Draw() {
    if (RPGrpRenderer::GetDrawStep() != 5) {
        return;
    }

    switch (RPGrpRenderer::GetCurrentViewNo()) {
    case 1:
        mpBalloonLyt->Draw();
        mpTimeLyt->Draw();
        mpStartMsgLyt->Draw();
        mpGoalMsgLyt->Draw();
        mpTimeUpMsgLyt->Draw();
        mpMsg03Lyt->Draw();
        break;
    case 2:
        mpScoreLyt->Draw();
        break;
    }
}

#include <Pack/RPParty/RPCowScene/RPCowHud_13.h>

#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPParty/RPCowScene/RPCowGame.h>

void RPCowAngleDisp::BindAnimsMode1() {
    RPCowGame* pGame = RPCowGame::GetInstance();
    mpPanes[0]->SetVisible(true);
    mpPanes[2]->SetVisible(true);
    mpPanes[4]->SetVisible(true);
    mpLayout->unbindAnmObj(mpAnmObjs[0]);
    mpLayout->unbindAnmObj(mpAnmObjs[2]);
    mpLayout->unbindAnmObj(mpAnmObjs[3]);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[0], mpPanes[0], false);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[2], mpPanes[2], true);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[2], mpPanes[4], true);
    if (!mSwitching[0]) {
        mpAnmObjs[2]->setRate(0.0f);
        mpAnmObjs[2]->setFrame(0.0f);
        mpAnmObjs[2]->restart();
    }
    if (pGame->GetPlayerNum() >= 2) {
        mpPanes[1]->SetVisible(true);
        mpPanes[3]->SetVisible(true);
        mpPanes[5]->SetVisible(true);
        RPSysLayout::bindAnmObjToPane(mpAnmObjs[0], mpPanes[1], false);
        RPSysLayout::bindAnmObjToPane(mpAnmObjs[3], mpPanes[3], true);
        RPSysLayout::bindAnmObjToPane(mpAnmObjs[3], mpPanes[5], true);
        if (!mSwitching[1]) {
            mpAnmObjs[3]->setRate(0.0f);
            mpAnmObjs[3]->setFrame(0.0f);
            mpAnmObjs[3]->restart();
        }
    }
    mpAnmObjs[0]->restart();
    mState = 1;
}

void RPCowAngleDisp::BindAnimsMode2() {
    RPCowGame* pGame = RPCowGame::GetInstance();
    mpLayout->unbindAnmObj(mpAnmObjs[1]);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[1], mpPanes[0], false);
    if (pGame->GetPlayerNum() >= 2) {
        RPSysLayout::bindAnmObjToPane(mpAnmObjs[1], mpPanes[1], false);
    }
    mpAnmObjs[1]->restart();
    mState = 2;
}

void RPCowAngleDisp::PlaySwitch(s32 player) {
    if (!mSwitching[player]) {
        switch (player) {
        case 0:
            mpLayout->unbindAnmObj(mpAnmObjs[2]);
            RPSysLayout::bindAnmObjToPane(mpAnmObjs[2], mpPanes[2], true);
            RPSysLayout::bindAnmObjToPane(mpAnmObjs[2], mpPanes[4], true);
            mpAnmObjs[2]->setRate(1.0f);
            mpAnmObjs[2]->setFrame(0.0f);
            mpAnmObjs[2]->restart();
            break;
        case 1:
            mpLayout->unbindAnmObj(mpAnmObjs[3]);
            RPSysLayout::bindAnmObjToPane(mpAnmObjs[3], mpPanes[3], true);
            RPSysLayout::bindAnmObjToPane(mpAnmObjs[3], mpPanes[5], true);
            mpAnmObjs[3]->setRate(1.0f);
            mpAnmObjs[3]->setFrame(0.0f);
            mpAnmObjs[3]->restart();
            break;
        }
        mSwitching[player] = true;
    }
}

void RPCowAngleDisp::SetAngle(s32 player, f32 angle) {
    switch (player) {
    case 0:
        mpPanes[4]->SetRotate(nw4r::math::VEC3(0.0f, 0.0f, -angle));
        mpPanes[4]->SetTranslate(nw4r::math::VEC3(0.75f * angle, sCountPosY, 0.0f));
        break;
    case 1:
        mpPanes[5]->SetRotate(nw4r::math::VEC3(0.0f, 0.0f, angle));
        mpPanes[5]->SetTranslate(nw4r::math::VEC3(0.75f * -angle, sCountPosY, 0.0f));
        break;
    }
}

RPCowAngleDisp::RPCowAngleDisp() {}

RPCowAngleDisp::~RPCowAngleDisp() {}

void RPCowAngleDisp::LoadResource() {
    RPCowGame* pGame = RPCowGame::GetInstance();
    mpLayout = RPSysLayout::create(NULL, pGame->GetLytAccessor(), "cowAngle_00.brlyt");
    const char* anm[] = {"cowAngle_00_in_00.brlan", "cowAngle_00_out_00.brlan",
                         "cowAngle_00_switchCow_00.brlan", "cowAngle_00_switchCow_00.brlan"};
    for (int i = 0; i < 4; i++) {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, pGame->GetLytAccessor(), anm[i]);
    }
    const char* pane[] = {"N_1pAngleDisp_00", "N_2pAngleDisp_00", "N_1pBar_00",
                          "N_2pBar_00",       "N_1pRotateCnt_00", "N_2pRotateCnt_00"};
    for (int i = 0; i < 6; i++) {
        mpPanes[i] = mpLayout->findPane(pane[i]);
    }
}

void RPCowAngleDisp::OnStart() {
    for (int i = 0; i < 6; i++) {
        mpPanes[i]->SetVisible(false);
    }
    mSwitching[0] = false;
    mSwitching[1] = false;
    mpPanes[4]->SetRotate(nw4r::math::VEC3(0.0f, 0.0f, 0.0f));
    mpPanes[4]->SetTranslate(nw4r::math::VEC3(0.0f, sCountPosY, 0.0f));
    mpPanes[5]->SetRotate(nw4r::math::VEC3(0.0f, 0.0f, 0.0f));
    mpPanes[5]->SetTranslate(nw4r::math::VEC3(0.0f, sCountPosY, 0.0f));
    mState = 0;
    mpLayout->unbindAllAnmObj();
    for (int i = 0; i < 4; i++) {
        mpAnmObjs[i]->resetFrame();
    }
}

void RPCowAngleDisp::OnStop() {}

void RPCowAngleDisp::OnActivate() {
    switch (mState) {
    case 1:
        if (mpPanes[0]->IsVisible() && mpAnmObjs[0]->isFinished()) {
        }
        break;
    case 2:
        if (mpAnmObjs[1]->isFinished()) {
            mpPanes[0]->SetVisible(false);
            mpPanes[1]->SetVisible(false);
        }
        break;
    }
    if (mState != 0) {
        if (mSwitching[0] && mpAnmObjs[2]->isFinished()) {
            mpAnmObjs[2]->restart();
            mSwitching[0] = false;
        }
        if (mSwitching[1] && mpAnmObjs[3]->isFinished()) {
            mpAnmObjs[3]->restart();
            mSwitching[1] = false;
        }
    }
    mpLayout->calc();
}

void RPCowAngleDisp::DrawScreen(u8 pass) {
    if (pass == 4) {
        mpLayout->draw();
    }
}

const f32 RPCowAngleDisp::sCountPosY = -21.0f;

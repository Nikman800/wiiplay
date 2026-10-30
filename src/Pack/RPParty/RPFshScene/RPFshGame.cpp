// Fishing game manager (RPFshGame): creates, resets, runs and destroys every Fishing object.
// Original range: 0x80214E78-0x8021588C. Functions in reverse address order.
#include <Pack/RPParty/RPFshScene/RPFshGame.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPParty/RPFshScene/RPFshBonusFish.h>
#include <Pack/RPParty/RPFshScene/RPFshCamera.h>
#include <Pack/RPParty/RPFshScene/RPFshFish.h>
#include <Pack/RPParty/RPFshScene/RPFshFishMgr.h>
#include <Pack/RPParty/RPFshScene/RPFshMsg.h>
#include <Pack/RPParty/RPFshScene/RPFshPlayerMgr.h>
#include <Pack/RPParty/RPFshScene/RPFshPointPopup.h>
#include <Pack/RPParty/RPFshScene/RPFshPrompt.h>
#include <Pack/RPParty/RPFshScene/RPFshResultLayout.h>
#include <Pack/RPParty/RPFshScene/RPFshScoreLayout.h>
#include <Pack/RPParty/RPFshScene/RPFshStage.h>
#include <Pack/RPParty/RPFshScene/RPFshStartLayout.h>
#include <Pack/RPParty/RPFshScene/RPFshTimer.h>
#include <Pack/RPParty/RPPartyCommon/RPCommon.h>
#include <Pack/RPParty/RPPartyCommon/RPLeadCursor.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneInfo.h>
#include <Pack/RPParty/RPPartyCommon/RPScreenFade.h>
#include <Pack/RPSystem.h>

const RPFshFishTypeParam RPFshFish::sTypeParams[TYPE_NUM] = {
    {3.0f, 0.98f, 50, 0.07f, 3, 60, 6.0f, 40.0f, 6.0f, 6.0f, 200, 50, -1, -1},
    {2.2f, 0.985f, 70, 0.05f, 2, 15, 9.0f, 40.0f, 6.0f, 8.0f, 120, 0, -1, -1},
    {2.4f, 0.985f, 60, 0.06f, 1, 90, 6.0f, 40.0f, 6.0f, 7.0f, 60, 0, -1, -1},
    {4.5f, 0.95f, 25, 0.13f, 0, 60, 8.0f, 26.0f, 6.0f, 5.0f, -50, 0, 1200, 1500},
    {1.5f, 0.985f, 50, 0.04f, 4, 30, 6.0f, 75.0f, 5.0f, 10.0f, 350, 0, -1, -1},
    {2.0f, 0.985f, 60, 0.04f, 1, 30, 6.0f, 80.0f, 5.0f, 8.0f, 250, 0, 300, 300},
};

RPFshGame* RPFshGame::sInstance;
s32 RPFshGame::sPlayerNum;

void RPFshGame::CreateInstance() {
    sInstance = new RPFshGame();
}

void RPFshGame::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPFshGame::~RPFshGame() {}

RPFshGame::RPFshGame() {
    sPlayerNum = RPSceneInfo::spInstance->GetPlayerNum();

    mpStateMachine = new RPStateMachine<RPFshGame>(this, STATE_NUM);
    RPFshStateFuncs<RPFshGame> funcs[STATE_NUM] = {
        {&RPFshGame::PlayEnter, &RPFshGame::PlayCalc},
    };
    for (int i = 0; i < STATE_NUM; i++) {
        mpStateMachine->SetStateFuncs(funcs[i].mEnter, funcs[i].mCalc, i);
    }

    RPFshStartLayout::CreateInstance();
    RPFshTimer::CreateInstance();
    RPFshCameraMgr::CreateInstance();
    RPFshStage::CreateInstance();
    RPFshPlayerMgr::CreateInstance();
    RPFshFishMgr::CreateInstance();
    RPFshResultLayout::CreateInstance();
    RPFshScoreLayout::CreateInstance();
    RPFshYankPrompt::CreateInstance();
    RPFshPointPopup::CreateInstance();
    RPFshAimArrow::CreateInstance();
    RPFshBonusFish::CreateInstance();
    RPFshMsg00::CreateInstance();
    RPFshMsg01::CreateInstance();

    RPCommonParam param;
    param.mGame = 6;
    param.mPlayerNum = sPlayerNum;
    RPCommon::CreateInstance(param);
}

void RPFshGame::CreateObjects() {
    RPFshStartLayout::GetInstance()->Create();
    RPFshTimer::GetInstance()->Load();
    RPFshStage::GetInstance()->Create();
    RPFshPlayerMgr::GetInstance()->CreateAll();
    RPFshFishMgr::GetInstance()->CreateAll();
    RPFshResultLayout::GetInstance()->Create();
    RPFshScoreLayout::GetInstance()->Create();
    RPFshYankPrompt::GetInstance()->Create();
    RPFshPointPopup::GetInstance()->Create();
    RPFshAimArrow::GetInstance()->Create();
    RPFshBonusFish::GetInstance()->Create();
    RPFshMsg00::GetInstance()->Create();
    RPFshMsg01::GetInstance()->Create();
    RPCommon::GetInstance()->LoadResources();
}

void RPFshGame::ResetObjects() {
    mpStateMachine->Reset(STATE_PLAY);

    RPFshStartLayout::GetInstance()->Reset();
    RPFshTimer::GetInstance()->Reset();
    RPFshCameraMgr::GetInstance()->Reset();
    RPFshStage::GetInstance()->Reset();
    RPFshPlayerMgr::GetInstance()->Reset();
    RPFshFishMgr::GetInstance()->Reset();
    RPFshResultLayout::GetInstance()->Reset();
    RPFshScoreLayout::GetInstance()->Reset();
    RPFshYankPrompt::GetInstance()->Reset();
    RPFshPointPopup::GetInstance()->Reset();
    RPFshAimArrow::GetInstance()->Reset();
    RPFshBonusFish::GetInstance()->Reset();
    RPFshMsg00::GetInstance()->Reset();
    RPFshMsg01::GetInstance()->Reset();
    RPCommon::GetInstance()->Reset();

    RPLeadCursor* pLeadCursor = RPCommon::GetInstance()->GetLeadCursor();
    pLeadCursor->SetTimer(30, 0);
    pLeadCursor->SetTimer(360, 1);

    RPGrpView::ScreenEffect* pEffect = RPGrpRenderer::GetCurrent()->GetView(4)->GetScreenEffect();
    GXColor white = {255, 255, 255, 0};
    pEffect->SetColorMultiply(white);
    pEffect->SetBlurAlpha(0);
    pEffect->SetBlurOffset(1.0f);
}

void RPFshGame::DestroyObjects() {
    RPFshStartLayout::GetInstance()->Exit();
    RPFshStage::GetInstance()->Exit();
    RPFshPlayerMgr::GetInstance()->ExitAll();
    RPFshFishMgr::GetInstance()->ExitAll();
    RPFshResultLayout::GetInstance()->Exit();
    RPFshScoreLayout::GetInstance()->Exit();
    RPFshYankPrompt::GetInstance()->Exit();
    RPFshPointPopup::GetInstance()->Exit();
    RPFshAimArrow::GetInstance()->Exit();

    RPFshStartLayout::DestroyInstance();
    RPFshCameraMgr::DestroyInstance();
    RPFshStage::DestroyInstance();
    RPFshPlayerMgr::DestroyInstance();
    RPFshFishMgr::DestroyInstance();
    RPFshTimer::DestroyInstance();
    RPFshResultLayout::DestroyInstance();
    RPFshScoreLayout::DestroyInstance();
    RPFshYankPrompt::DestroyInstance();
    RPFshPointPopup::DestroyInstance();
    RPFshAimArrow::DestroyInstance();
    RPFshBonusFish::DestroyInstance();
    RPFshMsg00::DestroyInstance();
    RPFshMsg01::DestroyInstance();
    RPCommon::DestroyInstance();
}

void RPFshGame::Calculate() {
    mpStateMachine->Calculate();

    RPGrpView::ScreenEffect* pEffect = RPGrpRenderer::GetCurrent()->GetView(4)->GetScreenEffect();
    GXColor color = RPCommon::GetInstance()->GetScreenFade()->GetColor();
    pEffect->SetColorMultiply(color);
    pEffect->SetBlurAlpha(color.a);
}

void RPFshGame::PlayEnter() {}

void RPFshGame::PlayCalc() {
    RPFshStartLayout::GetInstance()->Calculate();
    RPFshTimer::GetInstance()->Calculate();
    RPFshCameraMgr::GetInstance()->Calculate();
    RPFshStage::GetInstance()->Calculate();
    RPFshPlayerMgr::GetInstance()->Calculate();
    RPFshFishMgr::GetInstance()->Calculate();
    RPFshPlayerMgr::GetInstance()->CalculateRods();
    RPFshResultLayout::GetInstance()->Calculate();
    RPFshScoreLayout::GetInstance()->Calculate();
    RPFshYankPrompt::GetInstance()->Calculate();
    RPFshPointPopup::GetInstance()->Calculate();
    RPFshAimArrow::GetInstance()->Calculate();
    RPFshBonusFish::GetInstance()->Calculate();
    RPFshMsg00::GetInstance()->Calculate();
    RPFshMsg01::GetInstance()->Calculate();
    RPCommon::GetInstance()->Calculate();

    if (RPFshResultLayout::GetInstance()->GetState() == RPFshResultLayout::STATE_DONE &&
        RPSysSceneMgr::instance()->getCurrentSceneRP()->isSceneChangeEnable()) {
        if (RPFshResultLayout::GetInstance()->IsRetry()) {
            RPSysSceneCreator::instance()->changeSceneAfterFade(0x10);
        } else {
            RPSysSceneCreator::instance()->changeSceneAfterFade(0x1B);
        }
    }
}

const RPFshFishTypeParam* RPFshFish::GetTypeParam(s32 type) {
    return &sTypeParams[type];
}

// Flush the game functions, then emit the state-machine instantiation in original address
// order. Implicit instantiation emits CallExit before CallEnter; explicit instantiation must
// also disable automatic inlining to retain the original calls inside Calculate.
#pragma defer_codegen off
#pragma auto_inline off
template RPStateMachine<RPFshGame>::RPStateMachine(RPFshGame*, s32);
template void RPStateMachine<RPFshGame>::SetStateFuncs(const StateFunc&, const StateFunc&, s32);
template void RPStateMachine<RPFshGame>::Reset(s32);
template void RPStateMachine<RPFshGame>::Calculate();
template void RPStateMachine<RPFshGame>::CallEnter();
template void RPStateMachine<RPFshGame>::CallCalc();
template void RPStateMachine<RPFshGame>::CallExit();


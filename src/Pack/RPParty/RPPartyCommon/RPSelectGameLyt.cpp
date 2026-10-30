// Game-select board (RPSelectGameLyt, ptSelectGame_00.brlyt): construction, loading, the main
// states and the queries used by RPSelectScene. Unit 0x801ED2D0-0x801EF328. Functions in reverse
// address order; the state machine instantiation follows at the end of the file (the game ->
// message index function after it, Party_gameIndexToMsgIndex, starts RPSelectScene.cpp).
#include <Pack/RPParty/RPPartyCommon/RPSelectGameLyt.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneInfo.h>
#include <Pack/RPParty/RPPartyCommon/RPSelectScene.h>
#include <Pack/RPKernel/RPSysAvatar.h>
#include <Pack/RPSystem/RPSysPlayer.h>

#include <Pack/RPAudio.h>
#include <Pack/RPKernel.h>
#include <Pack/RPSystem.h>

#include <egg/gfxe/eggScreen.h>

#include <cstdio>

extern "C" {
// Still named by their ledger placeholders (their units are not linked yet)
s32 Menu_slotToGameId(s32 slot);
s32 Menu_gameIdToSlot(s32 id);
EGG::Vector2f Lyt_getPaneGlobalPos(const nw4r::lyt::Pane* pPane);
}

namespace {
struct RPSelectGameLytStateFuncs {
    RPSelectGameSeq::StateFunc mEnter; // at 0x0
    RPSelectGameSeq::StateFunc mCalc;  // at 0xC
};

inline RPSysMessage* GetSelectGameMessage() {
    RPSysMessage* pMessage = RPSelectScene::sInstance->mpMessage;
    return pMessage;
}

} // namespace

// Score ranges and medal thresholds of the result screens: local initializers of shared inline
// helpers this file does not call (they still come first in .rodata, 8031CEA0; the same
// 0x100 bytes are in RPVsWinner and RPFshScoreLayout)
inline s32 RPSelectGameScoreRange(s32 i) {
    s32 ranges[18] = {-999, 9999, 0, 999, 0, 999, 0, 9999, 0, 999, -99, 99, -999, 9999, 0, 999, 0, 999};
    return ranges[i];
}

inline s32 RPSelectGameMedalScore(s32 i) {
    s32 scores[36] = {200,  320,  450,  600,  20,  40,  60,  80,  40,  70,  100, 200,
                      150,  400,  800,  1200, 3,   7,   10,  14,  15,  27,  51,  60,
                      1000, 1600, 2300, 2600, 170, 250, 300, 325, 11,  32,  84,  125};
    return scores[i];
}

inline s32 RPSelectGameMsgIndex(s32 game) {
    s32 msgIdx[RPSelectGameLyt::GAME_NUM] = {0, 1, 2, 3, 4, 5, 7, 8, 9};
    return msgIdx[game];
}

RPSelectGameLyt::RPSelectGameLyt(s32 playerNum) {
    mpStateMachine = new RPSelectGameSeq(this, 6);
    {
        RPSelectGameLytStateFuncs funcs[6] = {
            {NULL, NULL},
            {&RPSelectGameLyt::EnterIn, &RPSelectGameLyt::CalcIn},
            {&RPSelectGameLyt::EnterIdle, &RPSelectGameLyt::CalcIdle},
            {&RPSelectGameLyt::EnterDecide, &RPSelectGameLyt::CalcDecide},
            {&RPSelectGameLyt::EnterOut, &RPSelectGameLyt::CalcOut},
            {NULL, NULL},
        };
        for (int i = 0; i < 6; i++) {
            mpStateMachine->SetStateFuncs(funcs[i].mEnter, funcs[i].mCalc, i);
        }
    }

    for (int i = 0; i < GAME_NUM; i++) {
        mpSlotStates[i] = new RPSelectGameSeq(this, 6);
        RPSelectGameLytStateFuncs funcs[6] = {
            {&RPSelectGameLyt::EnterSlotHighlighted, &RPSelectGameLyt::CalcSlotHighlighted},
            {&RPSelectGameLyt::EnterSlotHighlight, &RPSelectGameLyt::CalcSlotHighlight},
            {&RPSelectGameLyt::EnterSlotIdle, &RPSelectGameLyt::CalcSlotIdle},
            {&RPSelectGameLyt::EnterSlotUnhighlight, &RPSelectGameLyt::CalcSlotUnhighlight},
            {&RPSelectGameLyt::EnterSlotDecide, &RPSelectGameLyt::CalcSlotDecide},
            {NULL, NULL},
        };
        for (int j = 0; j < 6; j++) {
            mpSlotStates[i]->SetStateFuncs(funcs[j].mEnter, funcs[j].mCalc, j);
        }
    }

    for (int i = 0; i < GAME_NUM; i++) {
        mpLockedStates[i] = new RPSelectGameSeq(this, 4);
        RPSelectGameLytStateFuncs funcs[4] = {
            {&RPSelectGameLyt::EnterLockedIdle, &RPSelectGameLyt::CalcLockedIdle},
            {&RPSelectGameLyt::EnterLockedStart, &RPSelectGameLyt::CalcLockedStart},
            {&RPSelectGameLyt::EnterLockedLoop, &RPSelectGameLyt::CalcLockedLoop},
            {&RPSelectGameLyt::EnterLockedEnd, &RPSelectGameLyt::CalcLockedEnd},
        };
        for (int j = 0; j < 4; j++) {
            mpLockedStates[i]->SetStateFuncs(funcs[j].mEnter, funcs[j].mCalc, j);
        }
    }

    mpExpState = new RPSelectGameSeq(this, 2);
    {
        RPSelectGameLytStateFuncs funcs[2] = {
            {&RPSelectGameLyt::EnterExpIdle, &RPSelectGameLyt::CalcExpIdle},
            {&RPSelectGameLyt::EnterExpChange, &RPSelectGameLyt::CalcExpChange},
        };
        for (int i = 0; i < 2; i++) {
            mpExpState->SetStateFuncs(funcs[i].mEnter, funcs[i].mCalc, i);
        }
    }

    mPlayerNum = playerNum;

    const char* effectNames[EFFECT_NUM] = {
        "Bsm_medal_bronze01",    "Bsm_medal_silver01",    "Bsm_medal_gold01",
        "Bsm_medal_gold02",      "Bsm_medal_platinum01",  "Bsm_medal_platinum02",
        "Bsm_medal_platinum03",
    };
    for (int i = 0; i < GAME_NUM; i++) {
        for (int j = 0; j < EFFECT_NUM; j++) {
            mpMedalEffects[i][j] = new RPSysEffect(effectNames[j], 2, 0);
        }
    }
}

void RPSelectGameLyt::Load(EGG::Archive* pArchive) {
    AttachArcFile(pArchive, "ptSelectGame.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "ptSelectGame_00.brlyt");

    const char* anmNames[2] = {"ptSelectGame_00_in_00.brlan", "ptSelectGame_00_out_00.brlan"};
    for (int i = 0; i < 2; i++) {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[i]);
    }

    const char* slotAnmNames[3] = {"ptSelectGame_00_offOn_00.brlan",
                                   "ptSelectGame_00_onOff_00.brlan",
                                   "ptSelectGame_00_push_00.brlan"};
    for (int i = 0; i < GAME_NUM; i++) {
        for (int j = 0; j < 3; j++) {
            mpSlotAnmObjs[i][j] = mpLayout->createAnmObj(NULL, mpResAccessor, slotAnmNames[j]);
        }
    }

    const char* expAnmNames[2] = {"ptSelectGame_00_inExp_00.brlan",
                                  "ptSelectGame_00_outEep_00.brlan"};
    for (int i = 0; i < GAME_NUM; i++) {
        for (int j = 0; j < 2; j++) {
            mpExpAnmObjs[i][j] = mpLayout->createAnmObj(NULL, mpResAccessor, expAnmNames[j]);
        }
    }

    const char* scoreAnmNames[1] = {"ptSelectGame_00_changeScore_00.brlan"};
    for (int i = 0; i < 1; i++) {
        mpScoreAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, scoreAnmNames[i]);
    }

    mpCenterPane = mpLayout->findPane("N_center_00");
    mpPlayerPosPanes[0] = mpLayout->findPane("N_pPositionC_00");
    mpPlayerPosPanes[1] = mpLayout->findPane("N_pPositionL_00");
    mpPlayerPosPanes[2] = mpLayout->findPane("N_pPositionR_00");

    const char* singleNames[1] = {"N_player_00"};
    const char* vsNames[2] = {"N_vsResult_00", "N_vsPlayer_00"};
    for (int i = 0; i < 1; i++) {
        if (mPlayerNum == 2) {
            mpLayout->findPane(singleNames[i])->SetVisible(false);
        }
    }
    for (int i = 0; i < 2; i++) {
        if (mPlayerNum == 1) {
            mpLayout->findPane(vsNames[i])->SetVisible(false);
        }
    }

    char name[32];
    for (int i = 0; i < GAME_NUM; i++) {
        RPPartyPlayerData* pData = RPSysPlayerMgr::instance()->getPlayer(0).mpPartyPlayerData;
        s32 medal =
            pData->getMedal(static_cast<RPPartyPlayerData::EGame>(Menu_gameIdToSlot(i)));
        if (mPlayerNum == 1) {
            mMedals[i] = medal;
        } else {
            mMedals[i] = 0;
        }

        for (int j = 0; j < 4; j++) {
            const char* rankNames[4] = {"P_rankC_", "P_rankS_", "P_rankG_", "P_rankP_"};
            std::snprintf(name, sizeof(name), "%s%02u", rankNames[j], i);
            nw4r::lyt::Pane* pPane = mpLayout->findPane(name);
            if (mPlayerNum == 1 && medal == j + 1) {
                pPane->SetVisible(true);
            } else {
                pPane->SetVisible(false);
            }
        }

        std::snprintf(name, sizeof(name), "P_rankSh_%02u", i);
        nw4r::lyt::Pane* pPane = mpLayout->findPane(name);
        if (mPlayerNum == 1 && medal != 0) {
            pPane->SetVisible(true);
        } else {
            pPane->SetVisible(false);
        }
    }

    for (int i = 0; i < GAME_NUM; i++) {
        s32 posIdx[GAME_NUM] = {1, 0, 0, 1, 2, 3, 4, 4, 3};
        std::snprintf(name, sizeof(name), "N_gamesPoAd_%02u", posIdx[i]);
        mpGamePosPanes[i] = mpLayout->findPane(name);
    }

    for (int i = 0; i < GAME_NUM; i++) {
        for (int j = 0; j < 4; j++) {
            const char* paneNames[4] = {"N_game_", "N_gamePic_", "N_gameButton_",
                                        "N_gameCover_"};
            std::snprintf(name, sizeof(name), "%s%02u", paneNames[j], i);
            mpIconPanes[i][j] = mpLayout->findPane(name);
        }
    }

    for (int i = 0; i < GAME_NUM; i++) {
        const nw4r::math::VEC3& rTrans = mpIconPanes[i][0]->GetTranslate();
        nw4r::math::VEC3 trans(rTrans.x, rTrans.y, rTrans.z);
        mIconPos[i] = EGG::Vector2f(trans.x, trans.y);
    }

    for (int i = 0; i < GAME_NUM; i++) {
        std::snprintf(name, sizeof(name), "N_rank_%02u", i);
        mpRankPanes[i] = mpLayout->findPane(name);
    }

    for (int i = 0; i < GAME_NUM; i++) {
        std::snprintf(name, sizeof(name), "N_gameExp_%02u", i);
        mpExpPanes[i] = mpLayout->findPane(name);
    }

    for (int i = 0; i < GAME_NUM; i++) {
        if (RPSysSaveDataMgr::instance()->getPartySystemData().isGameOpen(
                Menu_gameIdToSlot(i))) {
            mpIconPanes[i][1]->SetVisible(true);
            mpIconPanes[i][2]->SetVisible(true);
            mpIconPanes[i][3]->SetVisible(false);
            mUnlocked[i] = true;
        } else {
            mpIconPanes[i][1]->SetVisible(false);
            mpIconPanes[i][2]->SetVisible(false);
            mpIconPanes[i][3]->SetVisible(true);
            mUnlocked[i] = false;
        }
    }

    for (int i = 0; i < GAME_NUM; i++) {
        for (int j = 1; j < 4; j++) {
            const char* boundingNames[4] = {"", "B_gamePic_", "B_gameButton_",
                                            "B_gameCover_"};
            std::snprintf(name, sizeof(name), "%s%02u", boundingNames[j], i);
            mpIconBoundings[i][j] = mpLayout->findPane(name);

            EGG::Matrix34f mtx;
            mtx.makeIdentity();
            mtx.setTranslation(EGG::Vector3f(12800.0f, 12800.0f, 0.0f));
            mpIconBoundings[i][j]->SetGlobalMtx(mtx);
        }
    }

    mpScoreTextBox = mpLayout->findTextBox("T_p1Score_00");

    RPSysMessage* pMessage = GetSelectGameMessage();
    const char* hNames[1] = {"T_nH_00"};
    const char* numNames[2] = {"T_nL_00", "T_nR_00"};
    RPSceneInfo* pSceneInfo = RPSceneInfo::spInstance;
    if (mPlayerNum == 2) {
        for (int i = 0; i < 2; i++) {
            wchar_t number[20];
            RPSysStringUtility::GetStringHalfSizeNumber(
                pSceneInfo->GetUnk8(i),
                number, 20, false);
            mpLayout->findTextBox(numNames[i])->setText(number, 0);
        }
    }

    if (mPlayerNum == 1) {
        mpLayout->findTextBox("T_p1_00")->setMessage(pMessage, 0x1B5D, 0, 0);
    } else {
        const char* playerNames[2] = {"T_vsP1_00", "T_vsP2_00"};
        for (int i = 0; i < 2; i++) {
            mpLayout->findTextBox(playerNames[i])->setMessage(pMessage, 0x1B5D, i, 0);
        }
    }

    if (mPlayerNum == 1) {
        RPSysKokeshi* pAvatar = RPSelectScene::sInstance->GetDollAvatar(0);
        mpLayout->findTextBox("T_p1Name_00")->setText(pAvatar->mAdditionalInfo.name, 0);
    } else {
        const char* nameNames[2] = {"T_vsP1Name_00", "T_vsP2Name_00"};
        for (int i = 0; i < 2; i++) {
            RPSysKokeshi* pAvatar = RPSelectScene::sInstance->GetDollAvatar(i);
            mpLayout->findTextBox(nameNames[i])->setText(pAvatar->mAdditionalInfo.name, 0);
        }
    }

    for (int i = 0; i < GAME_NUM; i++) {
        std::snprintf(name, sizeof(name), "T_gameExp_%02u", i);
        RPSysLytTextBox* pTextBox = mpLayout->findTextBox(name);
        pTextBox->setMessage(pMessage, 0x1B8A, RPSelectGameMsgIndex(Menu_gameIdToSlot(i)), 0);
    }
}

void RPSelectGameLyt::Reset() {
    mpStateMachine->Reset(0);
    for (int i = 0; i < GAME_NUM; i++) {
        mpSlotStates[i]->Reset(2);
    }
    for (int i = 0; i < GAME_NUM; i++) {
        mpLockedStates[i]->Reset(0);
        mLockedRequest[i] = false;
        mLockedStop[i] = false;
    }
    mpExpState->Reset(0);
    mExpPending = false;
    mHighlighted = GAME_NUM;

    for (int i = 0; i < 2; i++) {
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->stop();
        mpAnmObjs[i]->setFrame(0.0f);
    }
    for (int i = 0; i < GAME_NUM; i++) {
        for (int j = 0; j < 3; j++) {
            mpSlotAnmObjs[i][j]->resetFrame();
            mpSlotAnmObjs[i][j]->stop();
            mpSlotAnmObjs[i][j]->setFrame(0.0f);
        }
    }
    for (int i = 0; i < GAME_NUM; i++) {
        for (int j = 0; j < 2; j++) {
            mpExpAnmObjs[i][j]->resetFrame();
            mpExpAnmObjs[i][j]->stop();
            mpExpAnmObjs[i][j]->setFrame(0.0f);
        }
    }
    mpScoreAnmObjs[0]->resetFrame();
    mpScoreAnmObjs[0]->stop();
    mpScoreAnmObjs[0]->setFrame(0.0f);

    mpLayout->unbindAllAnmObj();
    for (int i = 0; i < GAME_NUM; i++) {
        mpIconPanes[i][0]->UnbindAllAnimation(true);
    }
    for (int i = 0; i < GAME_NUM; i++) {
        mpExpPanes[i]->UnbindAllAnimation(true);
    }
    mpScoreTextBox->UnbindAllAnimation(true);

    for (int i = 0; i < GAME_NUM; i++) {
        mpIconPanes[i][1]->SetVisible(false);
    }
    for (int i = 0; i < GAME_NUM; i++) {
        mpExpPanes[i]->SetVisible(false);
    }
    mpScoreTextBox->setMessage(RPSelectScene::sInstance->mpMessage, 1000, 0, 0);

    for (int i = 0; i < GAME_NUM; i++) {
        mIconOffset[i] = EGG::Vector2f::zero;
        mIconVelocity[i] = EGG::Vector2f::zero;
        mIconTimer[i] = 0;
        mIconTimerEnd[i] = 0;
    }

    mDecidedGame = -1;
}

void RPSelectGameLyt::Calculate() {
    mpStateMachine->Calculate();
    for (int i = 0; i < GAME_NUM; i++) {
        mCurSlot = i;
        mpSlotStates[i]->Calculate();
    }
    for (int i = 0; i < GAME_NUM; i++) {
        mCurLocked = i;
        mpLockedStates[i]->Calculate();
    }
    mpExpState->Calculate();

    AnimateIcons();
    mpLayout->calc();

    for (int i = 0; i < GAME_NUM; i++) {
        if (mpIconPanes[i][1]->IsVisible()) {
            EGG::Vector2f pos = Lyt_getPaneGlobalPos(mpRankPanes[i]);
            for (int j = 0; j < EFFECT_NUM; j++) {
                mpMedalEffects[i][j]->setPos(nw4r::math::VEC3(pos.x, pos.y, 0.0f));
            }
        }
    }

    for (int i = 0; i < GAME_NUM; i++) {
        for (int j = 0; j < EFFECT_NUM; j++) {
            mpMedalEffects[i][j]->update();
        }
    }
}

void RPSelectGameLyt::EnterIn() {
    mpLayout->unbindAllAnmObj();
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[0], mpCenterPane, false);
    mpAnmObjs[0]->restart();
}

void RPSelectGameLyt::CalcIn() {
    if (mpAnmObjs[0]->isFinished()) {
        mpStateMachine->ChangeState(2);
    }
}

void RPSelectGameLyt::EnterIdle() {}

void RPSelectGameLyt::CalcIdle() {}

void RPSelectGameLyt::EnterDecide() {}

void RPSelectGameLyt::CalcDecide() {
    if (mpSlotStates[mDecidedGame]->GetState() == 5) {
        mpStateMachine->ChangeState(2);
    }
}

void RPSelectGameLyt::EnterOut() {
    mpLayout->unbindAllAnmObj();
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[1], mpCenterPane, false);
    mpAnmObjs[1]->restart();
}

void RPSelectGameLyt::CalcOut() {
    if (mpAnmObjs[1]->isFinished()) {
        mpStateMachine->ChangeState(5);
    }
}

void RPSelectGameLyt::Draw() {
    if (mpStateMachine->GetState() != 0 && mpStateMachine->GetState() != 5) {
        mpLayout->draw();
    }
}

void RPSelectGameLyt::Open() {
    if (mpStateMachine->IsStateSettled(0) || mpStateMachine->IsStateOrNext(5)) {
        mpStateMachine->ChangeState(1);
        mDecidedGame = -1;
    }
}

s32 RPSelectGameLyt::HitTestGame(const EGG::Vector2f& rPos) {
    for (int i = 0; i < GAME_NUM; i++) {
        if (mpSlotStates[i]->GetState() == 0 || mpSlotStates[i]->GetState() == 1 ||
            mpSlotStates[i]->GetState() == 3 || mpSlotStates[i]->GetState() == 4 ||
            mpSlotStates[i]->GetState() == 5) {
            RPSysLytBounding* pBounding = RPSysLayout::castToBounding(mpIconBoundings[i][1]);
            if (mUnlocked[i] && mpLayout->isInsideBounding(pBounding, rPos)) {
                return Menu_gameIdToSlot(i);
            }
        }
    }

    for (int i = 0; i < GAME_NUM; i++) {
        RPSysLytBounding* pBounding = RPSysLayout::castToBounding(mpIconBoundings[i][2]);
        if (mUnlocked[i] && mpLayout->isInsideBounding(pBounding, rPos)) {
            return Menu_gameIdToSlot(i);
        }
    }

    for (int i = 0; i < GAME_NUM; i++) {
        RPSysLytBounding* pBounding = RPSysLayout::castToBounding(mpIconBoundings[i][3]);
        if (!mUnlocked[i] && mpLayout->isInsideBounding(pBounding, rPos)) {
            return Menu_gameIdToSlot(i);
        }
    }

    return GAME_NUM;
}

s32 RPSelectGameLyt::GetNeighborGame(s32 slot, u32 button) {
    s32 dir;
    switch (button) {
    case 0x10008:
        dir = 0;
        break;
    case 0x20004:
        dir = 1;
        break;
    case 0x40001:
        dir = 2;
        break;
    case 0x80002:
        dir = 3;
        break;
    }

    s32 game = Menu_slotToGameId(slot);
    s32 neighbors[GAME_NUM][4] = {
        {1, -1, 1, 8},  {2, 0, -1, 0},  {3, 1, -1, 3},  {-1, 2, 2, 4},  {-1, -1, 3, 5},
        {-1, 6, 4, 6},  {5, 7, 5, -1},  {6, 8, 8, -1},  {7, -1, 0, 7},
    };
    return Menu_gameIdToSlot(neighbors[game][dir]);
}

bool RPSelectGameLyt::DecideGame(s32 slot, u32 playerFlag) {
    s32 game = Menu_slotToGameId(slot);
    if (!mpStateMachine->IsStateSettled(2)) {
        return false;
    }

    UnhighlightOthers(slot);
    if (mpStateMachine->IsStateOrNext(3) || mpStateMachine->IsStateOrNext(5)) {
        return true;
    }

    mDecidedGame = game;
    mDecidePlayerFlag = playerFlag;
    mpStateMachine->ChangeState(3);
    mpSlotStates[game]->ChangeState(4);
    return true;
}

bool RPSelectGameLyt::IsSelectable() const {
    bool selectable = false;
    if (mDecidedGame != -1 && mpStateMachine->IsStateSettled(2)) {
        selectable = true;
    }
    return selectable;
}

void RPSelectGameLyt::Close() {
    if (!mpStateMachine->IsStateSettled(4)) {
        switch (mpStateMachine->IsStateOrNext(5)) {
        case false:
            mpStateMachine->ChangeState(4);
            break;
        }
    }
}

void RPSelectGameLyt::PlayCursorSe(s32 slot, bool rumble) {
    EGG::Vector2f pos = GetGameIconPos(slot);
    pos.x /= static_cast<f32>(static_cast<u16>(EGG::Screen::GetSizeXMax() / 2));
    pos.y /= static_cast<f32>(static_cast<u16>(EGG::Screen::GetSizeYMax() / 2));
    RPSndAudioMgr::instance()->startPointCursorSe("RP_COMMON_SE_KEY_CURSOR_01", pos, 0.5f, 0);

    if (rumble) {
        RPSysCoreController::getActiveController()->rumble(
            RPSysCoreController::ERumblePattern_Cursor);
    }
}

EGG::Vector2f RPSelectGameLyt::GetPlayerPanePos(s32 playerNum, s32 player) {
    if (playerNum == 1) {
        return Lyt_getPaneGlobalPos(mpPlayerPosPanes[0]);
    }

    return Lyt_getPaneGlobalPos(mpPlayerPosPanes[1 + player]);
}

f32 RPSelectGameLyt::GetAnmLength(s32 idx) {
    return mpAnmObjs[idx]->getFrameMax();
}

void RPSelectGameLyt::StartMedalEffects(s32 game) {
    s32 medal = mMedals[game];
    if (medal != 0 && mPlayerNum == 1) {
        s32 start, end;
        switch (medal) {
        case 1:
            start = 0;
            end = 0;
            break;
        case 2:
            start = 1;
            end = 1;
            break;
        case 3:
            start = 2;
            end = 3;
            break;
        case 4:
            start = 4;
            end = 6;
            break;
        }

        for (s32 i = start; i <= end; i++) {
            mpMedalEffects[game][i]->create();
        }
    }
}

void RPSelectGameLyt::StopMedalEffects(s32 game) {
    for (int i = 0; i < EFFECT_NUM; i++) {
        mpMedalEffects[game][i]->forceKill();
    }
}

// The state machine instantiation in original address order.
#pragma defer_codegen off
#pragma auto_inline off
template RPStateMachine<RPSelectGameLyt>::RPStateMachine(RPSelectGameLyt*, s32);
template void RPStateMachine<RPSelectGameLyt>::SetStateFuncs(const StateFunc&, const StateFunc&,
                                                             s32);
template void RPStateMachine<RPSelectGameLyt>::Reset(s32);
template void RPStateMachine<RPSelectGameLyt>::ChangeState(s32);
template void RPStateMachine<RPSelectGameLyt>::Calculate();
template void RPStateMachine<RPSelectGameLyt>::CallExit();
template void RPStateMachine<RPSelectGameLyt>::CallCalc();
template void RPStateMachine<RPSelectGameLyt>::CallEnter();
template bool RPStateMachine<RPSelectGameLyt>::IsStateSettled(s32) const;
template bool RPStateMachine<RPSelectGameLyt>::IsStateOrNext(s32) const;

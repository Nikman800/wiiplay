// Fishing score panels (RPFshScoreLayout): per player the score, fish count and Mii icons
// (fishingScore_00.brlyt), one state machine per panel.
// Unit 0x8020FEE8-0x8021172C (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPFshScene/RPFshScoreLayout.h>

#include <Pack/RPAudio.h>
#include <Pack/RPGraphics.h>
#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPFshScene/RPFshFish.h>
#include <Pack/RPParty/RPFshScene/RPFshGame.h>
#include <Pack/RPParty/RPFshScene/RPFshPlayer.h>
#include <Pack/RPParty/RPFshScene/RPFshPlayerMgr.h>
#include <Pack/RPParty/RPFshScene/RPFshResultLayout.h>
#include <Pack/RPParty/RPFshScene/RPFshSound.h>
#include <Pack/RPParty/RPPartyCommon/RPCls803572E0.h>
#include <Pack/RPParty/RPPartyCommon/RPLytObjBase.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>
#include <Pack/RPSystem.h>

#include <egg/math.h>

namespace {

struct ScoreRange {
    s32 min; // at 0x0
    s32 max; // at 0x4
};

struct ScoreRanges {
    ScoreRange ranges[9]; // at 0x0
};

// Value ranges, then tables this file does not read (0x100 bytes, 8031FB58)
struct ScoreParam {
    ScoreRanges ranges; // at 0x0
    s32 tables[46];     // at 0x48
};

} // namespace

static const ScoreParam sScoreParam = {
    {{{-999, 9999},
      {0, 999},
      {0, 999},
      {0, 9999},
      {0, 999},
      {-99, 99},
      {-999, 9999},
      {0, 999},
      {0, 999}}},
    {200, 320, 450, 600, 20,   40,   60,   80,   40,  70,  100, 200, 150, 400, 800, 1200,
     3,   7,   10,  14,  15,   27,   51,   60,   1000, 1600, 2300, 2600, 170, 250, 300, 325,
     11,  32,  84,  125, 0,    1,    2,    3,    4,   5,   7,   8,   9,   0},
};

inline ScoreRanges GetScoreRanges() {
    // NONMATCHING-DEBT: copies the range prefix through a cast of the whole object's address.
    // sScoreParam.ranges (returned directly, through a const reference or a pointer local)
    // lets the optimizer hoist the copy out of UpdateTexts' loop (200/294).
    ScoreRanges ranges = *reinterpret_cast<const ScoreRanges*>(&sScoreParam);
    return ranges;
}

inline s32 ClampScore(s32 x, s32 min, s32 max) {
    if (x < min) {
        x = min;
    } else if (x > max) {
        x = max;
    }
    return x;
}

inline s32 GetScoreMin(ScoreRanges ranges) {
    return ranges.ranges[6].min;
}

inline s32 GetScoreMax(ScoreRanges ranges) {
    return ranges.ranges[6].max;
}

inline s32 ClampTotalScore(s32 total) {
    ScoreRanges ranges = GetScoreRanges();
    return ClampScore(total, GetScoreMin(ranges), GetScoreMax(ranges));
}

s32 RPFshScoreLayout::sScoreCountStep = 2;
s32 RPFshScoreLayout::sPointsUnit = 10;

RPFshScoreLayout* RPFshScoreLayout::sInstance;

void RPFshScoreLayout::CreateInstance() {
    sInstance = new RPFshScoreLayout();
}

void RPFshScoreLayout::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPFshScoreLayout::~RPFshScoreLayout() {}

RPFshScoreLayout::RPFshScoreLayout() {
    for (int i = 0; i < PLAYER_MAX; i++) {
        mpStateMachines[i] = new RPStateMachine<RPFshScoreLayout>(this, STATE_NUM);

        RPFshStateFuncs<RPFshScoreLayout> funcs[STATE_NUM] = {
            {&RPFshScoreLayout::HiddenEnter, &RPFshScoreLayout::HiddenCalc},
            {&RPFshScoreLayout::InEnter, &RPFshScoreLayout::InCalc},
            {&RPFshScoreLayout::ShownEnter, &RPFshScoreLayout::ShownCalc},
            {&RPFshScoreLayout::GetEnter, &RPFshScoreLayout::GetCalc},
            {&RPFshScoreLayout::OutEnter, &RPFshScoreLayout::OutCalc},
        };
        for (int j = 0; j < STATE_NUM; j++) {
            mpStateMachines[i]->SetStateFuncs(funcs[j].mEnter, funcs[j].mCalc, j);
        }
    }

    mpResAccessor = RPSysLytResAccessor::create(NULL);
    mPlayerNum = RPFshGame::GetPlayerNum();

    for (int i = 0; i < mPlayerNum; i++) {
        RPSysKokeshiGenInfo info;
        GXColor bgColor = {0, 0, 0, 0};

        info.SetGenType(RPSysKokeshiManager::GenType_Player);
        info.SetPlayerNo(i);
        info.SetIconWidth(0x60);
        info.SetIconHeight(0x60);
        info.SetIconBGType(RFLIconBG_Direct);
        info.SetIconBGColor(bgColor);
        info.SetIconMipMap(true);

        mpIcons[i] = new RPSysKokeshiIcon(&info);
    }

    RPGrpRenderer::GetCurrent()->AppendDrawObject(&static_cast<IRPGrpDrawObject&>(*this));
}

// Only the string-pool symbol names differ in the function comparison.
void RPFshScoreLayout::Create() {
    RPLytObjBase::AttachArcFile(RPSysResourceManager::LoadGameLocalArchive(), "fishing.arc",
                                mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "fishingScore_00.brlyt");

    const char* anmNames[3] = {"fishingScore_00_inAll_00.brlan",
                               "fishingScore_00_outAll_00.brlan", "fishingScore_00_Get_00.brlan"};
    for (int i = 0; i < PLAYER_MAX; i++) {
        for (int j = 0; j < 3; j++) {
            mpAnmObjs[i][j] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[j]);
        }
    }

    const char* panelNames[PLAYER_MAX] = {"N_L_00", "N_R_00"};
    for (int i = 0; i < PLAYER_MAX; i++) {
        mpPanels[i] = mpLayout->findPane(panelNames[i]);
    }

    const char* scoreTextNames[PLAYER_MAX] = {"T_scoreL_00", "T_scoreR_00"};
    for (int i = 0; i < PLAYER_MAX; i++) {
        mpScoreTexts[i] = mpLayout->findTextBox(scoreTextNames[i]);
    }

    const char* scorePaneNames[PLAYER_MAX] = {"N_scoreL_00", "N_scoreR_00"};
    for (int i = 0; i < PLAYER_MAX; i++) {
        mpScorePanes[i] = mpLayout->findPane(scorePaneNames[i]);
    }

    const char* fishTextNames[PLAYER_MAX][2] = {{"T_fishL_00", "T_fishL_01"},
                                                {"T_fishR_00", "T_fishR_01"}};
    for (int i = 0; i < PLAYER_MAX; i++) {
        for (int j = 0; j < 2; j++) {
            mpFishTexts[i][j] = mpLayout->findTextBox(fishTextNames[i][j]);
        }
    }

    const char* faceNames[PLAYER_MAX][2] = {{"N_playerFaceL_00", "N_playerFaceL_01"},
                                            {"N_playerFaceR_00", "N_playerFaceR_01"}};
    for (int i = 0; i < PLAYER_MAX; i++) {
        for (int j = 0; j < 2; j++) {
            mpFacePanes[i][j] = mpLayout->findPane(faceNames[i][j]);
        }
    }

    RPSysMessage* pMessage = RPCls803572E0::GetInstance()->GetMessage();
    const char* playerTextNames[PLAYER_MAX] = {"T_playerL_00", "T_playerR_00"};
    for (int i = 0; i < PLAYER_MAX; i++) {
        mpLayout->findTextBox(playerTextNames[i])->setMessage(pMessage, 0x806, i, 0);
    }

    for (int i = 0; i < mPlayerNum; i++) {
        mpIcons[i]->LoadResource();
    }

    for (int i = 0; i < PLAYER_MAX; i++) {
        if (i < mPlayerNum) {
            mpPanels[i]->SetVisible(true);
        } else {
            mpPanels[i]->SetVisible(false);
        }
    }
}

inline void RPFshScoreLayout::UpdateFishCount(s32 player) {
    wchar_t text[10];
    RPSysStringUtility::GetStringHalfSizeNumber(mDispFishCount[player], text, 10, false);
    for (int j = 0; j < 2; j++) {
        mpFishTexts[player][j]->setText(text, 0);
    }
}

void RPFshScoreLayout::Reset() {
    for (int i = 0; i < PLAYER_MAX; i++) {
        mpStateMachines[i]->Reset(STATE_HIDDEN);
        mShowRequest[i] = false;
        mHideRequest[i] = false;
    }

    for (int i = 0; i < PLAYER_MAX; i++) {
        mDispScore[i] = 0;
        mScoreDir[i] = 0;
        mFishCount[i] = 0;
        mDispFishCount[i] = 0;

        if (i < mPlayerNum) {
            UpdateFishCount(i);
        }
    }

    for (int i = 0; i < PLAYER_MAX; i++) {
        for (int j = 0; j < 3; j++) {
            mpAnmObjs[i][j]->resetFrame();
            mpAnmObjs[i][j]->stop();
            mpAnmObjs[i][j]->setFrame(0.0f);
        }
    }

    mpLayout->unbindAllAnmObj();

    for (int i = 0; i < PLAYER_MAX; i++) {
        mpPanels[i]->UnbindAllAnimation(true);
    }
}

void RPFshScoreLayout::Calculate() {
    for (int i = 0; i < PLAYER_MAX; i++) {
        mCurPlayer = i;
        mpStateMachines[i]->Calculate();
    }

    UpdateTexts();

    bool shown = false;
    for (int i = 0; i < PLAYER_MAX; i++) {
        if (mpStateMachines[i]->GetState() != STATE_HIDDEN) {
            shown = true;
        }
    }

    if (shown) {
        mpLayout->calc();
    }
}

void RPFshScoreLayout::HiddenEnter() {}

void RPFshScoreLayout::HiddenCalc() {
    if (mShowRequest[mCurPlayer]) {
        mpStateMachines[mCurPlayer]->ChangeState(STATE_IN);
    }
}

void RPFshScoreLayout::InEnter() {
    mShowRequest[mCurPlayer] = false;
    mpPanels[mCurPlayer]->UnbindAllAnimation(true);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[mCurPlayer][0], mpPanels[mCurPlayer], true);
    mpAnmObjs[mCurPlayer][0]->restart();
}

void RPFshScoreLayout::InCalc() {
    if (mpAnmObjs[mCurPlayer][0]->isFinished()) {
        mpStateMachines[mCurPlayer]->ChangeState(STATE_SHOWN);
    }
}

void RPFshScoreLayout::ShownEnter() {}

void RPFshScoreLayout::ShownCalc() {
    if (mHideRequest[mCurPlayer]) {
        mpStateMachines[mCurPlayer]->ChangeState(STATE_OUT);
    } else if (mFishCount[mCurPlayer] != mDispFishCount[mCurPlayer]) {
        mpStateMachines[mCurPlayer]->ChangeState(STATE_GET);
    }
}

void RPFshScoreLayout::GetEnter() {
    mpScorePanes[mCurPlayer]->UnbindAllAnimation(true);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[mCurPlayer][2], mpScorePanes[mCurPlayer], true);
    mpAnmObjs[mCurPlayer][2]->restart();
}

void RPFshScoreLayout::GetCalc() {
    f32 diff = EGG::Math<f32>::abs(mpAnmObjs[mCurPlayer][2]->getFrameCtrl()->getFrame() - 21.0f);

    if (diff < 0.5f) {
        mDispFishCount[mCurPlayer] = mFishCount[mCurPlayer];

        UpdateFishCount(mCurPlayer);
    } else if (mpAnmObjs[mCurPlayer][2]->isFinished()) {
        mpStateMachines[mCurPlayer]->ChangeState(STATE_SHOWN);
    }
}

void RPFshScoreLayout::OutEnter() {
    mHideRequest[mCurPlayer] = false;
    mpPanels[mCurPlayer]->UnbindAllAnimation(true);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[mCurPlayer][1], mpPanels[mCurPlayer], true);
    mpAnmObjs[mCurPlayer][1]->restart();
}

void RPFshScoreLayout::OutCalc() {
    if (mpAnmObjs[mCurPlayer][1]->isFinished()) {
        mpStateMachines[mCurPlayer]->ChangeState(STATE_HIDDEN);
    }
}

inline s32 CalcPlayerTotalPoints(s32 player) {
    RPFshPlayerMgr* pPlayerMgr = RPFshPlayerMgr::GetInstance();
    s32 total = 0;
    RPFshPlayer* pPlayer = pPlayerMgr->GetPlayer(player);
    for (int i = 0; i < pPlayer->GetCaughtNum(); i++) {
        const RPFshCaughtFish& rFish = pPlayer->GetCaughtFish(i);
        total += rFish.mBonus * RPFshScoreLayout::CalcFishPoints(rFish.mType, rFish.mSize);
    }

    ScoreRanges ranges = GetScoreRanges();
    return ClampScore(total, GetScoreMin(ranges), GetScoreMax(ranges));
}

void RPFshScoreLayout::UpdateTexts() {
    s32 totals[PLAYER_MAX];
    for (int i = 0; i < mPlayerNum; i++) {
        totals[i] = CalcPlayerTotalPoints(i);
    }

    s32 prevDirs[PLAYER_MAX];
    for (int i = 0; i < mPlayerNum; i++) {
        prevDirs[i] = mScoreDir[i];
        mScoreDir[i] = 0;
    }

    for (int i = 0; i < mPlayerNum; i++) {
        if (mDispScore[i] < totals[i]) {
            mDispScore[i] += sScoreCountStep;
            mScoreDir[i] = 1;
            if (mDispScore[i] > totals[i]) {
                mDispScore[i] = totals[i];
            }
        }

        if (mDispScore[i] > totals[i]) {
            mDispScore[i] -= sScoreCountStep;
            mScoreDir[i] = -1;
            if (mDispScore[i] < totals[i]) {
                mDispScore[i] = totals[i];
            }
        }
    }

    if (RPFshResultLayout::GetInstance()->GetState() != RPFshResultLayout::STATE_NONE) {
        for (int i = 0; i < mPlayerNum; i++) {
            mDispScore[i] = totals[i];
        }
    }

    for (int i = 0; i < mPlayerNum; i++) {
        if (mScoreDir[i] != 0) {
            if (i == 0) {
                if (mScoreDir[i] == 1) {
                    RPFshHoldGlobalSound(0x24);
                } else {
                    RPFshHoldGlobalSound(0x26);
                }
            } else {
                if (mScoreDir[i] == 1) {
                    RPFshHoldGlobalSound(0x25);
                } else {
                    RPFshHoldGlobalSound(0x27);
                }
            }
        } else if (prevDirs[i] != 0) {
            if (i == 0) {
                RPSndAudioMgr::instance()->startSystemSe(0x29U);
            } else {
                RPSndAudioMgr::instance()->startSystemSe(0x2AU);
            }
        }
    }

    for (int i = 0; i < mPlayerNum; i++) {
        wchar_t text[10];
        RPSysStringUtility::GetStringFullSizeNumber(mDispScore[i], text, 10, false);
        mpScoreTexts[i]->setText(text, 0);
    }
}

s32 RPFshScoreLayout::CalcFishPoints(s32 type, f32 size) {
    const RPFshFishTypeParam* pParam = RPFshFish::GetTypeParam(type);
    if (type == RPFshFish::TYPE_SUKA_A) {
        return pParam->mPoints;
    }

    s32 points = pParam->mPoints * EGG::Math<f32>::sqrt(size);
    points -= points % sPointsUnit;
    return points + sPointsUnit;
}

s32 RPFshScoreLayout::CalcTotalPoints(s32 player) {
    RPFshPlayerMgr* pPlayerMgr = RPFshPlayerMgr::GetInstance();
    RPFshPlayer* pPlayer = pPlayerMgr->GetPlayer(player);
    s32 total = 0;
    for (int i = 0; i < pPlayer->GetCaughtNum(); i++) {
        const RPFshCaughtFish& rFish = pPlayer->GetCaughtFish(i);
        total += rFish.mBonus * CalcFishPoints(rFish.mType, rFish.mSize);
    }

    return ClampTotalScore(total);
}

void RPFshScoreLayout::RequestShow(s32 player) {
    mShowRequest[player] = true;
    mHideRequest[player] = false;
}

void RPFshScoreLayout::RequestHideAll() {
    for (int i = 0; i < mPlayerNum; i++) {
        RequestHide(i);
    }
}

void RPFshScoreLayout::RequestHide(s32 player) {
    mHideRequest[player] = true;
    mShowRequest[player] = false;
}

void RPFshScoreLayout::AddFishCount(s32 player) {
    mFishCount[player]++;
}

void RPFshScoreLayout::UserDraw() {
    if (RPGrpRenderer::GetCurrentViewNo() == 4 && RPGrpRenderer::GetDrawStep() == 5) {
        bool shown = false;
        for (int i = 0; i < PLAYER_MAX; i++) {
            if (mpStateMachines[i]->GetState() != STATE_HIDDEN) {
                shown = true;
            }
        }

        if (shown) {
            RPSysLayout::drawInit();
            mpLayout->draw();

            for (int i = 0; i < mPlayerNum; i++) {
                RPDrawKokeshiIcon(mpIcons[i], mpFacePanes[i][1], TRUE, NULL);
            }
            for (int i = 0; i < mPlayerNum; i++) {
                RPDrawKokeshiIcon(mpIcons[i], mpFacePanes[i][0], FALSE, NULL);
            }
        }
    }
}

// Wii Play: emit the state helpers in address order, followed by the exit stub.
#pragma defer_codegen off
#pragma auto_inline off
template RPStateMachine<RPFshScoreLayout>::RPStateMachine(RPFshScoreLayout*, s32);
template void RPStateMachine<RPFshScoreLayout>::SetStateFuncs(const StateFunc&, const StateFunc&, s32);
template void RPStateMachine<RPFshScoreLayout>::Reset(s32);
template void RPStateMachine<RPFshScoreLayout>::ChangeState(s32);
template void RPStateMachine<RPFshScoreLayout>::Calculate();
template void RPStateMachine<RPFshScoreLayout>::CallEnter();
template void RPStateMachine<RPFshScoreLayout>::CallCalc();
template void RPStateMachine<RPFshScoreLayout>::CallExit();

void RPFshScoreLayout::Exit() {}

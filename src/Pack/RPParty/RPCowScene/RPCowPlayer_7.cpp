// Charge! two-player cursor (RPCowPlayerCursor): shows where the other player's cow is on the
// split screen. Unit 0x802AF810-0x802AFF64 (dtk/game_units.tsv). Functions in reverse address
// order.
#include <Pack/RPParty/RPCowScene/RPCowPlayer_7.h>

#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPParty/RPCowScene/RPCowCamera.h>
#include <Pack/RPParty/RPCowScene/RPCowGame.h>
#include <Pack/RPParty/RPPartyCommon/RPCls803572E0.h>
#include <egg/gfxe/eggScreen.h>

extern "C" void RP_approachStep(f32* pValue, f32* pVel, f32 target, f32 rate);
extern "C" const EGG::Vector3f RP_eggVecZero;

namespace {
inline RPCowCamera* GetCamera(RPCowGame* pGame, u16 idx) {
    return &static_cast<RPCowCamera*>(pGame->GetSlotActor(0))[idx];
}
inline RPCowPlayer* GetPlayer(RPCowGame* pGame, u16 idx) {
    return &static_cast<RPCowPlayer*>(pGame->GetSlotActor(7))[idx];
}
inline bool IsPlaying(RPCowGame* pGame) {
    bool playing = true;
    if (pGame->GetState() != 7 && pGame->GetState() != 8) {
        playing = false;
    }
    return playing;
}
}

const f32 RPCowPlayerCursor::sOfsY = 12.0f;
const f32 RPCowPlayerCursor::sMarginX = 34.0f;
const f32 RPCowPlayerCursor::sNearDepth = 40.0f;

void RPCowPlayerCursor::Init(s32 player, s32 target) {
    RPCowGame* pGame = RPCowGame::GetInstance();
    mTarget = target;
    mPlayer = player;
    mpLayout = RPSysLayout::create(NULL, pGame->GetLytAccessor(), "cowCursor_00.brlyt");
    const char* anm[] = {"cowCursor_00_cursor1p_00.brlan", "cowCursor_00_cursor2p_00.brlan"};
    for (int i = 0; i < 2; i++) {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, pGame->GetLytAccessor(), anm[i]);
    }
    const char* pane[] = {"N_cursor_00", "N_cursor_01"};
    for (int i = 0; i < 2; i++) {
        mpPanes[i] = mpLayout->findPane(pane[i]);
        mpPanes[i]->SetVisible(false);
    }
    RPSysMessage* pMsg = RPCls803572E0::GetInstance()->GetMessage();
    mpLayout->findTextBox("T_cursor00_00")->setMessage(pMsg, 0xA8D, 0, 0);
    mpLayout->findTextBox("T_cursor01_00")->setMessage(pMsg, 0xA8D, 0, 0);
    mpLayout->findTextBox("T_cursor00_01")->setMessage(pMsg, 0xA8D, 1, 0);
    mpLayout->findTextBox("T_cursor01_01")->setMessage(pMsg, 0xA8D, 1, 0);
}

RPCowPlayerCursor::RPCowPlayerCursor() {}

RPCowPlayerCursor::~RPCowPlayerCursor() {}

void RPCowPlayerCursor::Reset() {
    RPCowGame* pGame = RPCowGame::GetInstance();
    mTimer = 0;
    mScale = 0.0f;
    mScaleVel = 0.0f;
    RPCowPlayer* pPlayer = GetPlayer(pGame, mTarget);
    mPos.x = pPlayer->GetPos().x;
    mPos.y = GetOfsY() + pPlayer->GetPos().y;
    mPos.z = pPlayer->GetPos().z;
    mVel = RP_eggVecZero;
    for (int i = 0; i < 2; i++) {
        mpPanes[i]->SetVisible(false);
    }
    mpLayout->unbindAllAnmObj();
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[0], mpPanes[0], true);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[1], mpPanes[1], true);
    mpAnmObjs[0]->resetFrame();
    mpAnmObjs[1]->resetFrame();
}

#pragma auto_inline off
extern "C" u16 Cow_getHalfScreenWidth() {
    return EGG::Screen::GetSizeXMax() / 2;
}

extern "C" u16 Cow_getHalfScreenHeight() {
    return EGG::Screen::GetSizeYMax() / 2;
}
#pragma auto_inline reset

void RPCowPlayerCursor::Calc() {
    RPCowGame* pGame = RPCowGame::GetInstance();
    RPCowCamera* pCamera = GetCamera(pGame, mPlayer);
    RPCowPlayer* pPlayer = GetPlayer(pGame, mTarget);
    f32 halfW = Cow_getHalfScreenWidth();
    f32 halfH = Cow_getHalfScreenHeight() / 2.0f;

    EGG::Vector3f pos = pPlayer->GetPos();
    pos.y += GetOfsY();
    EGG::Vector3f back = pCamera->GetBackDir();
    EGG::Vector3f eye = pCamera->GetEyePos();
    EGG::Vector3f diff = pos - eye;
    f32 depth = back.dot(diff);
    EGG::Vector3f screen = pCamera->ProjectToScreenClamped(pos, depth);
    pos.x = screen.x;
    pos.y = screen.y;
    pos.z = 0.0f;

    if (mTimer > 0) {
        RP_approachStep(&mPos.x, &mVel.x, pos.x, 0.2f);
        RP_approachStep(&mPos.y, &mVel.y, pos.y, 0.2f);
        RP_approachStep(&mPos.z, &mVel.z, pos.z, 0.2f);
    } else {
        mPos = pos;
        mVel = RP_eggVecZero;
    }

    if (mPos.x > halfW - GetMarginX()) {
        mPos.x = halfW - GetMarginX();
    } else {
        f32 minX = -halfW;
        minX += GetMarginX();
        if (mPos.x < minX) {
            mPos.x = minX;
        }
    }
    if (mPos.y <= -halfH) {
        mPos.y = -halfH;
    }

    if (IsPlaying(pGame) && depth <= GetNearDepth()) {
        mTimer = 30;
        if (mScale <= 0.0f) {
            mScaleVel = 0.2f;
        }
    }

    if (mTimer > 0) {
        if (mScale > 1.0f || mScaleVel > 0.0f || mScaleVel < 0.0f) {
            mScale += mScaleVel;
            mScaleVel -= 0.02f;
            if (mScale <= 1.0f && mScaleVel <= 0.0f) {
                mScale = 1.0f;
                mScaleVel = 0.0f;
            }
        } else {
            mTimer--;
        }
    } else if (mScale > 0.0f) {
        mScale += mScaleVel;
        mScaleVel -= 0.02f;
        if (mScale <= 0.0f && mScaleVel <= 0.0f) {
            mScale = 0.0f;
            mScaleVel = 0.0f;
        }
    }

    if (mScale > 0.0f) {
        mpPanes[mTarget]->SetVisible(true);
        mpPanes[mTarget]->SetScale(nw4r::math::VEC2(mScale, mScale));
    } else {
        mpPanes[mTarget]->SetVisible(false);
    }
    mpLayout->calc();
}

void RPCowPlayerCursor::Draw() {
    mpPanes[mTarget]->SetTranslate(mPos);
    mpLayout->draw();
}

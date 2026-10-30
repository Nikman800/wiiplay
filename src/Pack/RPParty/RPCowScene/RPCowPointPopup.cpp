// Charge! score popup (RPCowPointPopup): the points of a knocked object, shown over it on the
// scoring player's screen (and smaller on the other player's). Unit 0x802AE014-0x802AF030
// (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPCowScene/RPCowPointPopup.h>

#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPKernel/RPSysStringUtility.h>
#include <Pack/RPParty/RPCowScene/RPCowCamera.h>
#include <Pack/RPParty/RPCowScene/RPCowGame.h>
#include <Pack/RPParty/RPCowScene/RPCowPlayer.h>
#include <Pack/RPParty/RPPartyCommon/RPCollisionShape.h>

namespace {
inline RPCowCamera* GetCamera(RPCowGame* pGame, s32 idx) {
    return &static_cast<RPCowCamera*>(pGame->GetSlotActor(0))[idx];
}
inline RPCowPlayer* GetPlayer(RPCowGame* pGame, u16 idx) {
    return &static_cast<RPCowPlayer*>(pGame->GetSlotActor(7))[idx];
}
inline void SetRotX(RPCollisionShape* pShape, f32 x) {
    f32 v = x;
    pShape->GetRot().x = v;
}
inline void SetRotY(RPCollisionShape* pShape, f32 y) {
    f32 v = y;
    pShape->GetRot().y = v;
}
inline void SetRotZ(RPCollisionShape* pShape, f32 z) {
    f32 v = z;
    pShape->GetRot().z = v;
}
}

// Access to RPCollisionShape's protected radius members
struct RPCowPopupShape : public RPCollisionShape {
    f32& Radius() {
        return mRadius;
    }
    f32& RadiusSq() {
        return unk74;
    }
};

namespace {
inline bool IsStarted(RPActor* pActor) {
    return pActor->GetFlags().mActivate; // activate pending or active
}
inline RPCowPointPopup* FindFreePopup(RPCowGame* pGame, s32 start) {
    s32 num = pGame->GetSlotNum(17);
    if (start >= num) {
        start = 0;
    }
    RPCowPointPopup* pTable = static_cast<RPCowPointPopup*>(pGame->GetSlotActor(17));
    s32 i = start;
    do {
        if (!IsStarted(&pTable[i])) {
            return &pTable[i];
        }
        i++;
        i = (i < num) ? i : 0;
    } while (i != start);
    return NULL;
}
}

void RPCowPointPopup::Spawn(EGG::Vector3f pos, s32 player, s32 points, s32 perfect) {
    RPCowGame* pGame = RPCowGame::GetInstance();
    wchar_t text[4];
    RPSysStringUtility::GetStringFullSizeNumber(points, text, 4, true);

    if (perfect <= 0) {
        RPCowPointPopup* p = FindFreePopup(pGame, 0);
        if (p != NULL) {
            RPCowCamera* pCamera = GetCamera(pGame, (u16)player);
            pGame->StartActor(p);
            p->mScreen = player;
            p->mPlayer = player;
            p->mPoints = points;
            p->mPerfect = perfect;
            p->mTimer = 35;
            p->unkC8 = 0.0f;
            p->unkCC = perfect ? sUnkCCPerfect : sUnkCCNormal;
            p->unkD0 = perfect ? sUnkD0Perfect : sUnkD0Normal;
            p->mScale = perfect ? sScalePerfect : (points > 1 ? 1.2f : sScaleNormal);
            p->mpCollision->GetPos() = pos;
            f32 ofsY = perfect ? sOfsYPerfect : sOfsYNormal;
            p->mpCollision->GetPos().y += ofsY;
            EGG::Vector3f screenPos = pCamera->ProjectToScreen(p->mpCollision->GetPos());
            p->mpCollision->GetPos() = screenPos;
            f32 v;
            v = 0.0f;
            p->mpCollision->GetRot().x = v;
            v = perfect ? sRisePerfect : sRiseNormal;
            p->mpCollision->GetRot().y = v;
            v = 0.0f;
            p->mpCollision->GetRot().z = v;
            p->mRiseSpeed = -0.2f;
            RPCowPopupShape* pShape = static_cast<RPCowPopupShape*>(p->mpCollision);
            pShape->Radius() = perfect ? sRadiusPerfect : sRadiusNormal;
            pShape->RadiusSq() = (perfect ? sRadiusPerfect : sRadiusNormal) *
                            (perfect ? sRadiusPerfect : sRadiusNormal);
            if (points > 1) {
                p->mAnmIdx = 2;
                s32 pane = 7;
                if (player != 0) {
                    pane = 8;
                }
                p->mPaneIdx = pane;
            } else {
                p->mAnmIdx = 0;
                s32 pane = 1;
                if (player != 0) {
                    pane = 2;
                }
                p->mPaneIdx = pane;
            }
            p->mpLayout->unbindAnmObj(p->mpAnmObjs[p->mAnmIdx]);
            RPSysLayout::bindAnmObjToPane(p->mpAnmObjs[p->mAnmIdx], p->mpPanes[p->mPaneIdx], true);
            p->mpPanes[p->mPaneIdx]->SetVisible(true);
            p->mpAnmObjs[p->mAnmIdx]->restart();
            p->mpTextBoxes[0]->setText(text, 0);
            p->mpTextBoxes[1]->setText(text, 0);
            p->mpTextBoxes[4]->setText(text, 0);
            p->mpTextBoxes[5]->setText(text, 0);
        }
    }

    if (pGame->GetPlayerNum() >= 2) {
        RPCowPointPopup* p = FindFreePopup(pGame, 0);
        if (p != NULL) {
            s32 screen = player == 0;
            RPCowCamera* pCamera = GetCamera(pGame, screen);
            RPCowPlayer* pPlayer = GetPlayer(pGame, player);
            EGG::Vector3f back = pCamera->GetBackDir();
            EGG::Vector3f playerPos = pPlayer->GetPos();
            EGG::Vector3f eye = pCamera->GetEyePos();
            EGG::Vector3f diff = playerPos - eye;
            f32 depth = back.dot(diff);
            f32 scale = (sFarDist - depth) / (sFarDist - sNearDist);
            if (depth > 0.0f && scale > 0.01f) {
                pGame->StartActor(p);
                p->mScreen = screen;
                p->mPlayer = player;
                p->mPoints = points;
                p->mPerfect = perfect;
                p->mTimer = 35;
                p->unkC8 = 0.0f;
                p->unkCC = perfect ? sUnkCCPerfect : sUnkCCNormal;
                p->unkD0 = perfect ? sUnkD0Perfect : sUnkD0Normal;
                p->mScale = scale * (perfect ? sScalePerfect : (points > 1 ? 1.2f : sScaleNormal));
                p->mpCollision->GetPos() = pos;
                f32 ofsY = perfect ? sOfsYPerfect : sOfsYNormal;
                p->mpCollision->GetPos().y += scale * ofsY;
                p->mpCollision->GetPos() = pCamera->ProjectToScreen(p->mpCollision->GetPos());
                SetRotX(p->mpCollision, 0.0f);
                f32 rise = scale * (perfect ? sRisePerfect : sRiseNormal);
                SetRotY(p->mpCollision, rise);
                SetRotZ(p->mpCollision, 0.0f);
                f32 speed = -0.2f;
                p->mRiseSpeed = scale * speed;
                RPCowPopupShape* pShape = static_cast<RPCowPopupShape*>(p->mpCollision);
                pShape->Radius() = perfect ? sRadiusPerfect * scale : sRadiusNormal * scale;
                pShape->RadiusSq() = (perfect ? sRadiusPerfect * scale : sRadiusNormal * scale) *
                                     (perfect ? sRadiusPerfect * scale : sRadiusNormal * scale);
                if (perfect > 0) {
                    p->mAnmIdx = 4;
                    s32 pane = 13;
                    if (player != 0) {
                        pane = 14;
                    }
                    p->mPaneIdx = pane;
                } else if (points > 1) {
                    p->mAnmIdx = 3;
                    s32 pane = 10;
                    if (player != 0) {
                        pane = 11;
                    }
                    p->mPaneIdx = pane;
                } else {
                    p->mAnmIdx = 1;
                    s32 pane = 4;
                    if (player != 0) {
                        pane = 5;
                    }
                    p->mPaneIdx = pane;
                }
                p->mpLayout->unbindAnmObj(p->mpAnmObjs[p->mAnmIdx]);
                RPSysLayout::bindAnmObjToPane(p->mpAnmObjs[p->mAnmIdx], p->mpPanes[p->mPaneIdx],
                                              true);
                p->mpPanes[p->mPaneIdx]->SetVisible(true);
                p->mpAnmObjs[p->mAnmIdx]->restart();
                p->mpTextBoxes[2]->setText(text, 0);
                p->mpTextBoxes[3]->setText(text, 0);
                p->mpTextBoxes[6]->setText(text, 0);
                p->mpTextBoxes[7]->setText(text, 0);
                p->mpTextBoxes[8]->setText(text, 0);
                p->mpTextBoxes[9]->setText(text, 0);
            }
        }
    }
}

bool RPCowPointPopup::IsEnabled(IRPActorPriority* pOther) {
    RPCowPointPopup* p = static_cast<RPCowPointPopup*>(pOther);
    if (p != this && p->mScreen == mScreen) {
        return true;
    }
    return false;
}

RPCowPointPopup::RPCowPointPopup() {}

RPCowPointPopup::~RPCowPointPopup() {}

void RPCowPointPopup::LoadResource() {
    RPCowGame* pGame = RPCowGame::GetInstance();
    mpLayout = RPSysLayout::create(NULL, pGame->GetLytAccessor(), "cowPoint_00.brlyt");
    const char* anm[] = {
        "cowPoint_00_pointL_00.brlan",  "cowPoint_00_pointS_00.brlan",
        "cowPoint_00_pointXL_00.brlan", "cowPoint_00_pointXS_00.brlan",
        "cowPoint_00_PerfectS_00.brlan",
    };
    for (int i = 0; i < 5; i++) {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, pGame->GetLytAccessor(), anm[i]);
    }
    const char* pane[] = {
        "N_PointL_00",   "N_tPointL_00",   "N_tPointL_01",  "N_PointS_00",   "N_tPointS_00",
        "N_tPointS_01",  "N_PointXL_00",   "N_tPointXL_00", "N_tPointXL_01", "N_PointXS_00",
        "N_tPointXS_00", "N_tPointXS_01",  "N_perfectS_00", "N_tPerfectS_00", "N_tPerfectS_01",
    };
    for (int i = 0; i < 15; i++) {
        mpPanes[i] = mpLayout->findPane(pane[i]);
    }
    const char* text[] = {
        "T_pointL00_00",  "T_pointL00_01",  "T_pointS00_00",  "T_pointS00_01",
        "T_pointXL00_00", "T_pointXL00_01", "T_pointXS00_00", "T_pointXS00_01",
        "N_perfectS00_00", "N_perfectS00_01",
    };
    for (int i = 0; i < 10; i++) {
        mpTextBoxes[i] = mpLayout->findTextBox(text[i]);
    }
    mpCollision = new RPCollisionShape();
}

void RPCowPointPopup::OnStart() {
    mpCollision->Reset();
    mpCollision->mpOwner = static_cast<IRPActorEnable*>(this);
    for (int i = 0; i < 15; i++) {
        mpPanes[i]->SetVisible(false);
    }
    mpPanes[0]->SetVisible(true);
    mpPanes[3]->SetVisible(true);
    mpPanes[6]->SetVisible(true);
    mpPanes[9]->SetVisible(true);
    mpPanes[12]->SetVisible(true);
    mpLayout->unbindAllAnmObj();
    for (int i = 0; i < 5; i++) {
        mpAnmObjs[i]->resetFrame();
    }
}

void RPCowPointPopup::OnStop() {
    for (int i = 0; i < 15; i++) {
        mpPanes[i]->SetVisible(false);
    }
}

void RPCowPointPopup::OnActivate() {
    EGG::Vector3f pos = mpCollision->GetPos();
    mpPanes[0]->SetTranslate(pos);
    mpPanes[3]->SetTranslate(pos);
    mpPanes[6]->SetTranslate(pos);
    mpPanes[9]->SetTranslate(pos);
    mpPanes[12]->SetTranslate(pos);
    mpLayout->calc();
    if (mpAnmObjs[mAnmIdx]->isFinished()) {
        RPActorScene::GetActorList()->StopActor(this);
    }
}

void RPCowPointPopup::Calculate() {
    mpCollision->ApplyOffsets();
}

void RPCowPointPopup::DrawScreen(u8 pass) {
    if ((mScreen == 0 && pass == 2) || (mScreen == 1 && pass == 3)) {
        mpLayout->draw();
    }
}

const f32 RPCowPointPopup::sRadiusNormal = 16.0f;
const f32 RPCowPointPopup::sRadiusPerfect = 24.0f;
const f32 RPCowPointPopup::sScaleNormal = 1.0f;
const f32 RPCowPointPopup::sScalePerfect = 2.0f;
const f32 RPCowPointPopup::sOfsYNormal = 12.0f;
const f32 RPCowPointPopup::sRiseNormal = 4.0f;
const f32 RPCowPointPopup::sUnkCCNormal = 1.0f;
const f32 RPCowPointPopup::sUnkD0Normal = 1.0f;
const f32 RPCowPointPopup::sOfsYPerfect = 14.0f;
const f32 RPCowPointPopup::sRisePerfect = 4.0f;
const f32 RPCowPointPopup::sUnkCCPerfect = 1.0f;
const f32 RPCowPointPopup::sUnkD0Perfect = 1.0f;
const f32 RPCowPointPopup::sFarDist = 800.0f;
const f32 RPCowPointPopup::sNearDist = 0;

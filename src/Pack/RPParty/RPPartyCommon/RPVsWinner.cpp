// Two-player winner screen, 0x801EB2CC-0x801ECA34.
#include <Pack/RPParty/RPPartyCommon/RPVsWinner.h>
#include <Pack/RPParty/RPPartyCommon/RPCommon.h>
#include <Pack/RPParty/RPPartyCommon/RPSelectScene.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPKernel.h>
#include <Pack/RPSystem.h>
#include <Pack/RPParty/RPPartyCommon/RPFaceCtrl.h>
#include <Pack/RPParty/RPPartyCommon/RPLeadCursor.h>
#include <Pack/RPParty/RPPartyCommon/RPPauseText.h>
#include <Pack/RPParty/RPPartyCommon/RPScreenFade.h>

f32 RPVsWinner::sPlayerScale = 18.0f;
f32 RPVsWinner::sXOffsets[2] = {10.0f, -10.0f};
bool RPVsWinner::sFlag = true;
EGG::Vector2f RPVsWinner::sPlayerOffsets[2] = {
    EGG::Vector2f(50.0f, 95.0f), EGG::Vector2f(-50.0f, 95.0f)
};
EGG::Vector3f RPVsWinner::sTrophyVec(7.0f, 8.0f, 8.0f);
EGG::Vector2f RPVsWinner::sTrophyOffset(0.0f, -30.0f);

RPSelectScene* RPSelectScene::sInstance;

extern const s32 Party_vsWinnerScoreTable[64] = {
    -999, 9999, 0, 999, 0, 999, 0, 9999, 0, 999, -99, 99,
    -999, 9999, 0, 999, 0, 999,
    200, 320, 450, 600, 20, 40, 60, 80, 40, 70, 100, 200,
    150, 400, 800, 1200, 3, 7, 10, 14, 15, 27, 51, 60,
    1000, 1600, 2300, 2600, 170, 250, 300, 325, 11, 32, 84, 125,
    0, 1, 2, 3, 4, 5, 7, 8, 9, 0
};

namespace {
struct WinnerStateFuncs {
    RPVsWinnerSeq::StateFunc enter;
    RPVsWinnerSeq::StateFunc calc;
};

inline RPGrpView* WinnerView() {
    RPGrpView* view = RPGrpRenderer::GetCurrent()->GetView(RPCommon::GetLayoutViewNo(1));
    return view;
}

inline f32 WinnerIdentity(f32 value) {
    return value;
}

inline RPFaceCtrl* WinnerFaceCtrl(RPCommon* common, s32 player) {
    return common->mpFaceCtrls[player];
}

inline void SetAxes(EGG::Matrix34f& m, const EGG::Vector3f& x,
                    const EGG::Vector3f& y, const EGG::Vector3f& z) {
    m.setBase(0, x);
    m.setBase(1, y);
    m.setBase(2, z);
}

inline void SetColumns(nw4r::math::MTX34& m, const nw4r::math::VEC3& x,
                       const nw4r::math::VEC3& y, const nw4r::math::VEC3& z,
                       const nw4r::math::VEC3& p) {
    m._00 = x.x; m._10 = x.y; m._20 = x.z;
    m._01 = y.x; m._11 = y.y; m._21 = y.z;
    m._02 = z.x; m._12 = z.y; m._22 = z.z;
    m._03 = p.x; m._13 = p.y; m._23 = p.z;
}
}

RPVsWinner::~RPVsWinner() {}

RPVsWinner::RPVsWinner() {
    mpLayout = new RPVsWinnerLyt;
    mpStateMachine = new RPVsWinnerSeq(this, 6);
    WinnerStateFuncs funcs[6] = {
        {NULL, NULL},
        {&RPVsWinner::EnterIn, &RPVsWinner::CalcIn},
        {&RPVsWinner::EnterShow, &RPVsWinner::CalcShow},
        {&RPVsWinner::EnterClose, &RPVsWinner::CalcClose},
        {&RPVsWinner::EnterSave, &RPVsWinner::CalcSave},
    };
    for (int i = 0; i < 6; i++) {
        mpStateMachine->SetStateFuncs(funcs[i].enter, funcs[i].calc, i);
    }
}

void RPVsWinner::Load() {
    EGG::Archive* const archive = RPSysResourceManager::instance()->GetStaticLocalArchive();
    mpLayout->Load(archive);
    const char* const files[3] = {
        "G3D/win_1P_model.brres", "G3D/win_2P_model.brres", "G3D/tie_model.brres"
    };
    const char* const models[3] = {"win_1P", "win_2P", "tie"};
    const char* const animations[3][2] = {
        {"win_1P_0", "win_1P_1"}, {"win_2P_0", "win_2P_1"}, {"tie_0", "tie_1"}
    };
    RPGrpModelResManager* manager = RPGrpModelResManager::GetCurrent();
    for (int i = 0; i < MODEL_NUM; i++) {
        void* modelData = RPSysResourceManager::GetFileFromArchive(archive, files[i], NULL);
        RPGrpHandle handle = manager->CreateData(RPGrpModelResManager::Type_ResFile, modelData, 0);
        mpModels[i] = RPGrpModel::Construct(handle, models[i], 1, 0, 0);
        mpModels[i]->SetDrawScene(7);
        mpModels[i]->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Chr, 2);
        int j = 0;
        do {
            mpModels[i]->GetModelAnm()->RegistFromResFile(
                RPGrpModelAnm::Anm_Chr, j, handle, animations[i][j], 0);
        } while (++j < 2);
    }
    const char* effects[2] = {"Bcm_win_fubuki", "Bcm_win_spot01"};
    int i = 0;
    do {
        mpEffects[i] = new RPSysEffect(effects[i], RPCommon::GetEffectDrawLayer(), 0);
    } while (++i < EFFECT_NUM);
}

void RPVsWinner::Reset() {
    mpStateMachine->Reset(0);
    for (int i = 0; i < RP_PARTY_MAX_PLAYERS; i++) {
        mPlayerPos[i] = EGG::Vector3f::zero;
        mPlayerAxisX[i] = EGG::Vector3f(1.0f, 0.0f, 0.0f);
        mPlayerAxisY[i] = EGG::Vector3f(0.0f, 1.0f, 0.0f);
        mPlayerAxisZ[i] = EGG::Vector3f(0.0f, 0.0f, 1.0f);
        mPlayerScale[i] = sPlayerScale;
    }
    mTrophyPos = EGG::Vector3f::zero;
    mTrophyAxisX = EGG::Vector3f(1.0f, 0.0f, 0.0f);
    mTrophyAxisY = EGG::Vector3f(0.0f, 1.0f, 0.0f);
    mTrophyAxisZ = EGG::Vector3f(0.0f, 0.0f, 1.0f);
    unkAC = sTrophyVec;
    mpLayout->Reset();
    unk10 = false;
}

void RPVsWinner::Calculate() {
    if (mpStateMachine->GetState() != 0) {
        int i = 0;
        do {
            EGG::Matrix34f m;
            m.setAxisRotation(EGG::Vector3f(0.0f, 1.0f, 0.0f),
                sXOffsets[i] * WinnerIdentity(0.017453292f));
            m.getBase(0, mPlayerAxisX[i]);
            m.getBase(1, mPlayerAxisY[i]);
            m.getBase(2, mPlayerAxisZ[i]);
        } while (++i < RP_PARTY_MAX_PLAYERS);
        mpLayout->Calculate();
    }
    mpStateMachine->Calculate();
    if (mpStateMachine->GetState() != 0) {
        int i = 0;
        do {
            mpEffects[i]->update();
        } while (++i < EFFECT_NUM);
    }
}

void RPVsWinner::EnterIn() {
    RPCommon* common = RPCommon::GetInstance();
    if (mWinner == -1) {
        int i = 0;
        do {
            if (i == 0) common->PlayAvatarAnm(i, 4, 0.0f);
            else common->PlayAvatarAnm(i, 5, 0.0f);
            RPSysKokeshiManager::ChangeExpression(common->GetAvatar(i)->GetNigaoeModel(),
                                                 static_cast<RFLExpression>(0));
        } while (++i < RP_PARTY_MAX_PLAYERS);
    } else {
        int i = 0;
        do {
            if (i == mWinner) {
                common->PlayAvatarAnm(i, 1, 0.0f);
                RPSysKokeshiManager::ChangeExpression(common->GetAvatar(i)->GetNigaoeModel(),
                                                     static_cast<RFLExpression>(1));
            } else {
                common->PlayAvatarAnm(i, 3, 0.0f);
                RPSysKokeshiManager::ChangeExpression(common->GetAvatar(i)->GetNigaoeModel(),
                                                     static_cast<RFLExpression>(3));
            }
        } while (++i < RP_PARTY_MAX_PLAYERS);
    }
    int i = 0;
    do {
        common->mpFaceCtrls[i]->Reset(common->GetAvatar(i));
    } while (++i < RP_PARTY_MAX_PLAYERS);
    mpLayout->Open();
    common->unk30 = false;
    unk10 = false;
    common->GetScreenFade()->SetFadeIn(true);
}

void RPVsWinner::CalcIn() {
    if (mpLayout->mpStateMachine->GetState() == 2) mpStateMachine->ChangeState(2);
    UpdatePositions();
}

void RPVsWinner::EnterShow() {
    mpModels[mModelIdx]->GetModelAnm()->Start(RPGrpModelAnm::Anm_Chr, 0, 0.0f);
    unk10 = true;
    if (mWinner != -1) {
        mpEffects[0]->create();
        mpEffects[1]->create();
    }
    RPPartySystemData data = RPSysSaveDataMgr::instance()->getPartySystemData();
    data.setGameOpen(RPCommon::GetInstance()->GetNextGame(), true);
    RPSysSaveDataMgr::instance()->setPartySystemData(data);
    RPSysSaveDataMgr::instance()->saveAsync();
}

void RPVsWinner::CalcShow() {
    if (sFlag) {
        RPGrpModelAnm* anm = mpModels[mModelIdx]->GetModelAnm();
        f32 lastFrame = anm->GetNumFrame(RPGrpModelAnm::Anm_Chr, 0) - 1;
        if (anm->GetFrame(RPGrpModelAnm::Anm_Chr, 0) >= lastFrame &&
            (RPCommon::IsAnyABPressed(2) || mpStateMachine->GetTimer() >= 300)) {
            mpStateMachine->ChangeState(3);
        }
    }
    UpdatePositions();
    if (mWinner != -1) {
        EGG::Matrix34f m;
        SetAxes(m, mPlayerAxisX[mWinner], mPlayerAxisY[mWinner], mPlayerAxisZ[mWinner]);
        m.setTranslation(mPlayerPos[mWinner]);
        mpEffects[0]->setMtx(m);
        mpEffects[0]->setScale(mPlayerScale[mWinner]);
        EGG::Vector3f scale(0.0f, mPlayerScale[mWinner], mPlayerScale[mWinner]);
        scale.x = mpStateMachine->GetTimer() / 15.0f;
        scale.x = scale.x < 1.0f ? scale.x : 1.0f;
        scale.x = scale.x > 0.01f ? scale.x : 0.01f;
        scale.x *= mPlayerScale[mWinner];
        mpEffects[1]->setMtx(m);
        mpEffects[1]->setScale(scale);
    }
}

void RPVsWinner::EnterClose() {
    mpModels[mModelIdx]->GetModelAnm()->Start(RPGrpModelAnm::Anm_Chr, 1, 0.0f);
    mpLayout->Close();
    if (mWinner != -1) mpEffects[0]->fade();
}

void RPVsWinner::CalcClose() {
    RPGrpModelAnm* anm = mpModels[mModelIdx]->GetModelAnm();
    f32 lastFrame = anm->GetNumFrame(RPGrpModelAnm::Anm_Chr, 1) - 1;
    if (anm->GetFrame(RPGrpModelAnm::Anm_Chr, 1) >= lastFrame &&
        mpLayout->mpStateMachine->GetState() == 4) {
        if (mWinner != -1) mpEffects[1]->fade();
        mpStateMachine->ChangeState(4);
    }
    UpdatePositions();
    if (mWinner != -1) {
        EGG::Vector3f scale(0.0f, mPlayerScale[mWinner], mPlayerScale[mWinner]);
        scale.x = mpStateMachine->GetTimer() / 15.0f;
        scale.x = 1.0f - (scale.x < 1.0f ? scale.x : 1.0f);
        scale.x *= mPlayerScale[mWinner];
        mpEffects[1]->setScale(scale);
    }
}

void RPVsWinner::EnterSave() {}

void RPVsWinner::CalcSave() {
    if (RPSysSaveDataMgr::instance()->isAsyncFinish()) mpStateMachine->ChangeState(5);
}

void RPVsWinner::DrawLayer1A() {
    if (mpStateMachine->GetState() != 0) mpLayout->DrawLayer1A();
}

void RPVsWinner::DrawLayer1C() {
    if (mpStateMachine->GetState() != 0) mpLayout->DrawLayer1C();
}

void RPVsWinner::Start(s32 winner) {
    if (mpStateMachine->GetState() == 0) {
        mWinner = winner;
        if (winner == -1) mModelIdx = 2;
        else mModelIdx = winner;
        RPCommon::GetInstance()->GetLeadCursor()->SetAllEnabled(false);
        RPCommon::GetInstance()->GetPauseText()->SetEnabled(false);
        mpStateMachine->ChangeState(1);
    }
}

#pragma push
#pragma always_inline on
inline void UpdateWinnerPlayerModels(RPVsWinner* winner, EGG::Matrix34f& m) {
        for (s32 j = 0; j < RP_PARTY_MAX_PLAYERS; j++) {
            m.setBase(0, winner->mPlayerAxisX[j]);
            m.setBase(1, winner->mPlayerAxisY[j]);
            m.setBase(2, winner->mPlayerAxisZ[j]);
            m.setTranslation(winner->mPlayerPos[j]);
            winner->mPlayerScale[j] = RPVsWinner::sPlayerScale;
            RPCommon::GetInstance()->SetAvatarScale(j, winner->mPlayerScale[j], m);
        }
}
#pragma pop

void RPVsWinner::UpdatePositions() {
    const RPGrpView* view = WinnerView();
    RPCommon* common = RPCommon::GetInstance();
    if (mpLayout->mpStateMachine->GetState() != 0) {
        int i = 0;
        do {
            EGG::Vector2f pos = mpLayout->GetPlayerPanePos(i);
            f32 widthScale = static_cast<f32>(view->GetScreen()->GetWidth()) /
                EGG::Screen::GetSizeXMax(EGG::Screen::TV_MODE_STD);
            pos.x = widthScale * sPlayerOffsets[i].x + pos.x;
            pos.y += sPlayerOffsets[i].y;
            mPlayerPos[i] = common->ScreenToWorld(pos);
            WinnerFaceCtrl(common, i)->Calculate(common->GetAvatar(i));
        } while (++i < RP_PARTY_MAX_PLAYERS);
        EGG::Matrix34f m;
        UpdateWinnerPlayerModels(this, m);
    }
    if (mpLayout->mpStateMachine->GetState() != 0 && unk10) {
        EGG::Vector2f pos = mpLayout->GetTrophyPanePos();
        pos.x += sTrophyOffset.x *
                 (static_cast<f32>(static_cast<u16>(view->GetScreen()->GetWidth())) /
                  EGG::Screen::GetSizeXMax(EGG::Screen::TV_MODE_STD));
        pos.y += sTrophyOffset.y;
        mTrophyPos = common->ScreenToWorld(pos);
        const nw4r::math::VEC3& cameraPos = view->GetCamera()->GetPosition();
        EGG::Vector3f distance = static_cast<EGG::Vector3f&>(mTrophyPos) -
            EGG::Vector3f(cameraPos.x, cameraPos.y, cameraPos.z);
        static_cast<EGG::Vector3f&>(mTrophyPos) -= distance * 0.5f;
        nw4r::math::MTX34 m;
        SetColumns(m, mTrophyAxisX, mTrophyAxisY, mTrophyAxisZ, mTrophyPos);
        nw4r::g3d::ScnObj* obj = mpModels[mModelIdx]->GetScnObj();
        if (obj != NULL) obj->SetMtx(nw4r::g3d::ScnObj::MTX_LOCAL, &m);
        unkAC = sTrophyVec;
        mpModels[mModelIdx]->SetBaseScale(unkAC);
        mpModels[mModelIdx]->UpdateFrame();
        mpModels[mModelIdx]->Calc();
        if (RPCommon::GetInstance()->mEntryAvatars) mpModels[mModelIdx]->Entry();
    }
}

#pragma defer_codegen off
#pragma auto_inline off
template void RPStateMachine<RPVsWinner>::Calculate();
template void RPStateMachine<RPVsWinner>::ChangeState(s32);
template void RPStateMachine<RPVsWinner>::Reset(s32);
template void RPStateMachine<RPVsWinner>::SetStateFuncs(const StateFunc&, const StateFunc&, s32);
template RPStateMachine<RPVsWinner>::RPStateMachine(RPVsWinner*, s32);
template void RPStateMachine<RPVsWinner>::CallEnter();
template void RPStateMachine<RPVsWinner>::CallCalc();
template void RPStateMachine<RPVsWinner>::CallExit();

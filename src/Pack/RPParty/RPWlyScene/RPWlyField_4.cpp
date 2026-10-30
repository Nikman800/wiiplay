#include <Pack/RPParty/RPWlyScene/RPWlyField_4.h>
#include <Pack/RPParty/RPWlyScene/RPWlyCursor.h>

#include <Pack/RPGraphics.h>
#include <egg/gfxe/eggScreen.h>
#include <egg/math.h>

#include <Pack/RPParty/RPWlyScene/RPWlyField_5.h>
#include <Pack/RPParty/RPWlyScene/RPWlyGameMgr.h>
#include <Pack/RPParty/RPWlyScene/RPWlyLevelTable.h>

class RPWlyCursor;

// RPWlyCursor::GetPos returns the position by value (hidden result pointer in r3, cursor in r4);
// RPWlyCursor.h declares it with an out parameter, so it is called under its ledger name here.

// The game state is read as an enum (an int compare would become a range check).
enum WlyGameState {
    STATE_CLEAR = 3,
    STATE_RESULT = 4,
};

static inline WlyGameState GetState() {
    return static_cast<WlyGameState>(RPWlyGameMgr::sInstance->mState);
}

enum WlyGameMode {
    MODE_SINGLE,
};

static inline WlyGameMode GetMode() {
    return static_cast<WlyGameMode>(RPWlyGameMgr::sInstance->mMode);
}

RPWlyField06::~RPWlyField06() {}

RPWlyField06::RPWlyField06() {
    mTexScale = 0.03f;
    for (int i = 0; i < 2; i++) {
        mTexMtx[i]._00 = 1.0f;
        mTexMtx[i]._01 = 0.0f;
        mTexMtx[i]._02 = -(0.5f - 0.5f * mTexScale);
        mTexMtx[i]._03 = 0.0f;
        mTexMtx[i]._10 = 0.0f;
        mTexMtx[i]._11 = 1.0f;
        mTexMtx[i]._12 = -(0.5f - 0.5f * mTexScale);
        mTexMtx[i]._13 = 0.0f;
        mTexMtx[i]._20 = 0.0f;
        mTexMtx[i]._21 = 0.0f;
        mTexMtx[i]._22 = mTexScale;
        mTexMtx[i]._23 = 0.0f;
    }
    mPos.x = 0.0f;
    mPos.y = 0.0f;
    mPos.z = RPWlyLevelTable::sInstance->GetCurrentLevel()->mSpotlight;
    mScale.x = 0.05f;
    mScale.y = 0.05f;
    mScale.z = 0.05f;
    mAnmStarted = false;
}

void RPWlyField06::Load() {
    RPGrpHandle file = RPSimpleModel::LoadResource("/G3D/wtd_field06.brres");
    RPSimpleModel::LoadModel("wtd_field06", file, 1, 0, 0x108);
    mpModel->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Clr, 1);
    RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();
    const char* name = "wtd_field06";
    void* pData = pResMgr->GetPtr(RPGrpModelResManager::Type_ResAnmClr, file, name);
    RPGrpHandle anm = pResMgr->CreateData(RPGrpModelResManager::Type_ResAnmClr, pData);
    mpModel->GetModelAnm()->Regist(RPGrpModelAnm::Anm_Clr, 0, anm);
    mpModel->GetModelAnm()->SetFrame(RPGrpModelAnm::Anm_Clr, 0, 0.0f);
    mpModel->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_Clr, 0, 0.0f);
    mpModel->SetDrawScene(1);
}

void RPWlyField06::Reset() {}

void RPWlyField06::Calculate() {
    if ((GetState() == STATE_CLEAR || GetState() == STATE_RESULT) && !mAnmStarted) {
        mpModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_Clr, 0, 0.0f);
        mpModel->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_Clr, 0, 1.0f);
        mAnmStarted = true;
    }

    if (RPWlyLevelTable::sInstance->GetCurrentLevel()->mAvatarNum < 50 ||
        RPWlyGameMgr::sInstance->mpWipe->mDone) {
        for (int i = 0; i < RPWlyGameMgr::sInstance->mPlayerNum; i++) {
            mTexMtx[i]._02 = -(0.5f - 0.5f * mTexScale);
            mTexMtx[i]._12 = -(0.5f - 0.5f * mTexScale);
            mTexMtx[i]._22 = mTexScale;
            nw4r::math::VEC3 hit;
            EGG::Vector2f pos = RPWlyGameMgr::sInstance->mpCursors[i]->GetPos();
            RPGrpScreen* pScreen = RPGrpRenderer::GetCurrent()->GetView(0)->GetScreen();
            nw4r::math::VEC3 normal(0.0f, 0.0f, 1.0f);
            RPWlyGameMgr::sInstance->mpCamera->GetScreenToPlane(
                &hit, mPos, normal, pScreen,
                pos.x * (f32)(u16)(EGG::Screen::GetSizeXMax() / 2),
                pos.y * (f32)(u16)(EGG::Screen::GetSizeYMax() / 2));
            mTexMtx[i]._03 = -(hit.x / (5000.0f * mScale.x));
            mTexMtx[i]._13 = hit.y / (5000.0f * mScale.y);
        }
    }

    RPGrpModel* pModel = mpModel;
    RPGrpModelMaterial* pMat = pModel->GetMaterial("SPOT");
    nw4r::math::MTX34* pTexMtx0;
    nw4r::math::MTX34* pTexMtx1;
    WlyGameMode mode = GetMode();
    if (mode == MODE_SINGLE) {
        pTexMtx0 = pMat->GetEffectMtx(0);
        pTexMtx1 = pMat->GetEffectMtx(1);
        *pTexMtx0 = mTexMtx[0];
        *pTexMtx1 = *pTexMtx0;
    } else {
        pTexMtx0 = pMat->GetEffectMtx(0);
        pTexMtx1 = pMat->GetEffectMtx(1);
        *pTexMtx0 = mTexMtx[0];
        *pTexMtx1 = mTexMtx[1];
    }
    pMat->SetSyncFlags(RPGrpModelMaterial::ESyncFlag_EffectMtx);

    nw4r::math::VEC3 pos = mPos;
    nw4r::math::VEC3 scale = mScale;
    // NONMATCHING-DEBT: RPSimpleModel::sMtxIdentity is declared VEC3[3] (its static initializer
    // copy-constructs the rows, RPKcl_2.cpp); this copy is one 9-word MTX33 block copy, which only
    // an MTX33 source gives. Fix with the RPSimpleModel declaration (PartyCommon), not here.
    nw4r::math::MTX33 rotation = *reinterpret_cast<const nw4r::math::MTX33*>(sMtxIdentity);
    mRot = rotation;
    mPos.x = pos.x;
    mPos.y = pos.y;
    mPos.z = pos.z;
    mScale.x = scale.x;
    mScale.y = scale.y;
    mScale.z = scale.z;
    CalcModelMtx();
}

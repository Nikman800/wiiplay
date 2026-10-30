// Laser Hockey round paddle: the round paddle model and its glow effect, following its owner.
// Unit 0x80222118-0x802225F0 (dtk/game_units.tsv). With -inline deferred CodeWarrior emits a
// file's functions in reverse order, so the source runs from the last address to the first.
#include <Pack/RPParty/RPHkyScene/RPHkyPaddle.h>

#include <Pack/RPGraphics.h>

RPHkyPaddleRound::~RPHkyPaddleRound() {}

RPHkyPaddleRound::RPHkyPaddleRound(s32 playerIndex, RPHkyPaddle* pOwner)
    : mPlayerIndex(playerIndex), mpOwner(pOwner) {
    mpEffect = new RPSysEffect(mPlayerIndex == 0 ? "Hky_paddletail_bc_glow"
                                                 : "Hky_paddletail_rc_glow",
                               0, 0);
}

void RPHkyPaddleRound::Create() {}

void RPHkyPaddleRound::Load() {
    RPGrpHandle file = RPSimpleModel::LoadResource(
        mPlayerIndex == 0 ? "/G3D/Hck_paddle_C_B.brres" : "/G3D/Hck_paddle_C_R.brres");
    RPSimpleModel::LoadModel(mPlayerIndex == 0 ? "Hck_paddle_C_B" : "Hck_paddle_C_R", file, 1,
                             0, 0x108);

    RPGrpModel* pModel = mpModel;
    pModel->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Clr, 1);

    RPGrpHandle anmFile = RPSimpleModel::LoadResource("/G3D/Hck_anim.brres");
    const char* pAnmName = mPlayerIndex == 0 ? "Hck_paddle_B_hit" : "Hck_paddle_R_hit";
    RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();
    RPGrpHandle anm = pResMgr->CreateData(
        RPGrpModelResManager::Type_ResAnmClr,
        pResMgr->GetPtr(RPGrpModelResManager::Type_ResAnmClr, anmFile, pAnmName));
    pModel->GetModelAnm()->Regist(RPGrpModelAnm::Anm_Clr, 0, anm);
}

void RPHkyPaddleRound::Reset() {
    mPos = sVecZero;
    mScale = sVecOne;
    mpEffect->create();
    mpEffect->setDisableCalcDraw(true);
}

void RPHkyPaddleRound::Calculate() {
    if (mpOwner->GetShapeType() != 0) {
        return;
    }

    nw4r::math::VEC3 effectScale(1.15f, 1.15f, 1.15f);
    mpEffect->setScale(effectScale);

    SetRot(mpOwner->GetRot());
    SetPos(mpOwner->GetPos());
    SetScale(mScale);
    CalcModelMtx();

    nw4r::math::MTX34 mtx;
    mtx._00 = mRot._00;
    mtx._10 = mRot._01;
    mtx._20 = mRot._02;
    mtx._01 = mRot._10;
    mtx._11 = mRot._11;
    mtx._21 = mRot._12;
    mtx._02 = mRot._20;
    mtx._12 = mRot._21;
    mtx._22 = mRot._22;
    mtx._03 = mPos.x;
    mtx._13 = mPos.y;
    mtx._23 = mPos.z;
    mpEffect->setMtx(mtx);
    mpEffect->update();
}

void RPHkyPaddleRound::Exit() {}

void RPHkyPaddleRound::RestartHitAnim() {
    mpModel->GetModelAnm()->Start(RPGrpModelAnm::Anm_Clr, 0, 0.0f);
}

#include <Pack/RPParty/RPPartyCommon/RPModelObj.h>

#include <Pack/RPParty/RPPartyCommon/RPResSlots.h>

extern nw4r::math::VEC3 RP_vecZero;
extern nw4r::math::VEC3 RP_vecOne;
extern RPAxes RP_mtx33Identity;

extern RPResSlots* RP_resSlotsInstance;
u32 RP_ResSlots_findOrLoad(RPResSlots* pSlots, const char* pResName, u16 start, u32 kind,
                           u32 arg);

// The draw group given to every model created afterwards. A plain global in the binary
// (.sbss 8043C610), not RPModelObj::sDrawGroup.
u32 RP_modelObjDrawGroup;

void RPModelObj::SetUnk40(u8 value) {
    unk40 = value;
    if (mpModel != NULL) {
        mpModel->SetVisible(true);
    }
}

extern "C" void RP_ModelObj_setDrawGroup(u32 drawGroup) {
    RP_modelObjDrawGroup = drawGroup;
}

RPModelObj::RPModelObj() {
    mPos = RP_vecZero;
    mScale = RP_vecOne;
    mRot = RP_mtx33Identity;
    mpModel = NULL;
}

RPModelObj::~RPModelObj() {}

void RPModelObj::CalcModelMtx() {
    nw4r::math::MTX34 mtx;
    mtx._00 = RP_vecOne.x * mRot.x.x;
    mtx._01 = RP_vecOne.y * mRot.y.x;
    mtx._02 = RP_vecOne.z * mRot.z.x;
    mtx._03 = mPos.x;
    mtx._10 = RP_vecOne.x * mRot.x.y;
    mtx._11 = RP_vecOne.y * mRot.y.y;
    mtx._12 = RP_vecOne.z * mRot.z.y;
    mtx._13 = mPos.y;
    mtx._20 = RP_vecOne.x * mRot.x.z;
    mtx._21 = RP_vecOne.y * mRot.y.z;
    mtx._22 = RP_vecOne.z * mRot.z.z;
    mtx._23 = mPos.z;

    nw4r::g3d::ScnObj* pScnObj = mpModel->GetScnObj();
    if (pScnObj != NULL) {
        pScnObj->SetMtx(nw4r::g3d::ScnObj::MTX_LOCAL, &mtx);
    }

    mpModel->SetBaseScale(mScale);
    mpModel->UpdateFrame();
    mpModel->Calc();
    mpModel->Entry();
}

void RPModelObj::LoadModel(const char* pResName, u16 slot, u32 option) {
    mpModel = RPGrpModel::Construct(RP_ResSlots_findOrLoad(RP_resSlotsInstance, pResName, slot, 1,
                                                           option),
                                    1, 0, 0);
    mpModel->SetDrawGroup(RP_modelObjDrawGroup);
    mpModel->SetDrawScene(0);
}

void RPModelObj::CreateModel(RPGrpHandle handle) {
    mpModel = RPGrpModel::Construct(handle, 1, 0, 0);
    mpModel->SetDrawGroup(RP_modelObjDrawGroup);
    mpModel->SetDrawScene(0);
}

void RPModelObj::CreateAnmBuffer(RPGrpModelAnm::Anm anm, u16 num) {
    mpModel->GetModelAnm()->CreateBuffer(anm, num);
}

void RPModelObj::LoadAnm(const char* pResName, u16 slot, RPGrpModelAnm::Anm anm, u16 idx) {
    RPResSlots* pSlots = RP_resSlotsInstance;
    u32 kind;
    switch (anm) {
    case RPGrpModelAnm::Anm_Chr:
        kind = 3;
        break;
    case RPGrpModelAnm::Anm_Clr:
        kind = 5;
        break;
    case RPGrpModelAnm::Anm_TexPat:
        kind = 7;
        break;
    case RPGrpModelAnm::Anm_TexSrt:
        kind = 6;
        break;
    }
    u32 handle = RP_ResSlots_findOrLoad(pSlots, pResName, slot, kind, 1);
    RegistAnm(anm, idx, handle);
}

void RPModelObj::RegistAnm(RPGrpModelAnm::Anm anm, u16 idx, RPGrpHandle handle) {
    mpModel->GetModelAnm()->Regist(anm, idx, handle, 0);
}

bool RPModelObj::StartAnm(RPGrpModelAnm::Anm anm, u16 idx, f32 frame) {
    return mpModel->GetModelAnm()->Start(anm, idx, frame);
}

bool RPModelObj::StartAnmBlend(RPGrpModelAnm::Anm anm, u16 idx, f32 weight, f32 frame) {
    return mpModel->GetModelAnm()->StartBlend(anm, idx, weight, frame);
}

bool RPModelObj::CancelAnm(RPGrpModelAnm::Anm anm, u16 idx) {
    return mpModel->GetModelAnm()->Cancel(anm, idx);
}

void RPModelObj::SetAnmUpdateRate(RPGrpModelAnm::Anm anm, u16 idx, f32 rate) {
    mpModel->GetModelAnm()->SetUpdateRate(anm, idx, rate);
}

f32 RPModelObj::GetAnmFrame(RPGrpModelAnm::Anm anm, u16 idx) {
    return mpModel->GetModelAnm()->GetFrame(anm, idx);
}

f32 RPModelObj::GetAnmNumFrame(RPGrpModelAnm::Anm anm, u16 idx) {
    return mpModel->GetModelAnm()->GetNumFrame(anm, idx);
}

void RPModelObj::Calculate() {}

void RPModelObj::Draw() {}

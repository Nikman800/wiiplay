// Wii Play: RPUtlRandom has no seed setter; SetRandomSeed and RP_seedRandomFromTime store the
// seed directly
#include <Pack/RPUtility/RPUtlRandom.h>

#include <Pack/RPParty/RPPartyCommon/RPSimpleModel.h>

#include <Pack/RPGraphics/RPGrpModelResManager.h>
#include <Pack/RPSystem/RPSysResourceManager.h>
#include <revolution/OS.h>

extern "C" void RP_makeMtxFromRotPos(nw4r::math::MTX34* pMtx, const nw4r::math::VEC3* pPos,
                                     const nw4r::math::MTX33* pRot);

void RPSimpleModel::SetRandomSeed(u32 seed) {
    RPUtlRandom::sRandom.mSeed = seed;
}

// Called by RPSysSystem (ledger name)
extern "C" void RP_seedRandomFromTime() {
    OSCalendarTime calendarTime;
    OSTicksToCalendarTime(OSGetTime(), &calendarTime);

    RPUtlRandom::sRandom.mSeed = (calendarTime.usec | (calendarTime.msec << 10)) |
                                 ((calendarTime.min << 26) | (calendarTime.sec << 20));
}

// Wii Play: the original copies a 3x3 matrix as three 12-byte rows, not as one 36-byte struct
// (same as RPHkyPuck)
static inline void CopyRows(nw4r::math::MTX33& rDst, const nw4r::math::VEC3* pSrc) {
    struct Row {
        f32 v[3];
    };

    for (int i = 0; i < 3; i++) {
        *reinterpret_cast<Row*>(rDst.m[i]) = *reinterpret_cast<const Row*>(&pSrc[i]);
    }
}

RPSimpleModel::RPSimpleModel() {
    mPos = sVecZero;
    mScale = sVecOne;
    CopyRows(mRot, sMtxIdentity);
}

RPSimpleModel::~RPSimpleModel() {}

void RPSimpleModel::CalcModelMtx() {
    nw4r::math::MTX34 mtx;
    RP_makeMtxFromRotPos(&mtx, &mPos, &mRot);

    mpModel->GetModelEx()->setLocalMtx(&mtx);

    mpModel->SetBaseScale(mScale);
    mpModel->UpdateFrame();
    mpModel->Calc();
    mpModel->Entry();
}

RPGrpHandle RPSimpleModel::LoadResource(const char* pName) {
    EGG::Archive* pArchive = RPSysResourceManager::LoadGameCommonArchive(-1, NULL);
    void* pResFile = RPSysResourceManager::GetFileFromArchive(pArchive, pName, NULL);
    return RPGrpModelResManager::GetCurrent()->CreateData(RPGrpModelResManager::Type_ResFile,
                                                          pResFile, 0);
}

void RPSimpleModel::LoadModel(const char* pName, RPGrpHandle resFile, u8 viewNo,
                              u32 typeOption, u32 bufferOption) {
    RPGrpModelResManager* pResManager = RPGrpModelResManager::GetCurrent();
    void* pResMdl = pResManager->GetPtr(RPGrpModelResManager::Type_ResMdl, resFile, pName);
    RPGrpHandle mdlResFile =
        pResManager->CreateData(RPGrpModelResManager::Type_ResMdl, pResMdl, 0);
    mpModel = RPGrpModel::Construct(mdlResFile, viewNo, typeOption, bufferOption);
}

void RPSimpleModel::Calculate() {}

void RPSimpleModel::VF_0x18() {}

RPUtlRandom RPUtlRandom::sRandom;

const u32 RPUtlRandom::RANDF_MAX = 0xffff;
const u32 RPUtlRandom::RANDF_SHIFT = 16;
const u32 RPUtlRandom::MULT = 69069;

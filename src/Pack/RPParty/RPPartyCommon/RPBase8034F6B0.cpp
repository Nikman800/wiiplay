// nw4r::g3d::ScnRfl::TYPE_NAME (0x803181A8) is g3d_scnrfl.cpp's .rodata, not this file's: the
// ledger range for this unit's .rodata ends at 0x803181A8 (the "item" literal only).
#include <Pack/RPParty/RPPartyCommon/RPBase8034F6B0.h>

#include <Pack/RPGraphics/RPGrpDrawPathLightMap.h>
#include <Pack/RPGraphics/RPGrpDrawPathManager.h>
#include <Pack/RPGraphics/RPGrpModel.h>
#include <Pack/RPGraphics/RPGrpModelManager.h>
#include <Pack/RPKernel/RPSysKokeshiManager.h>
#include <Pack/RPUtility/RPUtlRandom.h>
#include <Pack/RPSystem/RPSysResourceManager.h>

RPBase8034F6B0::RPBase8034F6B0(const RPSysKokeshiGenInfo* pGenInfo)
    : RPSysAvatar(pGenInfo) {
    Construct();
}

RPBase8034F6B0::~RPBase8034F6B0() {}

static inline u8 GetRandomClothesType(IRPSysKokeshiBodyManager* pBodyManager) {
    u32 index = RPUtlRandom::getU32(pBodyManager->GetFriendClothesIndexRange());
    u32 start = pBodyManager->GetFriendClothesStartIndex();
    return start + index;
}

void RPBase8034F6B0::LoadResource(
    const RPSysKokeshiOverloadInfo* pOverloadInfo) {
    RPSysAvatar::LoadResource(pOverloadInfo);
    const RPSysKokeshiGenInfo& rGenInfo = GetGenInfo();
    RPSysKokeshiManager* pKokeshiManager =
        RPSysKokeshiManager::instance();
    IRPSysKokeshiBodyManager* pBodyManager = pKokeshiManager->GetBodyManager();
    EGG::Archive* pArchive =
        RPSysResourceManager::instance()->GetKokeshiArchive();

    void* pFile = RPSysResourceManager::GetFileFromArchive(
        pArchive, pKokeshiManager->GetHandFileName(rGenInfo.GetHandType()));

    mpHandModel = pKokeshiManager->CreateModel(
        pFile, 0, rGenInfo.GetViewNo(), rGenInfo.GetTypeOption(),
        rGenInfo.GetBufferOption());

    RPSysKokeshiOverloadInfo& rOverloadInfo = mOverloadInfo;
    u16 clothesType = rOverloadInfo.GetClothesType();
    if (clothesType == RPSysKokeshiManager::ClothesType_Random) {
        switch (rGenInfo.GetGenType()) {
        case RPSysKokeshiManager::GenType_Player:
            clothesType = rGenInfo.GetPlayerNo();
            break;
        case RPSysKokeshiManager::GenType_Friend:
        case RPSysKokeshiManager::GenType_Direct: {
            clothesType = GetRandomClothesType(pBodyManager);
            break;
        }
        default:
            break;
        }
        rOverloadInfo.SetClothesType(clothesType);
    }

    pFile = RPSysResourceManager::GetFileFromArchive(
        pArchive, pBodyManager->GetClothesFileName());

    const char* pTextureNameA = NULL;
    const char* pTextureNameB = NULL;
    pBodyManager->GetClothesName(clothesType,
                                 static_cast<RFLSex>(mAdditionalInfo.sex),
                                 &pTextureNameA, &pTextureNameB);

    RPGrpTexture* pTextureA =
        pKokeshiManager->CreateTexture(pFile, pTextureNameA);
    RPGrpTexture* pTextureB =
        pKokeshiManager->CreateTexture(pFile, pTextureNameB);
    pKokeshiManager->ChangeTexture(mpHandModel, pTextureA, pTextureB);

    GXColor body = GetFavoriteColor();
    pKokeshiManager->SetMatColor(mpHandModel, body,
                                mAdditionalInfo.skinColor);
}

static inline void ReplaceLightTexture(
    RPGrpDrawPathLightMap* pLightMap, RPGrpModel* pModel, u8 i) {
    int index = pLightMap->GetLightTextureManager()->getTextureIndex(
        RPSysKokeshiManager::GetLightTextureName(i));

    pLightMap->ReplaceModelTexture(index, pModel);
}

void RPBase8034F6B0::ApplyLightTexture(u8 drawScene) {
#pragma scheduling off
    RPGrpDrawPathLightMap* pDrawPath =
        RPGrpModelManager::GetCurrent()
            ->GetDrawPathManager(drawScene)
            ->GetDrawPathLightMap();

    for (u8 i = 0; i < RPSysKokeshiManager::LightMap_Max; i++) {
        ReplaceLightTexture(pDrawPath, mpHandModel, i);
    }
#pragma scheduling reset
}

void RPBase8034F6B0::GetItemJointMtx(nw4r::math::MTX34* pMtx) {
    RPSysKokeshiManager* pKokeshiManager = RPSysKokeshiManager::instance();
    u16 jointIndex = mpHandModel->GetJointIndex("item");
    mpHandModel->GetWorldMtx(jointIndex, pMtx);
    pKokeshiManager->GetModelBaseScaleInvalidatedMtx(
        pMtx, mpHandModel, pMtx);
}

void RPBase8034F6B0::Construct() {
    RPSysKokeshiManager* pKokeshiManager = RPSysKokeshiManager::instance();
    RPSysKokeshiLocation location;

    const RPSysKokeshiGenInfo& rGenInfo = GetGenInfo();
    switch (rGenInfo.GetGenType()) {
    case RPSysKokeshiManager::GenType_Player:
        pKokeshiManager->GetLocationPlayer(rGenInfo.GetPlayerNo(), &location);
        mGenInfo.GetLocation().SetDataSource(location.GetDataSource());
        mGenInfo.GetLocation().SetIndex(location.GetIndex());
        mGenInfo.GetLocation().SetMiddleDB(location.GetMiddleDB());
        break;
    case RPSysKokeshiManager::GenType_Friend:
        pKokeshiManager->GetLocationFriend(
            GetGenInfo().GetLocation().GetDataSource(),
            GetGenInfo().GetLocation().GetMiddleDB(), &location);
        mGenInfo.GetLocation().SetDataSource(location.GetDataSource());
        mGenInfo.GetLocation().SetIndex(location.GetIndex());
        mGenInfo.GetLocation().SetMiddleDB(location.GetMiddleDB());
        break;
    case RPSysKokeshiManager::GenType_Direct:
        location.SetDataSource(GetGenInfo().GetLocation().GetDataSource());
        location.SetIndex(GetGenInfo().GetLocation().GetIndex());
        location.SetMiddleDB(GetGenInfo().GetLocation().GetMiddleDB());
        break;
    default:
        break;
    }

    Setup();
}

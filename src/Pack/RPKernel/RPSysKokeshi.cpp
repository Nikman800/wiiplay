#include <Pack/RPGraphics.h>
// Ported from doldecomp/ogws (CC0) src/Pack/RPKernel/RPSysKokeshi.cpp @ f5a1bb5e; Wii Play changes marked "Wii Play:".
#include <Pack/RPKernel.h>
#include <Pack/RPSystem.h>
#include <Pack/RPUtility.h>

#include <RVLFaceLib.h>

#include <egg/core.h>

// Wii Play: not in Wii Sports; stubbed for release (always false). Called by the Pnp and Wly
// avatar code with two Mii locations.
bool Mii_isSameKokeshiLocation(const RPSysKokeshiLocation* /* pLhs */,
                               const RPSysKokeshiLocation* /* pRhs */) {
    return false;
}

/**
 * @brief Constructor
 *
 * @param pGenInfo Mii generation parameters
 */
RPSysKokeshi::RPSysKokeshi(const RPSysKokeshiGenInfo* pGenInfo)
    : RPSysAvatar(pGenInfo) {
    Construct();
}

/**
 * @brief Destructor
 */
RPSysKokeshi::~RPSysKokeshi() {}

// Wii Play: the skin color copy goes through an inline helper (it sits at the bottom of the frame)
static inline GXColor GetSkinColor(const RFLAdditionalInfo& rInfo) {
    GXColor color = rInfo.skinColor;
    return color;
}

/**
 * @brief Loads the assets necessary for this avatar
 *
 * @param pOverloadInfo Overload parameters
 */
void RPSysKokeshi::LoadResource(const RPSysKokeshiOverloadInfo* pOverloadInfo) {
    RPSysAvatar::LoadResource(pOverloadInfo);

    RPSysResourceManager* pResourceManager =
        RP_GET_INSTANCE(RPSysResourceManager);

    RPSysKokeshiManager* pKokeshiManager = RP_GET_INSTANCE(RPSysKokeshiManager);
    IRPSysKokeshiBodyManager* pBodyManager = pKokeshiManager->GetBodyManager();

    // Kokeshi common archive contains all of the assets
    EGG::Archive* pKokeshiArchive = pResourceManager->GetKokeshiArchive();

    // Open the body archive
    const char* pBodyFileName = pBodyManager->GetBodyFileName(
        mGenInfo.GetBodyType(), static_cast<RFLSex>(mAdditionalInfo.sex));

    void* pFile = RPSysResourceManager::GetFileFromArchive(pKokeshiArchive,
                                                           pBodyFileName);

    // Create the body model
    u16 bodyIdx = pBodyManager->GetBodyModelIndex(
        mGenInfo.GetBodyType(), static_cast<RFLSex>(mAdditionalInfo.sex),
        mAdditionalInfo.height, mAdditionalInfo.build);

    mpBodyModel = pKokeshiManager->CreateModel(
        pFile, bodyIdx, mGenInfo.GetViewNo(), mGenInfo.GetTypeOption(),
        mGenInfo.GetBufferOption());

    // Open the hand archive
    const char* pHandFileName =
        pKokeshiManager->GetHandFileName(mGenInfo.GetHandType());

    pFile = RPSysResourceManager::GetFileFromArchive(pKokeshiArchive,
                                                     pHandFileName);

    // Create the hand models
    mpLeftHandModel = pKokeshiManager->CreateModel(
        pFile, 0, mGenInfo.GetViewNo(), mGenInfo.GetTypeOption(),
        mGenInfo.GetBufferOption());

    mpRightHandModel = pKokeshiManager->CreateModel(
        pFile, 0, mGenInfo.GetViewNo(), mGenInfo.GetTypeOption(),
        mGenInfo.GetBufferOption());

    // Assign random clothes if requested via the overload info
    u16 clothesType = mOverloadInfo.GetClothesType();

    if (clothesType == RPSysKokeshiManager::ClothesType_Random) {
        switch (mGenInfo.GetGenType()) {
        case RPSysKokeshiManager::GenType_Player: {
            clothesType = mGenInfo.GetPlayerNo();
            break;
        }

        case RPSysKokeshiManager::GenType_Friend:
        case RPSysKokeshiManager::GenType_Direct: {
            clothesType = static_cast<u8>(
                pBodyManager->GetFriendClothesStartIndex() +
                RPUtlRandom::getU32(
                    pBodyManager->GetFriendClothesIndexRange()));
            break;
        }

        default: {
            break;
        }
        }

        mOverloadInfo.SetClothesType(clothesType);
    }

    // Open the clothes archive
    const char* pClothFileName = pBodyManager->GetClothesFileName();

    pFile = RPSysResourceManager::GetFileFromArchive(pKokeshiArchive,
                                                     pClothFileName);

    // Determine the correct type of clothes
    const char* pClothNameA = NULL;
    const char* pClothNameB = NULL;
    pBodyManager->GetClothesName(clothesType,
                                 static_cast<RFLSex>(mAdditionalInfo.sex),
                                 &pClothNameA, &pClothNameB);

    // Change the clothes textures
    RPGrpTexture* pClothTexA =
        pKokeshiManager->CreateTexture(pFile, pClothNameA);
    RPGrpTexture* pClothTexB =
        pKokeshiManager->CreateTexture(pFile, pClothNameB);

    pKokeshiManager->ChangeTexture(mpBodyModel, pClothTexA, pClothTexB);
    pKokeshiManager->ChangeTexture(mpLeftHandModel, pClothTexA, pClothTexB);
    pKokeshiManager->ChangeTexture(mpRightHandModel, pClothTexA, pClothTexB);

    // Apply favorite/skin color to the models
    GXColor favColor = GetFavoriteColor();
    GXColor skinColor = GetSkinColor(mAdditionalInfo);

    pKokeshiManager->SetMatColor(mpBodyModel, favColor, skinColor);
    pKokeshiManager->SetMatColor(mpLeftHandModel, favColor, skinColor);
    pKokeshiManager->SetMatColor(mpRightHandModel, favColor, skinColor);

    if (mpNigaoeModel->GetKind() == RPGrpModel::Kind_RFL) {
        static_cast<RPGrpModelRfl*>(mpNigaoeModel)
            ->SetOutputAlpha(pKokeshiManager->GetOutputAlpha());
    }
}

// Wii Play: applies one lightmap texture to one model
static inline void ReplaceLightTexture(RPGrpDrawPathLightMap* pLightMap,
                                       RPGrpModel* pModel, u8 i) {
    int index = pLightMap->GetLightTextureManager()->getTextureIndex(
        RPSysKokeshiManager::GetLightTextureName(i));

    pLightMap->ReplaceModelTexture(index, pModel);
}

/**
 * @brief Applies all lightmap textures to this avatar
 *
 * @param drawScene Draw scene index
 */
void RPSysKokeshi::ApplyLightTexture(u8 drawScene) {
    RPGrpDrawPathLightMap* pLightMap = RPGrpModelManager::GetCurrent()
                                           ->GetDrawPathManager(drawScene)
                                           ->GetDrawPathLightMap();

    // Wii Play: through a helper taking the model first and a u8 light index
    for (u32 i = 0; i < RPSysKokeshiManager::LightMap_Max; i++) {
        ReplaceLightTexture(pLightMap, mpNigaoeModel, i);
        ReplaceLightTexture(pLightMap, mpBodyModel, i);
        ReplaceLightTexture(pLightMap, mpLeftHandModel, i);
        ReplaceLightTexture(pLightMap, mpRightHandModel, i);
    }
}

/**
 * @brief Gets the transformation matrix of the head joint/node
 *
 * @param[out] pMtx Transformation matrix
 */
void RPSysKokeshi::GetHeadJointMtx(EGG::Matrix34f* pMtx) {
    RP_GET_INSTANCE(RPSysKokeshiManager)
        ->GetBodyManager()
        ->GetHeadJointMtx(mGenInfo.GetBodyType(), mpBodyModel, pMtx);
}

// Wii Play: GetLeftHandJointMtx and GetRightHandJointMtx are not in the binary

/**
 * @brief Handles post-constructor initialization
 */
void RPSysKokeshi::Construct() {
    RPSysAvatar::Construct();

    RPSysKokeshiManager* pKokeshiManager = RP_GET_INSTANCE(RPSysKokeshiManager);
    RPSysKokeshiLocation location;

    switch (mGenInfo.GetGenType()) {
    case RPSysKokeshiManager::GenType_Player: {
        pKokeshiManager->GetLocationPlayer(mGenInfo.GetPlayerNo(), &location);

        mGenInfo.SetLocation(location);
        break;
    }

    case RPSysKokeshiManager::GenType_Friend: {
        pKokeshiManager->GetLocationFriend(
            mGenInfo.GetLocation().GetDataSource(),
            mGenInfo.GetLocation().GetMiddleDB(), &location);

        mGenInfo.SetLocation(location);
        break;
    }

    case RPSysKokeshiManager::GenType_Direct: {
        location = mGenInfo.GetLocation();
        break;
    }

    default: {
        break;
    }
    }

    mpNigaoeModel = pKokeshiManager->CreateNigaoeModel(
        location, mGenInfo.GetResolution(), mGenInfo.GetExpressionFlag(),
        mGenInfo.GetViewNo());

    RPSysAvatar::Setup();
}

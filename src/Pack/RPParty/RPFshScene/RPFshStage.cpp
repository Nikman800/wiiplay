// Fishing stage (RPFshStage): lake, tree, cloud and water models, the lake shadow texture.
// Unit 0x8020BDFC-0x8020C470 (dtk/game_units.tsv). The file starts with RPFshStage::IsInPond
// (0x8020BD7C, at the end of RPFshCamera_2.cpp's range) and ends with the destructor thunk
// (RPFshStage_2.cpp, 0x8020C470-0x8020C478). Functions in reverse address order.
#include <Pack/RPGraphics.h>
#include <Pack/RPGraphics/RPGrpDrawPathShadowTexture.h>
#include <Pack/RPGraphics/RPGrpShadowTexture.h>

#include <Pack/RPParty/RPFshScene/RPFshStage.h>

#include <Pack/RPParty/RPPartyCommon/RPSceneInfo.h>
#include <Pack/RPSystem.h>

#include <egg/math.h>

// Wii Play: retain the existing linked names for these external entry points.

static EGG::Vector3f sPondCenter(0.0f, 0.0f, 0.0f);

RPFshStage* RPFshStage::sInstance;

f32 RPFshStage::GetWaterSurfaceY() {
    return -15.0f;
}

f32 RPFshStage::GetPondBottomY() {
    return -104.0f;
}

f32 RPFshStage::GetPondRadius() {
    return 210.0f;
}

inline RPFshStage::RPFshStage() {
    mpKcl = new RPKclHolder();
    mpProjection = new RPGrpProjectionObject(1);
}

void RPFshStage::CreateInstance() {
    sInstance = new RPFshStage();
}

void RPFshStage::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPFshStage::~RPFshStage() {}

// status reports string-pool aliases (@stringBase0 vs lbl_) as 99.77%; the code matches.
void RPFshStage::Create() {
    const char* kclName = "lake.kcl";
    mpKcl->SetResource(RPSysResourceManager::GetFileFromArchive(
        RPSysResourceManager::LoadGameCommonArchive(), kclName));

    // Wii Play: the (handle, int) overload is called with the model name
    mpModels[MODEL_LAKE] = RPGrpModel::Construct(4, "fsh_lake", 1, 0, 0);
    mpModels[MODEL_TREE] = RPGrpModel::Construct(5, "fsh_tree", 1, 0, 0);
    mpModels[MODEL_CLOUD] = RPGrpModel::Construct(6, "fsh_cloud", 1, 0, 0);
    mpModels[MODEL_WATERFACE] = RPGrpModel::Construct(7, "fsh_waterface", 1, 0, 0);

    mpModels[MODEL_LAKE]->SetDrawScene(0);
    mpModels[MODEL_TREE]->SetDrawScene(0);
    mpModels[MODEL_CLOUD]->SetDrawScene(0);
    mpModels[MODEL_WATERFACE]->SetDrawScene(0);
    mpModels[MODEL_LAKE]->SetDrawGroup(2);

    const u32 treeHandle[] = {5};
    const char* treeName[] = {"fsh_tree"};
    for (int i = 0; i < 1; ++i) {
        mpModels[MODEL_TREE]->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_Chr, 1);
        mpModels[MODEL_TREE]->GetModelAnm()->RegistFromResFile(RPGrpModelAnm::Anm_Chr, 0,
                                                           treeHandle[i], treeName[i]);
        mpModels[MODEL_TREE]->GetModelAnm()->Start(RPGrpModelAnm::Anm_Chr, 0, 0.0f);
        mpModels[MODEL_TREE]->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_Chr, 0, 1.0f);
    }

    const u32 cloudHandle[] = {6};
    const char* cloudName[] = {"fsh_cloud"};
    for (int i = 0; i < 1; ++i) {
        mpModels[MODEL_CLOUD]->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_TexSrt, 1);
        mpModels[MODEL_CLOUD]->GetModelAnm()->RegistFromResFile(RPGrpModelAnm::Anm_TexSrt, 0,
                                                            cloudHandle[i], cloudName[i]);
        mpModels[MODEL_CLOUD]->GetModelAnm()->Start(RPGrpModelAnm::Anm_TexSrt, 0, 0.0f);
        mpModels[MODEL_CLOUD]->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_TexSrt, 0, 1.0f);
    }

    const u32 waterHandle[] = {7};
    const char* waterName[] = {"fsh_waterface"};
    for (int i = 0; i < 1; ++i) {
        mpModels[MODEL_WATERFACE]->GetModelAnm()->CreateBuffer(RPGrpModelAnm::Anm_TexSrt, 1);
        mpModels[MODEL_WATERFACE]->GetModelAnm()->RegistFromResFile(RPGrpModelAnm::Anm_TexSrt, 0,
                                                                waterHandle[i], waterName[i]);
        mpModels[MODEL_WATERFACE]->GetModelAnm()->Start(RPGrpModelAnm::Anm_TexSrt, 0, 0.0f);
        mpModels[MODEL_WATERFACE]->GetModelAnm()->SetUpdateRate(RPGrpModelAnm::Anm_TexSrt, 0, 1.0f);
    }

    mpShadowTexture = new RPGrpShadowTexture(0x100, 1, 0x80);
    mpShadowTexture->InitList();
    mpShadowTexture->SHORT_0x70 |= 2;
    mpShadowTexture->Replace(mpModels[MODEL_LAKE], "shadow_dummy");
    mpShadowTexture->BYTE_0x72 = 0x46;
    mpShadowTexture->FLOAT_0x74 = 1.1f;
    mpShadowTexture->VEC_0x78 = nw4r::math::VEC3(0.0f, -1.0f, 0.0f);
    RPGrpModelManager::GetCurrent()
        ->GetModelScene(0)
        ->GetDrawPathManager()
        ->GetDrawPathShadowTexture()
        ->SetTexture(0, mpShadowTexture);
    mpShadowTexture->ResetList();

    mpProjection->LoadScnCamera(0, "RPScene", 0.0f);
    RPGrpModelManager::GetCurrent()->GetModelScene(0)->SetProjection(mpProjection);
}

void RPFshStage::Reset() {}

void RPFshStage::Calculate() {
    mpShadowTexture->ResetList();

    RPGrpModel* models[MODEL_NUM] = {mpModels[MODEL_LAKE], mpModels[MODEL_TREE],
                                     mpModels[MODEL_CLOUD], mpModels[MODEL_WATERFACE]};
    EGG::Matrix34f mtx;
    mtx.makeIdentity();

    for (int i = 0; i < MODEL_NUM; i++) {
        RPGrpModel* pModel = models[i];
        pModel->GetModelEx()->setLocalMtx(&mtx);
        pModel->SetBaseScale(1.0f);
        pModel->UpdateFrame();
        pModel->Calc();
        pModel->Entry();
    }
}

bool RPFshStage::IsInPond(const nw4r::math::VEC3& rPos) {
    if (rPos.y <= -15.0f) {
        nw4r::math::VEC2 d(rPos.x - sPondCenter.x, rPos.z - sPondCenter.z);
        if (EGG::Math<f32>::sqrt(d.x * d.x + d.y * d.y) <= 210.0f) {
            return true;
        }
    }

    return false;
}

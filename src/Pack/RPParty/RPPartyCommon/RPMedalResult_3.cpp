#include <Pack/RPParty/RPPartyCommon/RPCommon.h>

#include <Pack/RPGraphics/RPGrpCamera.h>
#include <Pack/RPGraphics/RPGrpDrawPathLightMap.h>
#include <Pack/RPGraphics/RPGrpDrawPathManager.h>
#include <Pack/RPGraphics/RPGrpLightManager.h>
#include <Pack/RPGraphics/RPGrpModel.h>
#include <Pack/RPGraphics/RPGrpModelAnm.h>
#include <Pack/RPGraphics/RPGrpModelManager.h>
#include <Pack/RPGraphics/RPGrpModelResManager.h>
#include <Pack/RPGraphics/RPGrpModelScene.h>
#include <Pack/RPGraphics/RPGrpRenderer.h>
#include <Pack/RPGraphics/RPGrpScreen.h>
#include <Pack/RPGraphics/RPGrpView.h>
#include <Pack/RPKernel/RPSysController.h>
#include <Pack/RPKernel/RPSysKokeshi.h>
#include <Pack/RPKernel/RPSysKokeshiGenInfo.h>
#include <Pack/RPKernel/RPSysMessage.h>
#include <Pack/RPKernel/RPSysStringUtility.h>
#include <Pack/RPParty/RPPartyCommon/RPFaceCtrl.h>
#include <Pack/RPParty/RPPartyCommon/RPLeadCursor.h>
#include <Pack/RPParty/RPPartyCommon/RPMedalResult.h>
#include <Pack/RPParty/RPPartyCommon/RPMsgBoard.h>
#include <Pack/RPParty/RPPartyCommon/RPPauseText.h>
#include <Pack/RPParty/RPPartyCommon/RPResultMenu.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneInfo.h>
#include <Pack/RPParty/RPPartyCommon/RPScreenFade.h>
#include <Pack/RPParty/RPPartyCommon/RPVsWinner.h>
#include <Pack/RPSystem/RPSysPlayerMgr.h>
#include <Pack/RPSystem/RPSysResourceManager.h>

extern "C" s32 Menu_slotToGameId(s32 slot);
extern "C" s32 Menu_gameIdToSlot(s32 game);

RPCommon* RPCommon::sInstance;

inline void GetGameMsgIndexImpl(s32 num, s32 game, s32* pGameIdx, s32* pPlural) {
    *pPlural = 0;
    if (!RPSysStringUtility::IsSingular(num)) {
        *pPlural = 1;
    }
    s32 indices[] = {0, 1, 2, 3, 4, 5, 7, 8, 9};
    *pGameIdx = indices[game];
}

inline void PlayCommonAvatarAnm(RPSysKokeshi* pAvatar, s32 anm, f32 frame) {
    pAvatar->GetBodyModel()->GetModelAnm()->StartBlend(
        RPGrpModelAnm::Anm_Chr, anm, frame, 0.0f);
}

void RPCommon::CreateInstance(const RPCommonParam& rParam) {
    sInstance = new RPCommon(rParam);
}

RPCommon::~RPCommon() {}

void RPCommon::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
    RPMsgBoard::DestroyInstance();
}

// NONMATCHING: RPFaceCtrl's shared inline constructor removes the original call.
// Matching requires an out-of-line declaration in RPFaceCtrl.h, outside this unit's scope.
RPCommon::RPCommon(const RPCommonParam& rParam) {
    mParam = rParam;
    RPGrpModelManager::GetCurrent()->CreateModelScene(7, 1, 1, NULL, NULL);
    RPGrpRenderer* pRenderer = RPGrpRenderer::GetCurrent();
    mpScreen = new RPGrpScreen;
    mpScreen->SetCanvasMode(EGG::Frustum::CANVAS_CC);
    mpCamera = new RPGrpCamera;
    mpCamera->SetPosition(EGG::Vector3f::zero);
    mpCamera->SetTarget(nw4r::math::VEC3(0.0f, 0.0f, -1.0f));
    mpCamera->SetUp(nw4r::math::VEC3(0.0f, 1.0f, 0.0f));
    mpCamera->SetCameraType(RPGrpCamera::ECameraType_LookAt);
    mpCamera->CalcMatrix();
    pRenderer->CreateView2D(0x1A, mpScreen);
    pRenderer->CreateView3D(0x1B, mpCamera, mpScreen);
    pRenderer->CreateView2D(0x1C, mpScreen);
    pRenderer->GetView(0x1B)->SetDrawScene(7);
    pRenderer->GetView(0x1B)->SetFlag(8);
    pRenderer->GetView(0x1A)->CreateScreenEffect();
    mpMedalResult = new RPMedalResult;
    mpResultMenu = new RPResultMenu;
    mpVsWinner = new RPVsWinner;
    mpLeadCursor = new RPLeadCursor(mParam.mPlayerNum);
    mpPauseText = new RPPauseText;
    mpScreenFade = new RPScreenFade;
    RPMsgBoard::CreateInstance();
    mpAvatars[0] = NULL;
    mpAvatars[1] = NULL;
    for (s32 player = 0; player < mParam.mPlayerNum; ++player) {
        RPSysKokeshiGenInfo info;
        info.SetGenType(RPSysKokeshiManager::GenType_Player);
        info.SetPlayerNo(player);
        info.SetResolution(static_cast<RFLResolution>(0x100));
        info.SetBodyType(4);
        info.SetExpressionFlag(0x2B);
        info.SetBufferOption(1);
        mpAvatars[player] = new RPSysKokeshi(&info);
        mpFaceCtrls[player] = new RPFaceCtrl;
    }
    mEntryAvatars = true;
    RPGrpRenderer::GetCurrent()->AppendDrawObject(
        &static_cast<IRPGrpDrawObject&>(*this));
}

RPCommonParam::RPCommonParam() {
    mGame = 6;
    mPlayerNum = 1;
}

void RPCommon::LoadResources() {
    RPGrpModelResManager* pResManager = RPGrpModelResManager::GetCurrent();
    EGG::Archive* pArchive = RPSysResourceManager::instance()->GetStaticCommonArchive();
    RPGrpHandle scene = pResManager->CreateData(
        RPGrpModelResManager::Type_ResFile,
        RPSysResourceManager::GetFileFromArchive(pArchive, "G3D/sceneFile.brres", NULL), 0);
    static_cast<RPGrpLightManager*>(
        RPGrpModelManager::GetCurrent()->GetModelScene(7)->GetLightManager())
        ->LoadScnLight(scene, "RPScene", 0.0f, -1);
    for (s32 player = 0; player < mParam.mPlayerNum; ++player) {
        mpAvatars[player]->LoadResource(NULL);
        RPSysKokeshiManager::CreateLightTexture(7);
        void* pBinary = RPSysResourceManager::GetFileFromArchive(
            pArchive, "Environment/scene_Light.plight", NULL);
        RPGrpModelManager::GetCurrent()->GetLightManager(7)->SetBinary(pBinary);
        pBinary = RPSysResourceManager::GetFileFromArchive(
            pArchive, "Environment/scene_Lm.plmap", NULL);
        RPGrpDrawPathLightMap* pPath = RPGrpModelManager::GetCurrent()
            ->GetDrawPathManager(7)->GetDrawPathLightMap();
        pPath->GetLightTextureManager()->SetBinary(pBinary);
        pPath->GetLightTextureManager()->onFlag2();
        mpAvatars[player]->ApplyLightTexture(7);
        mpAvatars[player]->GetBodyModel()->SetDrawScene(7);
        mpAvatars[player]->GetNigaoeModel()->SetDrawScene(7);
    }
    RPGrpHandle animation = pResManager->CreateData(
        RPGrpModelResManager::Type_ResFile,
        RPSysResourceManager::GetFileFromArchive(pArchive, "G3D/avatar_anim.brres", NULL), 0);
    const char* names[] = {
        "cmn_waitR", "cmn_winR", "cmn_win2R", "cmn_loseR", "cmn_drawL", "cmn_drawR"
    };
    for (s32 player = 0; player < mParam.mPlayerNum; ++player) {
        RPGrpModelAnm* pAnm = mpAvatars[player]->GetBodyModel()->GetModelAnm();
        pAnm->CreateBuffer(RPGrpModelAnm::Anm_Chr, 6);
        for (s32 index = 0; index < 6; ++index) {
            pAnm->RegistFromResFile(RPGrpModelAnm::Anm_Chr, index, animation, names[index], 0);
        }
    }
    void* pMessage = RPSysResourceManager::GetMessageResource("party_message.bmg");
    mpMessage = new RPSysMessage(pMessage, NULL);
    mpMedalResult->Load();
    mpResultMenu->Load();
    mpVsWinner->Load();
    mpLeadCursor->Load();
    mpPauseText->Load();
    RPMsgBoard::GetInstance()->LoadMessages();
}

void RPCommon::Reset() {
    unkC = 0;
    RPGrpView::ScreenEffect* pEffect =
        RPGrpRenderer::GetCurrent()->GetView(0x1A)->GetScreenEffect();
    GXColor color = {255, 255, 255, 0};
    pEffect->SetColorMultiply(color);
    pEffect->SetBlurAlpha(0);
    pEffect->SetBlurOffset(1.0f);
    mpMedalResult->Reset();
    mpResultMenu->Reset();
    mpVsWinner->Reset();
    mpLeadCursor->Reset();
    mpPauseText->Reset();
    mpScreenFade->Reset();
    unk30 = true;
    RPSceneInfo* pSceneInfo = RPSceneInfo::spInstance;
    pSceneInfo->unk18 = 4;
    pSceneInfo->unk14 = -1;
}

void RPCommon::Calculate() {
    mpMedalResult->Calculate();
    mpResultMenu->Calculate();
    mpVsWinner->Calculate();
    mpLeadCursor->Calculate();
    mpPauseText->Calculate();
    mpScreenFade->Calculate();
}

void RPCommon::UserDraw() {
    if (RPGrpRenderer::GetCurrentViewNo() == 0x1A) {
        if (RPGrpRenderer::GetDrawStep() == 5) {
            mpMedalResult->DrawLayer1A();
            mpResultMenu->DrawLayer1A();
            mpVsWinner->DrawLayer1A();
        }
    } else if (RPGrpRenderer::GetCurrentViewNo() == 0x1C) {
        if (RPGrpRenderer::GetDrawStep() == 5) {
            mpMedalResult->DrawLayer1C();
            mpResultMenu->DrawLayer1C();
            mpVsWinner->DrawLayer1C();
            mpPauseText->Draw();
            mpLeadCursor->Draw();
        }
    }
}

u16 RPCommon::GetResourceNum() {
    return 16;
}

u8 RPCommon::GetLayoutViewNo(s32 layer) {
    return layer + 0x1A;
}

s32 RPCommon::GetSavedMedal() const {
    const RPSysPlayer& rPlayer = RPSysPlayerMgr::instance()->getPlayer(0);
    const RPPartyPlayerData* pData = rPlayer.mpPartyPlayerData;
    return pData->getMedal(static_cast<RPPartyPlayerData::EGame>(mParam.mGame));
}

u8 RPCommon::GetEffectDrawLayer() {
    return 7;
}

EGG::Vector3f RPCommon::ScreenToWorld(const nw4r::math::VEC2& rScreenPos) const {
    const RPGrpCamera* pCamera = mpCamera;
    const RPGrpScreen* pScreen = mpScreen;
    EGG::Vector3f worldPos;
    pCamera->GetScreenToWorld(&worldPos, rScreenPos, pScreen);
    return worldPos;
}

void RPCommon::SetAvatarScale(s32 player, f32 scale, const nw4r::math::MTX34& rBodyMtx) {
    RPGrpModel* pModel = GetAvatar(player)->GetBodyModel();
    if (pModel->GetScnObj() != NULL) {
        pModel->GetScnObj()->SetMtx(nw4r::g3d::ScnObj::MTX_LOCAL, &rBodyMtx);
    }
    pModel->SetBaseScale(scale);
    pModel->UpdateFrame();
    pModel->Calc();
    if (mEntryAvatars) {
        pModel->Entry();
    }
    pModel = GetAvatar(player)->GetNigaoeModel();
    EGG::Matrix34f headMtx;
    GetAvatar(player)->GetHeadJointMtx(&headMtx);
    if (pModel->GetScnObj() != NULL) {
        pModel->GetScnObj()->SetMtx(nw4r::g3d::ScnObj::MTX_LOCAL, &headMtx);
    }
    pModel->SetBaseScale(scale);
    pModel->Calc();
    if (mEntryAvatars) {
        pModel->Entry();
    }
}

#pragma push
#pragma peephole off
void RPCommon::PlayAvatarAnm(s32 player, u16 anm, f32 frame) {
    PlayCommonAvatarAnm(mpAvatars[player], anm, frame);
}
#pragma pop

bool RPCommon::IsAPressed(s32) {
    return RPSysCoreController::getActiveController()->downTrigger(EGG::cCORE_BUTTON_A);
}

bool RPCommon::IsAnyABPressed(s32 playerNum) {
    for (s32 player = 0; player < playerNum; ++player) {
        if (RP_GET_CORE_CTRL(player)->downTrigger(EGG::cCORE_BUTTON_A) ||
            RP_GET_CORE_CTRL(player)->downTrigger(EGG::cCORE_BUTTON_B)) {
            return true;
        }
    }
    return false;
}

void RPCommon::GetGameMsgIndex(s32 num, s32* pGameIdx, s32* pPlural) const {
    GetGameMsgIndexImpl(num, mParam.mGame, pGameIdx, pPlural);
}

void RPCommon::GetGameMsgIndex(s32 num, s32 game, s32* pGameIdx, s32* pPlural) {
    GetGameMsgIndexImpl(num, game, pGameIdx, pPlural);
}

s32 RPCommon::GetNextGame() const {
    s32 game = Menu_slotToGameId(GetGame());
    if (game < 8) {
        return Menu_gameIdToSlot(game + 1);
    }
    return 9;
}

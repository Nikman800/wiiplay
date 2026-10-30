// Class declaration order ModelScene, LightManager, FogManager: the vtables come out in reverse
// order, the explicit inline destructors in declaration order
#include <Pack/RPGraphics/RPGrpModelScene.h>
#include <Pack/RPGraphics/RPGrpModelResManager.h>
#include <egg/gfxe.h>

#include <Pack/RPGraphics/RPGrpLightManager.h>
#include <Pack/RPGraphics/RPGrpFogManager.h>
#include <Pack/RPGraphics.h>
#include <Pack/RPAssert.h>

#include <nw4r/g3d.h>

// Allocator for the g3d scene roots (set by RPGrpModelManager). Wii Play: this
// file's .sbss (8043C5C0)
EGG::Allocator* RPGrpModelScene::spAllocator;

// Linked units call these by their ledger names
extern "C" void RP_801AFB80();
namespace {

// Draw-list links and priority offsets are protected in RPGrpModel
class RPGrpModelSceneModelAccess : public RPGrpModel {
public:
    static RPGrpModel* EntryNext(RPGrpModel* pModel) {
        return static_cast<RPGrpModelSceneModelAccess*>(pModel)->mpEntryNext;
    }
    static void SetEntryNext(RPGrpModel* pModel, RPGrpModel* pNext) {
        static_cast<RPGrpModelSceneModelAccess*>(pModel)->mpEntryNext = pNext;
    }
    static u8 DrawGroup(RPGrpModel* pModel) {
        return static_cast<RPGrpModelSceneModelAccess*>(pModel)->mDrawGroup;
    }
    static u8 OpaOffset(RPGrpModel* pModel) {
        return static_cast<RPGrpModelSceneModelAccess*>(pModel)->unk3;
    }
    static u8 XluOffset(RPGrpModel* pModel) {
        return static_cast<RPGrpModelSceneModelAccess*>(pModel)->unk4;
    }
    static void ClearEntered(RPGrpModel* pModel) {
        static_cast<RPGrpModelSceneModelAccess*>(pModel)->mFlags &= ~EFlag_Entered;
    }
};

// The draw settings are protected in EGG::ScnRootEx
class RPGrpModelSceneRootAccess : public EGG::ScnRootEx {
public:
    static void SetDitherOpa(EGG::ScnRootEx* pRoot) {
        static_cast<RPGrpModelSceneRootAccess*>(pRoot)->mDrawSettings |= cDrawSetting_DitherOpa;
    }
    static void SetDitherXlu(EGG::ScnRootEx* pRoot) {
        static_cast<RPGrpModelSceneRootAccess*>(pRoot)->mDrawSettings |= cDrawSetting_DitherXlu;
    }
    static void ClearDitherOpa(EGG::ScnRootEx* pRoot) {
        static_cast<RPGrpModelSceneRootAccess*>(pRoot)->mDrawSettings &= ~cDrawSetting_DitherOpa;
    }
    static void ClearDitherXlu(EGG::ScnRootEx* pRoot) {
        static_cast<RPGrpModelSceneRootAccess*>(pRoot)->mDrawSettings &= ~cDrawSetting_DitherXlu;
    }
};

inline void ClearModelSceneDrawLists(const u8& count, RPGrpModelScene::DrawList*& rpLists) {
    for (int i = 0; i < count; i++) {
        RPGrpModelScene::DrawList& rList = rpLists[i];
        rList.pHead = rList.pTail = NULL;
    }
}

} // namespace

const u32 RPGrpModelScene::sOpaPriorityPaths[] = {EGG::ScnRenderer::cTiming_Opa4th,
                                           EGG::ScnRenderer::cTiming_Opa1st,
                                           EGG::ScnRenderer::cTiming_Opa2nd};
const u32 RPGrpModelScene::sXluPriorityPaths[] = {EGG::ScnRenderer::cTiming_Xlu1st,
                                           EGG::ScnRenderer::cTiming_Xlu1st,
                                           EGG::ScnRenderer::cTiming_Xlu1st};

void RP_801AFB80() {}

RPGrpModelScene::RPGrpModelScene(u32 pathFlags, u8 lightNum, EGG::LightManager* pLightManager,
                                 EGG::FogManager* pFogManager)
    : mDrawListNum(0),
      mLightNum(lightNum),
      mFlags(0x1C),
      mpOpaPriorities(NULL),
      mpXluPriorities(NULL),
      mpLightManager(pLightManager),
      mpFogManager(pFogManager),
      mpDrawPathManager(NULL),
      mpScnRoot(NULL),
      mpDrawLists(NULL) {
    u32 size;
    mpScnRoot = nw4r::g3d::ScnRoot::Construct(RPGrpModelScene::spAllocator, &size, 0x400, 0x400, 16, 8);

    nw4r::math::MTX34 mtx;
    PSMTXIdentity(mtx);
    for (int i = 0; i < 32; i++) {
        mpScnRoot->GetCamera((u8)i).SetCameraMtxDirectly(mtx);
    }

    if (mpLightManager == NULL) {
        RPGrpLightManager* pNewLight = new RPGrpLightManager(16, 8, lightNum);
        mpLightManager = pNewLight;
        pNewLight->Reset();
    }

    for (int i = 0; i < 8; i++) {
        nw4r::g3d::LightSet l = mpScnRoot->GetLightSet(i);
#line 90
        RP_ASSERT(l.IsValid());
        bool success = l.SelectAmbLightObj(0);
#line 92
        RP_ASSERT(success);

        for (int j = 0; j < mpLightManager->GetNum(); j++) {
            if (j < 8) {
                success = l.SelectLightObj(j, j);
#line 98
                RP_ASSERT(success);
            }
        }
    }

    if (mpFogManager == NULL) {
        RPGrpFogManager* pNewFog = new RPGrpFogManager();
        mpFogManager = pNewFog;
        pNewFog->Reset();
    }

    u32 createFlags = 0;
    if (pathFlags & 0x4) {
        createFlags |= EGG::ScnRenderer::cCreateFlag_XluSnap;
    }
    if (pathFlags & 0x8) {
        createFlags |= EGG::ScnRenderer::cCreateFlag_SV;
    }
    if (pathFlags & 0x10) {
        createFlags |= EGG::ScnRenderer::cCreateFlag_HDR;
    }
    if (pathFlags & 0x20) {
        createFlags |= EGG::ScnRenderer::cCreateFlag_Bloom;
    }
    if (pathFlags & 0x40) {
        createFlags |= EGG::ScnRenderer::cCreateFlag_DOF;
    }

    mpScnRenderer = new EGG::ScnRenderer(mpScnRoot);
    mpScnRenderer->configure();
    mpScnRenderer->setLightManager(mpLightManager);
    mpScnRenderer->setFogManager(mpFogManager);
    mpScnRenderer->createPath(createFlags, RPGrpModelScene::spAllocator);

    mpOpaPriorities = new u8[3];
    mpXluPriorities = new u8[3];
    for (int i = 0; i < 3; i++) {
        mpScnRenderer->getTimingPriorityRange(RPGrpModelScene::sOpaPriorityPaths[i], &mpOpaPriorities[i],
                                              NULL);
        mpScnRenderer->getTimingPriorityRange(RPGrpModelScene::sXluPriorityPaths[i], &mpXluPriorities[i],
                                              NULL);
    }

    mpDrawPathManager = RPGrpDrawPathManager::Construct(pathFlags, this);

    mDrawListNum = 1;
    mpDrawLists = new DrawList[1];
    ClearModelSceneDrawLists(mDrawListNum, mpDrawLists);

    for (int i = 0; i < 32; i++) {
        mpProjections[i] = NULL;
    }
}

RPGrpModelScene::~RPGrpModelScene() {
    delete mpScnRenderer;
}

void RPGrpModelScene::ClearDrawLists() {
    for (int i = 0; i < mDrawListNum; i++) {
        DrawList& rList = mpDrawLists[i];
        for (RPGrpModel* pModel = rList.pHead; pModel != NULL;
             pModel = RPGrpModelSceneModelAccess::EntryNext(pModel)) {
            RPGrpModelSceneModelAccess::ClearEntered(pModel);
        }
        rList.pHead = rList.pTail = NULL;
    }

    mpScnRoot->Clear();
    mFlags |= 0x10;
}

void RPGrpModelScene::CalcView() {
    mpScnRenderer->UpdateFrame();
    mpScnRenderer->CalcWorld();
}

void RPGrpModelScene::CalcDrawList() {
    for (int i = 0; i < mDrawListNum; i++) {
        DrawList& rList = mpDrawLists[i];
        if (rList.pTail != NULL) {
            RPGrpModelSceneModelAccess::SetEntryNext(rList.pTail, NULL);
            for (RPGrpModel* pModel = rList.pHead; pModel != NULL;
                 pModel = RPGrpModelSceneModelAccess::EntryNext(pModel)) {
                mpScnRoot->PushBack(pModel->GetScnObj());
                mFlags |= 0x1;
            }
        }
    }

    unk2 = 0;
    for (int i = 0; i < 32; i++) {
        if (mpProjections[i] != NULL) {
            mpProjections[i]->Calc(mpScnRoot->GetCamera(i));
        }
    }

    mpScnRenderer->pushBackDrawPath();
    mpDrawPathManager->pushBackToScnGroup(mpScnRoot);
}

void RPGrpModelScene::ResetViews() {
    unk2 = 0;
    mpScnRenderer->CalcVtx();
    mpScnRenderer->CalcMaterial();
}

void RPGrpModelScene::DrawPrepare(const RPGrpCamera* pCamera, const RPGrpScreen* pScreen) {
    mpScnRenderer->SetCurrentCamera(unk2++, pCamera->GetSavedCameraMatrix(), *pScreen);
    mpScnRenderer->CalcView();
}

void RPGrpModelScene::Draw() {
    for (int i = 0; i < mDrawListNum; i++) {
        for (RPGrpModel* pModel = mpDrawLists[i].pHead; pModel != NULL;
             pModel = RPGrpModelSceneModelAccess::EntryNext(pModel)) {
            pModel->CalcBeforeDraw();
            if (pModel->IsVisible()) {
                SetModelDrawPriority(pModel);
            }
        }
    }

    mpScnRenderer->GatherDrawScnObj();
    mpScnRenderer->ZSort();

    if (mFlags & 0x4) {
        RPGrpModelSceneRootAccess::SetDitherOpa(mpScnRenderer);
        RPGrpModelSceneRootAccess::SetDitherXlu(mpScnRenderer);
    } else {
        RPGrpModelSceneRootAccess::ClearDitherOpa(mpScnRenderer);
        RPGrpModelSceneRootAccess::ClearDitherXlu(mpScnRenderer);
    }

    mpScnRenderer->DrawOpa();
    mpScnRenderer->DrawXlu();
}

void RPGrpModelScene::DrawFinish() {
    mpScnRenderer->finishDraw();
}

void RPGrpModelScene::SetProjection(RPGrpProjectionObject* pProjection) {
    mpProjections[pProjection->GetViewNo()] = pProjection;
}

void RPGrpModelScene::SetModelDrawPriority(RPGrpModel* pModel) {
    u8 group = RPGrpModelSceneModelAccess::DrawGroup(pModel);
    pModel->GetScnObj()->SetPriorityDrawOpa(
        (u8)(mpOpaPriorities[group] + RPGrpModelSceneModelAccess::OpaOffset(pModel)));
    pModel->GetScnObj()->SetPriorityDrawXlu(
        (u8)(mpXluPriorities[group] + RPGrpModelSceneModelAccess::XluOffset(pModel)));
}

void RPGrpModelScene::AppendDrawModel(RPGrpModel* pModel) {
    DrawList* pList = mpDrawLists;
    if (pList->pTail != NULL) {
        RPGrpModelSceneModelAccess::SetEntryNext(pList->pTail, pModel);
    } else {
        pList->pHead = pModel;
    }
    pList->pTail = pModel;
}

void RPGrpModelScene::PreConditionForRenderingGX() const {
    mpScnRenderer->getScreen().SetProjectionGX();
    mpLightManager->LoadAmbientColorGX();
    mpLightManager->LoadObjIndxGX();
    mpFogManager->GetFog(0)->SetGX();
}

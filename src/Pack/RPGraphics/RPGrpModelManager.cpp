// RPGrpModelMisc first: the vtables come out in reverse class order
#include <Pack/RPGraphics/RPGrpModelMisc.h>
#include <Pack/RPGraphics/RPGrpModelManager.h>
#include <Pack/RPGraphics.h>

#include <egg/gfxe.h>

#include <nw4r/g3d.h>

// Allocator for the g3d scene roots (RPGrpModelScene's .sbss, ledger name)
// EGG::G3DUtility's allocator (eggG3DUtility .sbss 8043BB74, private there; ledger name)
// Empty function in RPGrpModelScene's range (ledger name)
extern "C" void RP_801AFB80();

RPGrpModelManager* RPGrpModelManager::spCurrent;

// The draw-entry list statics are protected in RPGrpModel
class RPGrpModelManagerModelAccess : public RPGrpModel {
public:
    static RPGrpModel*& EntryHead() {
        return spEntryHead;
    }
    static RPGrpModel*& EntryTail() {
        return spEntryTail;
    }
    static RPGrpModel*& GenList() {
        return spGenList;
    }
    static RPGrpModel* EntryNext(RPGrpModel* pModel) {
        return static_cast<RPGrpModelManagerModelAccess*>(pModel)->mpEntryNext;
    }
    static void SetEntryNext(RPGrpModel* pModel, RPGrpModel* pNext) {
        static_cast<RPGrpModelManagerModelAccess*>(pModel)->mpEntryNext = pNext;
    }
    static u8 DrawScene(RPGrpModel* pModel) {
        return static_cast<RPGrpModelManagerModelAccess*>(pModel)->mDrawScene;
    }
};

// The g3d root is protected in EGG::ScnRootEx
class RPGrpModelManagerScnRootAccess : public EGG::ScnRootEx {
public:
    static nw4r::g3d::ScnRoot* ScnRoot(EGG::ScnRootEx* pRootEx) {
        return static_cast<RPGrpModelManagerScnRootAccess*>(pRootEx)->mpScnRoot;
    }
};

// Inline: emitted after the weak IBinary<RPGrpModelMisc> copy, with a weak vtable
inline RPGrpModelManager::~RPGrpModelManager() {}

void RPGrpModelMisc::GetBinaryInner(Bin* /* pBinary */) const {}

void RPGrpModelMisc::SetBinaryInner(const Bin& /* rBinary */) {}

RPGrpModelMisc::~RPGrpModelMisc() {}

u8 RPGrpModelMisc::GetVersion() const {
    return VERSION;
}

RPGrpModelManager::RPGrpModelManager(EGG::Allocator* pAllocator, EGG::Allocator* pScnAllocator)
    : mpAllocator(pAllocator), mpScnAllocator(pScnAllocator), mCalcDisabled(false) {
    RPGrpModelManagerModelAccess::GenList() = NULL;
    mpResManager = new RPGrpModelResManager();
    mpResManager->Configure();

    for (int i = 0; i < MAX_SCENE; i++) {
        mpModelScenes[i] = NULL;
    }

    mpModelMisc = new RPGrpModelMisc();
}

void RPGrpModelManager::BecomeCurrent() {
    spCurrent = this;
    RPGrpModelResManager::spCurrent = mpResManager;
    RPGrpModel::SetAllocator(mpAllocator);
    RPGrpModelManagerModelAccess::EntryTail() = NULL;
    RPGrpModelManagerModelAccess::EntryHead() = NULL;
    RPGrpModelScene::spAllocator = mpScnAllocator;
    EGG::G3DUtility::setAllocator(mpAllocator);
}

void RPGrpModelManager::CreateModelScene(u8 drawScene, u32 pathFlags, u8 lightNum,
                                         EGG::LightManager* pLightManager,
                                         EGG::FogManager* pFogManager) {
    mpModelScenes[drawScene] =
        new RPGrpModelScene(pathFlags, lightNum, pLightManager, pFogManager);
}

void RPGrpModelManager::ClearDrawEntries() {
    for (int i = 0; i < MAX_SCENE; i++) {
        if (mpModelScenes[i] != NULL) {
            mpModelScenes[i]->ClearDrawLists();
        }
    }
}

static inline void RPGrpModelManager_CalcViews(RPGrpModelScene** ppScenes) {
    for (int i = 0; i < RPGrpModelManager::MAX_SCENE; i++) {
        if (ppScenes[i] != NULL) {
            ppScenes[i]->CalcView();
        }
    }
}

void RPGrpModelManager::CalcModelScenes() {
    if (mCalcDisabled) {
        return;
    }

    RPGrpModelManager_CalcViews(mpModelScenes);

    if (RPGrpModelManagerModelAccess::EntryTail() != NULL) {
        RPGrpModelManagerModelAccess::SetEntryNext(RPGrpModelManagerModelAccess::EntryTail(),
                                                   NULL);
        for (RPGrpModel* pModel = RPGrpModelManagerModelAccess::EntryHead(); pModel != NULL;
             pModel = RPGrpModelManagerModelAccess::EntryNext(pModel)) {
            mpModelScenes[RPGrpModelManagerModelAccess::DrawScene(pModel)]->AppendDrawModel(
                pModel);
        }
        RPGrpModelManagerModelAccess::EntryTail() = NULL;
        RPGrpModelManagerModelAccess::EntryHead() = NULL;
    }

    for (int i = 0; i < MAX_SCENE; i++) {
        if (mpModelScenes[i] != NULL) {
            mpModelScenes[i]->CalcDrawList();
        }
    }
}

void RPGrpModelManager::CalcWorldModelScenes() {
    for (int i = 0; i < MAX_SCENE; i++) {
        if (mpModelScenes[i] != NULL) {
            RPGrpModelManagerScnRootAccess::ScnRoot(mpModelScenes[i]->GetScnRenderer())
                ->SetScnObjOption(nw4r::g3d::ScnObj::OPTID_DISABLE_CALC_WORLD, TRUE);
            mpModelScenes[i]->GetScnRenderer()->CalcWorld();
            RPGrpModelManagerScnRootAccess::ScnRoot(mpModelScenes[i]->GetScnRenderer())
                ->SetScnObjOption(nw4r::g3d::ScnObj::OPTID_DISABLE_CALC_WORLD, FALSE);
        }
    }
}

void RPGrpModelManager::ResetViews() {
    RP_801AFB80();
    for (int i = 0; i < MAX_SCENE; i++) {
        if (mpModelScenes[i] != NULL) {
            mpModelScenes[i]->ResetViews();
        }
    }
}

void RPGrpModelManager::DrawPost() {
    for (int i = 0; i < MAX_SCENE; i++) {
        if (mpModelScenes[i] != NULL) {
            mpModelScenes[i]->DrawFinish();
        }
    }
}

void RPGrpModelManager::TerminateResources() {
    mpResManager->TerminateAll();
    if (spCurrent == this) {
        ClearCurrent();
    }
}

void RPGrpModelManager::ClearCurrent() {
    RPGrpModel::SetAllocator(NULL);
    RPGrpModelScene::spAllocator = NULL;
    RPGrpModelResManager::spCurrent = NULL;
    spCurrent = NULL;
}

RPGrpDrawPathManager* RPGrpModelManager::GetDrawPathManager(u8 drawScene) const {
    return mpModelScenes[drawScene]->GetDrawPathManager();
}

EGG::LightManager* RPGrpModelManager::GetLightManager(u8 drawScene) const {
    return mpModelScenes[drawScene]->GetLightManager();
}

// Charge! game manager, the whole original source file; linked DOL verified byte for byte.
// .text 0x80295DD0-0x8029CF74 = today's units RPCowGame, RPCowHud, RPCowModelBatch(_2,_3),
// RPCowPlayer(_2), RPCowCamera, RPCowGame_2, RPCowHud_2, RPCowPlayer_3, RPCowPlayer_4 and the
// head of RPCowCamera_2 (camera ctor, OnTimeUp, OnCourseComplete, __sinit). One .sdata2 pool
// (0x80440990-0x804409C0), one .ctors entry (0x802E9DF8 -> __sinit at 0x8029CF20).
// The small constructors/destructors were weak copies emitted after the first function (in
// output order) that needed them; here they are strong definitions placed where the copies
// come out (normal functions are emitted last-defined first).
// Every function matches the original instructions; LoadResource retains string-pool labels.

#include <Pack/RPParty/RPCowScene/RPCowGame.h>

#include <Pack/RPAudio.h>
#include <Pack/RPGraphics.h>
#include <Pack/RPGraphics/RPGrpDrawPathManager.h>
#include <Pack/RPGraphics/RPGrpDrawPathSV.h>
#include <Pack/RPGraphics/RPGrpLightManager.h>
#include <Pack/RPKernel/RPSysEffectCreator.h>
#include <Pack/RPKernel/RPSysLytResAccessor.h>
#include <Pack/RPParty/RPCowScene/RPCowCamera.h>
#include <Pack/RPParty/RPCowScene/RPCowCowModel.h>
#include <Pack/RPParty/RPCowScene/RPCowField.h>
#include <Pack/RPParty/RPCowScene/RPCowFieldObj.h>
#include <Pack/RPParty/RPCowScene/RPCowHud.h>
#include <Pack/RPParty/RPCowScene/RPCowKakasiGroup.h>
#include <Pack/RPParty/RPCowScene/RPCowModelBatch.h>
#include <Pack/RPParty/RPCowScene/RPCowPlayer.h>
#include <Pack/RPParty/RPCowScene/RPCowPointPopup.h>
#include <Pack/RPParty/RPPartyCommon/RPCls803572E0.h>
#include <Pack/RPParty/RPPartyCommon/RPColliderList.h>
#include <Pack/RPParty/RPPartyCommon/RPCollisionModel.h>
#include <Pack/RPParty/RPPartyCommon/RPCommon.h>
#include <Pack/RPParty/RPPartyCommon/RPMedalResult.h>
#include <Pack/RPParty/RPPartyCommon/RPPauseText.h>
#include <Pack/RPParty/RPPartyCommon/RPResFile.h>
#include <Pack/RPParty/RPPartyCommon/RPResultMenu.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneInfo.h>
#include <Pack/RPParty/RPPartyCommon/RPTextWriter.h>
#include <Pack/RPParty/RPPartyCommon/RPVsWinner.h>
#include <Pack/RPSystem/RPSysResourceManager.h>
#include <Pack/RPSystem/RPSysScene.h>
#include <Pack/RPSystem/RPSysSceneCreator.h>
#include <Pack/RPSystem/RPSysSceneMgr.h>
#include <Pack/RPUtility/RPUtlRandom.h>

#include <egg/gfxe.h>

// Wii Play: these shared entry points retain their original C symbol names.
extern "C" void RP_Cls803572E0_createInstance();
// RPActorList member (this unused): RPGrpModelResManager::CreateResourceList(num)
// Stops the Charge! BGM handle (8043C9A0) if it is playing
extern "C" void Cow_stopBgm();

// Actor idx of a table slot (the index is a u16)
template <typename T>
inline T* CowGetActor(RPActor** ppActors, int slot, u16 idx) {
    return &static_cast<T*>(ppActors[slot])[idx];
}

extern "C" void Cow_playBgm(u32 id);
extern "C" bool Cow_holdGlobalSe(u32 id);
extern "C" u32 RP_actorSceneFlags;
extern "C" u8 Lyt_getCurrentViewNo();
extern "C" s32 Lyt_getDrawStep();

// Base resource count of the draw resource list; set by the static initializer
inline u16 CowGetDrawPriorityBase() {
    return 0x60;
}
u16 RPCowGame::sDrawPriorityBase = CowGetDrawPriorityBase(); // [L] a dynamic initializer giving 0x60
RPColliderList RPCowGame::sColliderListA;
RPColliderList Cow_objectColliders;
RPColliderList RPCowGame::sColliderListB;

namespace {

inline u16 GetSoundPlayerNum(const RPCowGame* pGame) {
    return pGame->GetPlayerNum() < 2 ? pGame->GetPlayerNum() : 2;
}

} // namespace

template <typename T>
inline T* CowNewPlaced(u16 num) {
    return num > 1 ? new T[num] : new T();
}

// Registers num actors (last first) with group = table slot
template <typename T>
inline void RPCowGame::RegistArray(T* pActors, int num, u16 group, u16 key) {
    for (int i = num - 1; i >= 0; i--) {
        RegistActor(&pActors[i], group, i, key);
    }
}
// One actor (num 1) or an array of num, registered and stored in a table slot
template <typename T>
inline void RPCowGame::CreateActors(int num, int slot, u16 key) {
    T* pActors = num > 1 ? new T[num] : new T();
    RegistArray(pActors, num, slot, key);
    mpTableA[slot] = pActors;
    mpTableNum[slot] = num;
}
// A single actor, registered through a one-iteration loop
template <typename T>
inline void RPCowGame::CreateActor(int slot, u16 key) {
    T* pActor = new T();
    for (int i = 0; i >= 0; i--) {
        RegistActorNoIndex(&pActor[i], slot, key);
    }
    mpTableA[slot] = pActor;
    mpTableNum[slot] = 1;
}
// Placed objects of one resource type of the course's placement table
template <typename T>
inline void RPCowGame::CreatePlaced(RPResTable* pTable, u32 type, int slot, u16 key) {
    u16 num = pTable->CountEntriesOfType(type);
    if (num != 0) {
        T* pActors = CowNewPlaced<T>(num);
        RegistArray(pActors, num, slot, key);
        mpTableA[slot] = pActors;
        mpTableNum[slot] = num;
    }
}

inline RPCowKakasiBatch::RPCowKakasiBatch() {}
inline RPCowHurdleBatch::RPCowHurdleBatch() {}
inline RPCowTreeBatch::RPCowTreeBatch() {}

void RPCowGame::OnCourseComplete(s32 player) {
    if (mState == 7 || mState == 8) {
        static_cast<RPCowLimitCount*>(GetSlotActor(26))->HideMessage();
        static_cast<RPCowMainHud*>(GetSlotActor(18))->SetTimeWarning(false);
        static_cast<RPCowGoalMsg*>(GetSlotActor(25))->Show(GetKnockedGroups() >= GetSpawnedGroups());
        for (int i = 0; i < mPlayerNum; i++) {
            CowGetActor<RPCowCamera>(mpTableA, 0, i)->StartFinish();
        }
        unk44 = player;
        mTimer = 60;
        mState = 9;
        Cow_playBgm(15);
        RPSndAudioMgr::instance()->startSystemSe(0x51);
        RPSndAudioMgr::instance()->startStream(0x93);
    }
}

// Keep the score comparison's temporaries inside this inline helper.
inline void CowSetTimeUpWinner(RPActor* pActor, s32& winner) {
    RPCowPlayer* pPlayer0 = static_cast<RPCowPlayer*>(pActor);
    RPCowPlayer* pPlayer1 = &pPlayer0[1];
    if (pPlayer0->GetTotalScore() > pPlayer1->GetTotalScore()) {
        winner = 0;
    } else if (pPlayer0->GetTotalScore() < pPlayer1->GetTotalScore()) {
        winner = 1;
    } else {
        winner = -1;
    }
}

void RPCowGame::OnTimeUp(s32 player) {
    int i;
    mTimeUp[player] = 1;
    if (mState == 7 || mState == 8) {
        static_cast<RPCowLimitCount*>(GetSlotActor(26))->ShowFinish();
        static_cast<RPCowMainHud*>(GetSlotActor(18))->SetTimeWarning(false);
        for (i = 0; i < mPlayerNum; i++) {
            CowGetActor<RPCowCamera>(mpTableA, 0, i)->StartFinish();
        }
        if (mPlayerNum <= 1) {
            mTimer = 60;
            mState = 15;
        } else {
            CowSetTimeUpWinner(GetSlotActor(7), unk48);
            mTimer = 60;
            mState = 19;
        }
        Cow_playBgm(15);
        {
            RPSndAudioMgr* pAudio = RPSndAudioMgr::instance();
            pAudio->startStream(0x94);
        }
        if (mPlayerNum <= 1) {
            {
                RPSndAudioMgr* const pAudio = RPSndAudioMgr::instance();
                pAudio->startSystemSe(0x53);
            }
        } else {
            {
                RPSndAudioMgr* const pAudio = RPSndAudioMgr::instance();
                pAudio->startSystemSe(0x52);
            }
        }
    }
}

RPCowCamera::RPCowCamera() {}

RPCowPosHistory::RPCowPosHistory() {}

RPCowPlayer::RPCowPlayer() {}

RPCowBalloon::RPCowBalloon() {}

void RPCowSceneSound::initSound3D(s32 arg) {
    switch (arg) {
    case 0: {
        EGG::TAudioSound3DMgr<4>::TAudioSound3DMgrArg param;
        param.maxPrioReduction = 32;
        param.interiorSize = 1800.0f;
        param.maxVolumeDistance = 100.0f;
        param.unitDistance = 900.0f;
        mpAudioMgr->setParam3D(&param);
        break;
    }
    }
}

// Single actors use an enum-typed zero index to preserve the original constant scheduling.
RPCowGame::RPCowGame() {
    RPCowSceneSound* pSceneSound;
    RPSceneInfo* pInfo = RPSceneInfo::spInstance;
    if (pInfo != NULL) {
        mPlayerNum = pInfo->GetPlayerNum() < 2 ? pInfo->GetPlayerNum() : 2;
        if (pInfo->GetUnk1C() == 0) {
            if (mPlayerNum <= 1) {
                mCourse = 1;
            } else {
                mCourse = 2;
            }
        } else {
            mCourse = pInfo->GetUnk1C();
        }
    } else {
        mPlayerNum = 1;
        mCourse = 0;
    }

    if (mPlayerNum <= 1) {
        RPGrpModelManager::GetCurrent()->CreateModelScene(0, 0xB, 2, NULL, NULL);
        RPGrpModelManager::GetCurrent()
            ->GetDrawPathManager(0)
            ->GetDrawPathSV()
            ->GetShadowVolume()
            ->OffSVFlag(2);
        GXColor shadowColor = {0, 0, 0x14, 0x64};
        RPGrpModelManager::GetCurrent()
            ->GetDrawPathManager(0)
            ->GetDrawPathSV()
            ->GetShadowVolume()
            ->SetShadowColor(shadowColor);
    } else {
        RPGrpModelManager::GetCurrent()->CreateModelScene(0, 3, 2, NULL, NULL);
    }

    CreateResourceList(RPCowGame::sDrawPriorityBase + RPCommon::GetResourceNum());
    AllocTables(0x1D);

    CreateActors<RPCowCamera>(2, 0, 0x1000);

    CreateActor<RPCowField>(1, 0x2000);

    CreateActor<RPCowModelBatch>(16, 0x8000);

    CreateActors<RPCowKakasi>(32, 2, 0x4000);

    CreateActor<RPCowKakasiBatch>(3, 0x8000);

    CreateActors<RPCowKakasiKing>(8, 4, 0x4000);

    CreateActor<RPCowHurdleBatch>(6, 0x8000);

    CreateActor<RPCowTreeBatch>(10, 0x8000);

    CreateActors<RPCowPointPopup>(32, 17, 0x5000);

    CreateActors<RPCowPlayer>(2, 7, 0x6000);

    CreateActor<RPCowMainHud>(18, 0x9000);

    CreateActor<RPCowPerfectMsg>(19, 0x9000);

    CreateActor<RPCowExMsg>(20, 0x9000);

    CreateActor<RPCowOrderMsg>(21, 0x9000);

    CreateActors<RPCowTargetMarker>(2, 22, 0x9000);

    CreateActors<RPCowBalloon>(2, 23, 0x9000);

    CreateActor<RPCowStartMsg>(24, 0x9000);

    CreateActor<RPCowGoalMsg>(25, 0x9000);

    CreateActor<RPCowLimitCount>(26, 0x9000);

    CreateActor<RPCowVsResult>(27, 0x9000);

    CreateActor<RPCowAngleDisp>(28, 0x9000);

    RPCommonParam param;
    param.mGame = 7;
    if (RPSceneInfo::spInstance != NULL) {
        param.mPlayerNum = RPSceneInfo::spInstance->GetPlayerNum();
    } else {
        param.mPlayerNum = 1;
    }
    RPCommon::CreateInstance(param);
    RPGrpRenderer::GetCurrent()->LinkViews();

    RPSndAudioMgr::instance()->configureMiniGame(GetSoundPlayerNum(this));
    RPSndAudioMgr::instance()->setPlayerNum(GetSoundPlayerNum(this));
    RPSndAudioMgr::instance()->stopStream(10);
    Cow_stopBgm();

    pSceneSound = CowNewPlaced<RPCowSceneSound>(1);
    mpSceneSound = pSceneSound;
    mpSceneSound->initSound3D(0);

    mpLytAccessor = RPSysLytResAccessor::create(NULL);
    RP_Cls803572E0_createInstance();
}

RPCowCamera::~RPCowCamera() {}

RPCowPosHistory::~RPCowPosHistory() {}

RPCowPlayer::~RPCowPlayer() {}

RPCowKakasiBatch::~RPCowKakasiBatch() {}

RPCowHurdleBatch::~RPCowHurdleBatch() {}

RPCowTreeBatch::~RPCowTreeBatch() {}

RPCowBalloon::~RPCowBalloon() {}

RPCowSceneSound::~RPCowSceneSound() {}

// Every table slot is deleted the same way: VF_0x40 on each actor (last first), then the
// array (or the single actor) is deleted.
template <typename T>
inline void CowDeleteSlot(RPActor** ppActors, s32* pNums, int slot) {
    T* pActors = static_cast<T*>(ppActors[slot]);
    s32 num = pNums[slot];
    for (int i = num - 1; i >= 0; i--) {
        pActors[i].VF_0x40();
    }
    if (num > 1) {
        delete[] pActors;
    } else {
        delete pActors;
    }
}

RPCowGame::~RPCowGame() {
    CowDeleteSlot<RPCowCamera>(mpTableA, mpTableNum, 0);
    CowDeleteSlot<RPCowField>(mpTableA, mpTableNum, 1);
    CowDeleteSlot<RPCowPlayer>(mpTableA, mpTableNum, 7);
    CowDeleteSlot<RPCowKakasiGroup>(mpTableA, mpTableNum, 8);
    CowDeleteSlot<RPCowModelBatch>(mpTableA, mpTableNum, 16);
    CowDeleteSlot<RPCowKakasi>(mpTableA, mpTableNum, 2);
    CowDeleteSlot<RPCowKakasiBatch>(mpTableA, mpTableNum, 3);
    CowDeleteSlot<RPCowKakasiKing>(mpTableA, mpTableNum, 4);
    CowDeleteSlot<RPCowHurdle>(mpTableA, mpTableNum, 5);
    CowDeleteSlot<RPCowHurdleBatch>(mpTableA, mpTableNum, 6);
    CowDeleteSlot<RPCowTree>(mpTableA, mpTableNum, 9);
    CowDeleteSlot<RPCowTreeBatch>(mpTableA, mpTableNum, 10);
    CowDeleteSlot<RPCowKnitCowEx>(mpTableA, mpTableNum, 11);
    CowDeleteSlot<RPCowKnitCowEx>(mpTableA, mpTableNum, 12);
    CowDeleteSlot<RPCowCloud>(mpTableA, mpTableNum, 13);
    CowDeleteSlot<RPCowCloud>(mpTableA, mpTableNum, 14);
    CowDeleteSlot<RPCowGoal>(mpTableA, mpTableNum, 15);
    CowDeleteSlot<RPCowPointPopup>(mpTableA, mpTableNum, 17);
    CowDeleteSlot<RPCowMainHud>(mpTableA, mpTableNum, 18);
    CowDeleteSlot<RPCowPerfectMsg>(mpTableA, mpTableNum, 19);
    CowDeleteSlot<RPCowExMsg>(mpTableA, mpTableNum, 20);
    CowDeleteSlot<RPCowOrderMsg>(mpTableA, mpTableNum, 21);
    CowDeleteSlot<RPCowTargetMarker>(mpTableA, mpTableNum, 22);
    CowDeleteSlot<RPCowBalloon>(mpTableA, mpTableNum, 23);
    CowDeleteSlot<RPCowStartMsg>(mpTableA, mpTableNum, 24);
    CowDeleteSlot<RPCowGoalMsg>(mpTableA, mpTableNum, 25);
    CowDeleteSlot<RPCowLimitCount>(mpTableA, mpTableNum, 26);
    CowDeleteSlot<RPCowVsResult>(mpTableA, mpTableNum, 27);
    CowDeleteSlot<RPCowAngleDisp>(mpTableA, mpTableNum, 28);

    RPCommon::DestroyInstance();
    Cow_stopBgm();
    RPSndAudioMgr::instance()->stopStream(10);
    RPSndAudioMgr::instance()->changeScene();
    delete mpSceneSound;
    RPCls803572E0::DestroyInstance();
}
// ---- RPCowGame::LoadResource (Cow_Game_load, 0x80297C24) ----

// Kept as a macro: the seven courses repeat this sequence inline, with each name's address
// formed where it is used. An inline member taking the names (tried: RPCowGame::LoadField) has
// them computed up front into saved registers (r24-r27), 174 words differ in LoadResource.
#define LOAD_FIELD(pmp, kcl, brres, mdl)                                                       \
    {                                                                                          \
        mpResTable = new RPResTable(pmp);                                                      \
        mpCollisionModel = new RPCollisionModel;                                               \
        mpCollisionModel->Reset(                                                               \
            RPSysResourceManager::GetFileFromArchive(pArchive, kcl));                          \
        pResMgr->CreateDataAt(1, RPGrpModelResManager::Type_ResFile,                           \
                              RPSysResourceManager::GetFileFromArchive(pArchive, brres));      \
        pResMgr->CreateDataAt(2, RPGrpModelResManager::Type_ResMdl,                            \
                              pResMgr->GetPtr(RPGrpModelResManager::Type_ResMdl, 1, mdl));     \
        pResMgr->CreateDataAt(3, RPGrpModelResManager::Type_ResMdl,                            \
                              pResMgr->GetPtr(RPGrpModelResManager::Type_ResMdl, 1, "cow_sky")); \
        static_cast<RPGrpLightManager*>(RPGrpModelManager::GetCurrent()->GetLightManager(0))   \
            ->LoadScnLight(1, "cow_light", 0.0f, -1);                                          \
    }

// Creates a model resource (or animation) from a file that was created at another handle
inline void CreateResAt(RPGrpModelResManager* pResMgr, RPGrpHandle handle,
                        RPGrpModelResManager::Type type, RPGrpHandle file, const char* pName) {
    pResMgr->CreateDataAt(handle, type, pResMgr->GetPtr(type, file, pName));
}

// String-pool labels differ; the instructions and offsets match.
void RPCowGame::LoadResource() {
    EGG::Archive* pArchive = RPSysResourceManager::LoadGameCommonArchive();
    RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();

    if (mCourse <= 1) {
        LOAD_FIELD("cow_field1P.pmp", "cow_field1P.kcl", "G3D/cow_field.brres", "cow_field1P")
    } else if (mCourse <= 3 - 1) {
        LOAD_FIELD("cow_field2P.pmp", "cow_field2P.kcl", "G3D/cow_field.brres", "cow_field2P")
    } else if (mCourse <= 4 - 1) {
        LOAD_FIELD("cow_field01.pmp", "cow_field01.kcl", "G3D/cow_field01.brres", "cow_field01")
    } else if (mCourse <= 5 - 1) {
        LOAD_FIELD("cow_field02.pmp", "cow_field01.kcl", "G3D/cow_field01.brres", "cow_field01")
    } else if (mCourse <= 6 - 1) {
        LOAD_FIELD("cow_field03.pmp", "cow_field03.kcl", "G3D/cow_field03.brres", "cow_field03")
    } else if (mCourse <= 7 - 1) {
        LOAD_FIELD("cow_field04.pmp", "cow_field03.kcl", "G3D/cow_field03.brres", "cow_field03")
    } else {
        LOAD_FIELD("cow_knit_field.pmp", "cow_knit_field.kcl", "G3D/cow_knit_field.brres",
                   "cow_knit_field")
    }

    CreatePlaced<RPCowKakasiGroup>(mpResTable, 0x70005, 8, 0x4000);
    CreatePlaced<RPCowHurdle>(mpResTable, 0x70000, 5, 0x4000);
    CreatePlaced<RPCowTree>(mpResTable, 0x70004, 9, 0x7000);
    CreatePlaced<RPCowGoal>(mpResTable, 0x70002, 15, 0x7000);
    CreatePlaced<RPCowKnitCowEx>(mpResTable, 0x70007, 11, 0x4000);
    CreatePlaced<RPCowKnitCowEx>(mpResTable, 0x70008, 12, 0x4000);
    CreatePlaced<RPCowCloud>(mpResTable, 0x70009, 13, 0x4000);
    CreatePlaced<RPCowCloud>(mpResTable, 0x7000A, 14, 0x4000);

    pResMgr->CreateDataAt(5, RPGrpModelResManager::Type_ResFile,
                          RPSysResourceManager::GetFileFromArchive(pArchive, "G3D/cow_kakasi.brres"));
    CreateResAt(pResMgr, 6, RPGrpModelResManager::Type_ResMdl, 5, "cow_kakasi");
    CreateResAt(pResMgr, 7, RPGrpModelResManager::Type_ResMdl, 5, "cow_kakasiL1");
    CreateResAt(pResMgr, 8, RPGrpModelResManager::Type_ResMdl, 5, "cow_kakasiL1_many");
    CreateResAt(pResMgr, 9, RPGrpModelResManager::Type_ResMdl, 5, "cow_kakasiL2");
    CreateResAt(pResMgr, 10, RPGrpModelResManager::Type_ResMdl, 5, "cow_kakasiL2_many");
    CreateResAt(pResMgr, 11, RPGrpModelResManager::Type_ResMdl, 5, "cow_kakasi_king");
    pResMgr->CreateDataAt(13, RPGrpModelResManager::Type_ResFile,
                          RPSysResourceManager::GetFileFromArchive(pArchive, "G3D/cow_hurdle.brres"));
    CreateResAt(pResMgr, 14, RPGrpModelResManager::Type_ResMdl, 13, "cow_hurdle");
    CreateResAt(pResMgr, 15, RPGrpModelResManager::Type_ResMdl, 13, "cow_hurdleL1");
    CreateResAt(pResMgr, 16, RPGrpModelResManager::Type_ResMdl, 13, "cow_hurdleL1_many");
    pResMgr->CreateDataAt(18, RPGrpModelResManager::Type_ResFile,
                          RPSysResourceManager::GetFileFromArchive(pArchive, "G3D/cow_tree.brres"));
    CreateResAt(pResMgr, 19, RPGrpModelResManager::Type_ResMdl, 18, "cow_tree");
    CreateResAt(pResMgr, 20, RPGrpModelResManager::Type_ResMdl, 18, "cow_treeL1");
    CreateResAt(pResMgr, 22, RPGrpModelResManager::Type_ResMdl, 18, "cow_treeL1_many");
    CreateResAt(pResMgr, 21, RPGrpModelResManager::Type_ResMdl, 18, "cow_treeL2");
    CreateResAt(pResMgr, 23, RPGrpModelResManager::Type_ResMdl, 18, "cow_treeL2_many");
    pResMgr->CreateDataAt(25, RPGrpModelResManager::Type_ResFile,
                          RPSysResourceManager::GetFileFromArchive(pArchive, "G3D/cow_maru_shadow.brres"));
    CreateResAt(pResMgr, 26, RPGrpModelResManager::Type_ResMdl, 25, "cow_maru_shadow");
    CreateResAt(pResMgr, 27, RPGrpModelResManager::Type_ResMdl, 25, "cow_maru_shadow_128");
    pResMgr->CreateDataAt(29, RPGrpModelResManager::Type_ResFile,
                          RPSysResourceManager::GetFileFromArchive(pArchive, "G3D/cow_hurdle_shadow.brres"));
    CreateResAt(pResMgr, 30, RPGrpModelResManager::Type_ResMdl, 29, "cow_hurdle_shadow");
    CreateResAt(pResMgr, 31, RPGrpModelResManager::Type_ResMdl, 29, "cow_hurdle_shadow_64");
    pResMgr->CreateDataAt(33, RPGrpModelResManager::Type_ResFile,
                          RPSysResourceManager::GetFileFromArchive(pArchive, "G3D/cow_knit.brres"));
    CreateResAt(pResMgr, 34, RPGrpModelResManager::Type_ResMdl, 33, "cow_knit_1P");
    CreateResAt(pResMgr, 35, RPGrpModelResManager::Type_ResMdl, 33, "cow_knit_2P");
    CreateResAt(pResMgr, 40, RPGrpModelResManager::Type_ResAnmChr, 33, "cow_knit_run_L");
    CreateResAt(pResMgr, 41, RPGrpModelResManager::Type_ResAnmChr, 33, "cow_knit_run_R");
    CreateResAt(pResMgr, 42, RPGrpModelResManager::Type_ResAnmChr, 33, "cow_knit_runF_L");
    CreateResAt(pResMgr, 43, RPGrpModelResManager::Type_ResAnmChr, 33, "cow_knit_runF_R");
    CreateResAt(pResMgr, 36, RPGrpModelResManager::Type_ResAnmChr, 33, "cow_knit_wait_L");
    CreateResAt(pResMgr, 37, RPGrpModelResManager::Type_ResAnmChr, 33, "cow_knit_wait_R");
    CreateResAt(pResMgr, 38, RPGrpModelResManager::Type_ResAnmChr, 33, "cow_knit_waitF_L");
    CreateResAt(pResMgr, 39, RPGrpModelResManager::Type_ResAnmChr, 33, "cow_knit_waitF_R");
    CreateResAt(pResMgr, 44, RPGrpModelResManager::Type_ResMdl, 33, "cow_knit_exA");
    CreateResAt(pResMgr, 45, RPGrpModelResManager::Type_ResMdl, 33, "cow_knit_exB");
    CreateResAt(pResMgr, 46, RPGrpModelResManager::Type_ResAnmChr, 33, "cow_knit_ex_wait");
    CreateResAt(pResMgr, 47, RPGrpModelResManager::Type_ResAnmChr, 33, "cow_knit_ex_eat");
    pResMgr->CreateDataAt(48, RPGrpModelResManager::Type_ResFile,
                          RPSysResourceManager::GetFileFromArchive(pArchive, "G3D/cow_avatar_anim.brres"));
    CreateResAt(pResMgr, 49, RPGrpModelResManager::Type_ResAnmChr, 48, "cow_run_L");
    CreateResAt(pResMgr, 50, RPGrpModelResManager::Type_ResAnmChr, 48, "cow_run_R");
    CreateResAt(pResMgr, 52, RPGrpModelResManager::Type_ResAnmChr, 48, "cow_runF_L");
    CreateResAt(pResMgr, 51, RPGrpModelResManager::Type_ResAnmChr, 48, "cow_runF_R");
    CreateResAt(pResMgr, 54, RPGrpModelResManager::Type_ResAnmChr, 48, "cow_runB_L");
    CreateResAt(pResMgr, 53, RPGrpModelResManager::Type_ResAnmChr, 48, "cow_runB_R");
    CreateResAt(pResMgr, 55, RPGrpModelResManager::Type_ResAnmChr, 48, "cow_waitR");
    CreateResAt(pResMgr, 56, RPGrpModelResManager::Type_ResAnmChr, 48, "cow_jumpR");
    CreateResAt(pResMgr, 57, RPGrpModelResManager::Type_ResAnmChr, 48, "cow_chakuchiR");
    CreateResAt(pResMgr, 58, RPGrpModelResManager::Type_ResAnmChr, 48, "cow_goalR");
    pResMgr->CreateDataAt(60, RPGrpModelResManager::Type_ResFile,
                          RPSysResourceManager::GetFileFromArchive(pArchive, "G3D/cow_gate.brres"));
    CreateResAt(pResMgr, 61, RPGrpModelResManager::Type_ResMdl, 60, "cow_gate");
    pResMgr->CreateDataAt(63, RPGrpModelResManager::Type_ResFile,
                          RPSysResourceManager::GetFileFromArchive(pArchive, "G3D/cow_cloud.brres"));
    CreateResAt(pResMgr, 64, RPGrpModelResManager::Type_ResMdl, 63, "cow_cloudA");
    CreateResAt(pResMgr, 65, RPGrpModelResManager::Type_ResMdl, 63, "cow_cloudB");
    pResMgr->CreateDataAt(66, RPGrpModelResManager::Type_ResFile,
                          RPSysResourceManager::GetFileFromArchive(pArchive, "G3D/cow_arrow.brres"));
    CreateResAt(pResMgr, 67, RPGrpModelResManager::Type_ResMdl, 66, "cow_arrow1P");
    CreateResAt(pResMgr, 68, RPGrpModelResManager::Type_ResMdl, 66, "cow_arrow2P");
    CreateResAt(pResMgr, 69, RPGrpModelResManager::Type_ResAnmChr, 66, "cow_arrow");
    CreateResAt(pResMgr, 70, RPGrpModelResManager::Type_ResAnmClr, 66, "cow_arrow1P");
    CreateResAt(pResMgr, 71, RPGrpModelResManager::Type_ResAnmClr, 66, "cow_arrow2P");

    RPCommon::GetInstance()->LoadResources();
    RPSndAudioMgr::instance()->attachArchive("rp_Cow_sound.brsar", false, NULL);
    RPSndAudioMgr::instance()->loadGroup(0U);

    void* pEffect = RPSysResourceManager::GetFileFromArchive(pArchive, "basic_cow.breff");
    void* pTexture = RPSysResourceManager::GetFileFromArchive(pArchive, "basic_cow.breft");
    RPSysSceneMgr::instance()->getCurrentSceneRP()->getEffectCreator()->setEffectResource(
        pEffect, pTexture);

    mpLytAccessor->mountArchive(RPSysResourceManager::GetFileFromArchive(
        RPSysResourceManager::LoadGameLocalArchive(), "cow.arc"));
    RPCls803572E0::GetInstance()->LoadMessage("party_message.bmg");
}

#undef LOAD_RES
#undef LOAD_FIELD
#undef LOAD_PLACED_ACTORS
// ---- RPCowGame::Reset ----

namespace {

// Actor idx of a table slot (the original indexes with a u16) [M]
template <typename T>
inline T* CowGetActor(const RPCowGame* pGame, int slot, u16 idx) {
    return &static_cast<T*>(pGame->GetSlotActor(slot))[idx];
}

// Starts the first num actors of a slot (0: all of them) [M]
template <typename T>
inline void CowStartActors(RPCowGame* pGame, int slot, u16 num) {
    int i;
    T* p = static_cast<T*>(pGame->GetSlotActor(slot));
    u16 n = num != 0 ? num : static_cast<u16>(pGame->GetSlotNum(slot));
    for (i = 0; i < n; i++) {
        pGame->StartActor(p);
        p++;
    }
}

// FAKEMATCH candidate: same as CowStartActors with the pointer declared first (the camera
// slot's registers only come out right this way)
template <typename T>
inline void CowStartActorsP(RPCowGame* pGame, int slot, u16 num) {
    T* p = static_cast<T*>(pGame->GetSlotActor(slot));
    u16 n = num != 0 ? num : static_cast<u16>(pGame->GetSlotNum(slot));
    int i;
    for (i = 0; i < n; i++) {
        pGame->StartActor(p);
        p++;
    }
}

// Starts every actor of a slot [M]
template <typename T>
inline void CowStartActors(RPCowGame* pGame, int slot) {
    u16 n = static_cast<u16>(pGame->GetSlotNum(slot));
    int i;
    T* p = static_cast<T*>(pGame->GetSlotActor(slot));
    for (i = 0; i < n; i++) {
        pGame->StartActor(p);
        p++;
    }
}

} // namespace

void RPCowGame::Reset() {
    mTimeBonus = 0;
    unk34 = 0;
    unk38 = 0;
    mTime = 0.0f;
    mTimer = 0;
    mCountdown = 0;
    unk28 = 150;
    mSpawnedGroups = 0;
    mKnockedGroups = 0;
    mGroupChain = 0;
    unk44 = -1;
    unk48 = -1;
    mPlayerNum = static_cast<u16>(mPlayerNum < 2 ? mPlayerNum : 2);

    sColliderListA = RPColliderList();
    Cow_objectColliders = RPColliderList();
    sColliderListB = RPColliderList();

    CowStartActorsP<RPCowCamera>(this, 0, mPlayerNum);
    CowStartActors<RPCowField>(this, 1);
    CowStartActors<RPCowPlayer>(this, 7, mPlayerNum);
    CowStartActors<RPCowKakasiGroup>(this, 8, mpResTable->CountEntriesOfType(0x70005));
    CowStartActors<RPCowModelBatch>(this, 16);
    CowStartActors<RPCowKakasiBatch>(this, 3);
    CowStartActors<RPCowHurdle>(this, 5, mpResTable->CountEntriesOfType(0x70000));
    CowStartActors<RPCowHurdleBatch>(this, 6);
    CowStartActors<RPCowTree>(this, 9, mpResTable->CountEntriesOfType(0x70004));
    CowStartActors<RPCowTreeBatch>(this, 10);
    CowStartActors<RPCowKnitCowEx>(this, 11, mpResTable->CountEntriesOfType(0x70007));
    CowStartActors<RPCowKnitCowEx>(this, 12, mpResTable->CountEntriesOfType(0x70008));
    CowStartActors<RPCowCloud>(this, 13, mpResTable->CountEntriesOfType(0x70009));
    CowStartActors<RPCowCloud>(this, 14, mpResTable->CountEntriesOfType(0x7000A));
    CowStartActors<RPCowGoal>(this, 15, mpResTable->CountEntriesOfType(0x70002));
    CowStartActors<RPCowMainHud>(this, 18);
    CowStartActors<RPCowPerfectMsg>(this, 19);
    CowStartActors<RPCowExMsg>(this, 20);
    CowStartActors<RPCowOrderMsg>(this, 21);
    CowStartActors<RPCowTargetMarker>(this, 22, mPlayerNum);
    CowStartActors<RPCowBalloon>(this, 23, mPlayerNum);
    CowStartActors<RPCowStartMsg>(this, 24);
    CowStartActors<RPCowGoalMsg>(this, 25);
    CowStartActors<RPCowLimitCount>(this, 26);
    CowStartActors<RPCowVsResult>(this, 27);
    CowStartActors<RPCowAngleDisp>(this, 28);

    RPSceneInfo* pInfo = RPSceneInfo::spInstance;
    if (pInfo != NULL) {
        mPlayerNum = pInfo->GetPlayerNum() < 2 ? pInfo->GetPlayerNum() : 2;
        if (pInfo->GetUnk1C() == 0) {
            pInfo->SetUnk1C(static_cast<u16>(mCourse));
            mPlayIntro = true;
        } else {
            mPlayIntro = false;
        }
    } else {
        mPlayIntro = true;
    }

    if (mPlayIntro) {
        for (int i = 0; i < mPlayerNum; i++) {
            CowGetActor<RPCowCamera>(this, 0, i)->StartIntro();
            CowGetActor<RPCowPlayer>(this, 7, i)->mpCowModel->SetIntroState();
        }
        mState = 0;
        Cow_playBgm(14);
        RPSndAudioMgr::instance()->startStream(0x96);
    } else {
        for (int i = 0; i < mPlayerNum; i++) {
            CowGetActor<RPCowCamera>(this, 0, i)->SetupReady();
            CowGetActor<RPCowPlayer>(this, 7, i)->mpCowModel->SetRunState();
        }
        mState = 1;
        Cow_playBgm(15);
        RPSndAudioMgr::instance()->stopStream(10);
    }

    RPCommon::GetInstance()->Reset();
    if (mPlayerNum <= 1) {
        RPCommon::GetInstance()->GetPauseText()->SetEnabled(0, true);
    }
}
// ---- RPCowGame::Calculate ----
// (uses CowGetActor from reset.inc)

// RPActorScene::sFlags (private; ledger name)

void RPCowGame::Calculate() {
    RPCowLimitCount* pLimit;
    switch (mState) {
    case 0: {
        bool done = true;
        for (int i = 0; i < mPlayerNum; i++) {
            if (!CowGetActor<RPCowCamera>(this, 0, i)->IsIntroDone()) {
                done = false;
            }
        }
        if (done) {
            for (int i = 0; i < mPlayerNum; i++) {
                CowGetActor<RPCowCamera>(this, 0, i)->SetupReady();
                CowGetActor<RPCowPlayer>(this, 7, i)->mpCowModel->SetRunState();
            }
            mTimer = 0;
            mState = 1;
            Cow_playBgm(15);
        }
        break;
    }
    case 1: {
        if (--unk28 <= 0) {
            unk28 = 0;
        }
        bool done = true;
        for (int i = 0; i < mPlayerNum; i++) {
            if (CowGetActor<RPCowPlayer>(this, 7, i)->unk72E0 > 0) {
                done = false;
            }
        }
        if (done) {
            for (int i = 0; i < mPlayerNum; i++) {
                CowGetActor<RPCowCamera>(this, 0, i)->StartStart();
            }
            mState = 2;
        }
        break;
    }
    case 2: {
        bool done = true;
        for (int i = 0; i < mPlayerNum; i++) {
            if (!CowGetActor<RPCowCamera>(this, 0, i)->IsPullbackDone()) {
                done = false;
            }
        }
        if (done) {
            static_cast<RPCowOrderMsg*>(GetSlotActor(21))->Show();
            mTimer = 60;
            mState = 4;
        }
        break;
    }
    case 4:
        if (--mTimer <= 0) {
            static_cast<RPCowOrderMsg*>(GetSlotActor(21))->ShowAllTargetMarkers();
            RPCowKakasiGroup::SpawnAll();
            mTimer = 120;
            mState = 5;
        }
        break;
    case 5:
        if (--unk28 <= 0) {
            unk28 = 0;
        }
        if (--mTimer <= 0) {
            RPCommon::GetInstance()->Reset();
            if (mPlayerNum <= 1) {
                RPCommon::GetInstance()->GetPauseText()->SetEnabled(0, false);
            }
            int i;
            RPCowMainHud* pHud = static_cast<RPCowMainHud*>(GetSlotActor(18));
            pHud->SetTime(mTime + CowGetActor<RPCowPlayer>(this, 7, 0)->mTime / 60.0f);
            for (i = 0; i < mPlayerNum; i++) {
                pHud->SetPlayerScore(i, 0, 0);
            }
            pHud->Show();
            static_cast<RPCowAngleDisp*>(GetSlotActor(28))->BindAnimsMode1();
            static_cast<RPCowStartMsg*>(GetSlotActor(24))->Show();
            mTimer = 240;
            mState = 6;
        }
        break;
    case 6: {
        s32 count = mTimer / 60 + 1;
        if (mCountdown != count) {
            mCountdown = count;
            if (count >= 1 && count <= 3) {
                RPSndAudioMgr::instance()->startSystemSe(0x75);
            }
        }
        if (--mTimer <= 0) {
            mTimer = 60;
            mCountdown = 0;
            mState = 7;
            Cow_stopBgm();
            RPSndAudioMgr::instance()->startSystemSe(0x76);
            RPSndAudioMgr::instance()->startStream(0x95);
        }
        break;
    }
    case 7:
        if (--mTimer <= 0) {
            static_cast<RPCowOrderMsg*>(GetSlotActor(21))->HideAllBalloons();
            mTimer = 0;
            mState = 8;
        }
    case 8: {
        mTime = RPUtlRandom::getF32() * (1.0f / 60.0f);
        for (int i = 0; i < mPlayerNum; i++) {
            RPCowPlayer* pPlayer = CowGetActor<RPCowPlayer>(this, 7, i);
            if ((pPlayer->mTime -= 1.0f) <= 0.0f) {
                mTime = 0.0f;
            }
        }
        static_cast<RPCowMainHud*>(GetSlotActor(18))
            ->SetTime(mTime + CowGetActor<RPCowPlayer>(this, 7, 0)->mTime / 60.0f);
        s32 count = CowGetActor<RPCowPlayer>(this, 7, 0)->mTime / 60.0f + 1.0f;
        if (mCountdown != count) {
            mCountdown = count;
            if (count == 11) {
                pLimit = static_cast<RPCowLimitCount*>(GetSlotActor(26));
                pLimit->Start();
                pLimit->SetCount(mCountdown);
            } else if (count >= 6 && count <= 10) {
                static_cast<RPCowLimitCount*>(GetSlotActor(26))->SetCount(count);
                static_cast<RPCowMainHud*>(GetSlotActor(18))->SetTimeWarning(true);
                RPSndAudioMgr::instance()->startSystemSe(0x73);
            } else if (count >= 1 && count <= 5) {
                static_cast<RPCowLimitCount*>(GetSlotActor(26))->SetCount(count);
                RPSndAudioMgr::instance()->startSystemSe(0x74);
            }
        }
        break;
    }
    case 9:
        if (mTimer > 0 && --mTimer <= 0) {
            static_cast<RPCowMainHud*>(GetSlotActor(18))->PlayOut1P();
            static_cast<RPCowAngleDisp*>(GetSlotActor(28))->BindAnimsMode2();
            mTimer = 30;
            mState = 10;
            for (int i = 0; i < mPlayerNum; i++) {
                CowGetActor<RPCowCamera>(this, 0, i)->StartResult();
            }
        }
        break;
    case 10:
        if (mTimer > 0 && --mTimer <= 0) {
            static_cast<RPCowMainHud*>(GetSlotActor(18))->PlayInResult();
            mTimer = 90;
            mState = 11;
        }
        break;
    case 11:
        if (mTimer > 0 && --mTimer <= 0) {
            static_cast<RPCowMainHud*>(GetSlotActor(18))->PlayTimeBonus();
            RPSndAudioMgr::instance()->startSystemSe(0x55);
            mState = 12;
        }
        break;
    case 12:
        if (static_cast<RPCowMainHud*>(GetSlotActor(18))->IsAnimIdle()) {
            RPCowPlayer* pPlayer = CowGetActor<RPCowPlayer>(this, 7, 0);
            mTimeBonus = pPlayer->mTime / 60.0f;
            pPlayer->AddBonusScore(mTimeBonus);
            mState = 13;
        }
        break;
    case 13: {
        bool done = true;
        for (int i = 0; i < mPlayerNum; i++) {
            if (CowGetActor<RPCowPlayer>(this, 7, i)->mBonusScore > 0) {
                done = false;
            }
        }
        if (done) {
            static_cast<RPCowMainHud*>(GetSlotActor(18))->PlayTotal();
            mTimer = 90;
            mState = 14;
            RPSndAudioMgr::instance()->startSystemSe(0x57);
        } else {
            Cow_holdGlobalSe(0x56);
        }
        break;
    }
    case 14:
        if (mTimer > 0 && --mTimer <= 0) {
            static_cast<RPCowMainHud*>(GetSlotActor(18))->PlayOutResult();
            RPCommon::GetInstance()->GetMedalResult()->Start(
                CowGetActor<RPCowPlayer>(this, 7, 0)->GetTotalScore());
            mState = 22;
        }
        break;
    case 15:
        if (mTimer > 0 && --mTimer <= 0) {
            static_cast<RPCowMainHud*>(GetSlotActor(18))->PlayOut();
            static_cast<RPCowAngleDisp*>(GetSlotActor(28))->BindAnimsMode2();
            mTimer = 30;
            mState = 16;
            for (int i = 0; i < mPlayerNum; i++) {
                CowGetActor<RPCowCamera>(this, 0, i)->StartResult();
            }
        }
        break;
    case 16:
        if (mTimer > 0 && --mTimer <= 0) {
            static_cast<RPCowMainHud*>(GetSlotActor(18))->PlayInResult();
            mTimer = 90;
            mState = 17;
        }
        break;
    case 17:
        if (mTimer > 0 && --mTimer <= 0) {
            CowGetActor<RPCowPlayer>(this, 7, 0)->mScore = 0;
            static_cast<RPCowMainHud*>(GetSlotActor(18))->ShowTimeout();
            mTimer = 90;
            mState = 18;
        }
        break;
    case 18:
        if (mTimer > 0 && --mTimer <= 0) {
            static_cast<RPCowMainHud*>(GetSlotActor(18))->PlayOutResult();
            RPCommon::GetInstance()->GetMedalResult()->Start(
                CowGetActor<RPCowPlayer>(this, 7, 0)->GetTotalScore());
            mState = 22;
        }
        break;
    case 19:
        if (mTimer > 0 && --mTimer <= 0) {
            static_cast<RPCowMainHud*>(GetSlotActor(18))->PlayOut();
            static_cast<RPCowAngleDisp*>(GetSlotActor(28))->BindAnimsMode2();
            mTimer = 20;
            mState = 20;
            for (int i = 0; i < mPlayerNum; i++) {
                CowGetActor<RPCowCamera>(this, 0, i)->StartResult();
            }
        }
        break;
    case 20:
        if (mTimer > 0 && --mTimer <= 0) {
            int i;
            RPCowVsResult* pResult = static_cast<RPCowVsResult*>(GetSlotActor(27));
            for (i = 0; i < mPlayerNum; i++) {
                pResult->SetScore(i, CowGetActor<RPCowPlayer>(this, 7, i)->GetTotalScore());
            }
            pResult->Show();
            mTimer = 180;
            mState = 21;
        }
        break;
    case 21:
        if (mTimer > 0 && --mTimer <= 0) {
            int i;
            RPCowVsResult* pResult = static_cast<RPCowVsResult*>(GetSlotActor(27));
            for (i = 0; i < mPlayerNum; i++) {
                if (i == unk48) {
                    pResult->ShowWinner(i);
                }
            }
            RPCommon::GetInstance()->GetVsWinner()->Start(unk48);
            mTimer = 0;
            mState = 23;
        }
        break;
    case 22:
        if (RPCommon::GetInstance()->GetMedalResult()->GetState() == 4) {
            RPCommon::GetInstance()->GetResultMenu()->Start(1, -1);
            mState = 24;
        }
        break;
    case 23:
        if (RPCommon::GetInstance()->GetVsWinner()->GetState() == 5 ||
            RPCommon::GetInstance()->GetVsWinner()->IsStateEntered(3)) {
            static_cast<RPCowVsResult*>(GetSlotActor(27))->PlayOut();
        }
        if (RPCommon::GetInstance()->GetVsWinner()->GetState() == 5) {
            RPCommon::GetInstance()->GetResultMenu()->Start(2, unk48);
            mState = 24;
        }
        break;
    case 24:
        if (RPCommon::GetInstance()->GetResultMenu()->GetState() == 4) {
            if (RPCommon::GetInstance()->GetResultMenu()->GetChoice() == 0) {
                RPSceneInfo::spInstance->unk20 = 0;
                RPSceneInfo::spInstance->unk24 = 0;
                RPSysSceneCreator::instance()->changeSceneAfterFade(0x15);
            } else {
                RPSceneInfo::spInstance->unk20 = 0;
                RPSceneInfo::spInstance->unk24 = 0;
                RPSceneInfo::spInstance->SetUnk1C(0);
                RPSysSceneCreator::instance()->changeSceneAfterFade(0x1B);
            }
            RPSndAudioMgr::instance()->stopStream(10);
        }
        break;
    }

    if (unk34 > 0) {
        unk34--;
        if (++unk38 >= 30) {
            unk38 = 0;
        }
    }
    for (int i = 0; i < mPlayerNum; i++) {
        CowGetActor<RPCowPlayer>(this, 7, i)->CalcInput();
    }
    for (int i = 0; i < mPlayerNum; i++) {
        CowGetActor<RPCowPlayer>(this, 7, i)->CalcMovement();
    }
    for (int i = 0; i < mPlayerNum; i++) {
        CowGetActor<RPCowPlayer>(this, 7, i)->HandleCollisions();
    }
    for (int i = 0; i < mPlayerNum; i++) {
        CowGetActor<RPCowPlayer>(this, 7, i)->CalcProgress();
    }
    if (RP_actorSceneFlags & 4) {
        RPCommon::GetInstance()->mEntryAvatars = false;
        RPCommon::GetInstance()->Calculate();
    } else {
        RPCommon::GetInstance()->mEntryAvatars = true;
        RPCommon::GetInstance()->Calculate();
    }
}

// Single HUD actors keep their pointer in the original saved register through this helper.
template <typename T>
inline void CowDrawOne(RPActor** ppActors, int slot, u16 index) {
    T* const pActor = CowGetActor<T>(ppActors, slot, index);
    pActor->DrawScreen(Lyt_getCurrentViewNo());
}

void RPCowGame::Draw() {
    if (Lyt_getCurrentViewNo() == 0 || Lyt_getCurrentViewNo() == 1) {
        if (Lyt_getDrawStep() == 2) {
            for (int i = 0; i < mPlayerNum; i++) {
                CowGetActor<RPCowCamera>(mpTableA, 0, i)->SetModelVisible(Lyt_getCurrentViewNo() == i);
                CowGetActor<RPCowPlayer>(mpTableA, 7, i)->SetArrowVisible(Lyt_getCurrentViewNo() == i);
            }
            s32 num = GetSlotNum(2);
            for (int i = 0; i < num; i++) {
                CowGetActor<RPCowKakasi>(mpTableA, 2, i)->SetVisibleForScreen();
            }
            num = GetSlotNum(5);
            for (int i = 0; i < num; i++) {
                CowGetActor<RPCowHurdle>(mpTableA, 5, i)->SetVisibleForScreen();
            }
            num = GetSlotNum(9);
            for (int i = 0; i < num; i++) {
                CowGetActor<RPCowTree>(mpTableA, 9, i)->SetVisibleForScreen();
            }
        }
        if (Lyt_getDrawStep() == 9) {
            s32 num = GetSlotNum(15);
            for (int i = 0; i < num; i++) {
                CowGetActor<RPCowGoal>(mpTableA, 15, i)->DrawDebugBox();
            }
        }
        if (Lyt_getDrawStep() == 6) {
            for (int i = 0; i < mPlayerNum; i++) {
                CowGetActor<RPCowPlayer>(mpTableA, 7, i)->DrawShadowTex();
            }
        }
    }
    if ((Lyt_getCurrentViewNo() == 4 || Lyt_getCurrentViewNo() == 2 ||
         Lyt_getCurrentViewNo() == 3) &&
        Lyt_getDrawStep() == 3) {
        for (int i = 0; i < mPlayerNum; i++) {
            CowGetActor<RPCowPlayer>(mpTableA, 7, i)->DrawScreen(Lyt_getCurrentViewNo());
        }
        CowDrawOne<RPCowMainHud>(mpTableA, 18, 0);
        CowDrawOne<RPCowPerfectMsg>(mpTableA, 19, 0);
        CowDrawOne<RPCowExMsg>(mpTableA, 20, 0);
        CowDrawOne<RPCowOrderMsg>(mpTableA, 21, 0);
        for (int i = 0; i < mPlayerNum; i++) {
            CowGetActor<RPCowTargetMarker>(mpTableA, 22, i)->DrawScreen(Lyt_getCurrentViewNo());
        }
        for (int i = 0; i < mPlayerNum; i++) {
            CowGetActor<RPCowBalloon>(mpTableA, 23, i)->DrawScreen(Lyt_getCurrentViewNo());
        }
        CowDrawOne<RPCowStartMsg>(mpTableA, 24, 0);
        CowDrawOne<RPCowGoalMsg>(mpTableA, 25, 0);
        CowDrawOne<RPCowLimitCount>(mpTableA, 26, 0);
        CowDrawOne<RPCowVsResult>(mpTableA, 27, 0);
        CowDrawOne<RPCowAngleDisp>(mpTableA, 28, 0);
        for (int i = 0; i < 32; i++) {
            CowGetActor<RPCowPointPopup>(mpTableA, 17, i)->DrawScreen(Lyt_getCurrentViewNo());
        }
    }
}

void RPCowGame::DrawScreen(u8 pass) {
    if (pass == 2 || pass == 3) {
        RPTextWriter writer;
        writer.Begin(-1);
        writer.End();
    }
    if (pass == 4) {
        RPTextWriter writer;
        writer.Begin(-1);
        writer.End();
    }
}

void RPCowGame::DrawPass(u8 pass) {
    if (pass == 0) {
        Cow_objectColliders.DrawAll();
    }
    if (pass == 2) {
        sColliderListB.DrawAll();
    }
}

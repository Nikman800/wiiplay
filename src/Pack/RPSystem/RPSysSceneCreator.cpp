#include <Pack/RPAudio.h>
// Ported from doldecomp/ogws (CC0) src/Pack/RPSystem/RPSysSceneCreator.cpp @ f5a1bb5e; Wii Play changes marked "Wii Play:".
#include <Pack/RPSystem.h>

#include <Pack/RPSystem/RPSysBootScene.h>
#include <Pack/RPSystem/RPSysNunchukCheckScene.h>
#include <Pack/RPSystem/RPSysPlayerSelectScene.h>

#if defined(PACK_SPORTS)
#include <Pack/RPSports/RPBowScene.h>
#include <Pack/RPSports/RPBoxScene.h>
#include <Pack/RPSports/RPBsbScene.h>
#include <Pack/RPSports/RPGolScene.h>
#include <Pack/RPSports/RPSportsCommon.h>
#include <Pack/RPSports/RPTnsScene.h>
#elif defined(PACK_PARTY)
#include <Pack/RPParty/RPBilScene.h>
#include <Pack/RPParty/RPBomScene.h>
#include <Pack/RPParty/RPCowScene.h>
#include <Pack/RPParty/RPDucScene.h>
#include <Pack/RPParty/RPFshScene.h>
#include <Pack/RPParty/RPHkyScene.h>
#include <Pack/RPParty/RPPartyCommon.h>
#include <Pack/RPParty/RPPartyCommon/RPAvatarScene.h>
#include <Pack/RPParty/RPPartyCommon/RPPartyRootScene/RPPartyRootScene.h>
#include <Pack/RPParty/RPPnpScene.h>
#include <Pack/RPParty/RPTnkScene.h>
#include <Pack/RPParty/RPWlyScene.h>
#endif

// Wii Play: DestroyInstance is not in the binary (unused, dead-stripped)
RP_SINGLETON_IMPL_EX_NO_DESTROY(RPSysSceneCreator);

#if defined(PACK_SPORTS)
#define RP_COMMON_DIR "RPCommon/"
#elif defined(PACK_PARTY)
// Wii Play: "RPCommonParty/"
#define RP_COMMON_DIR "RPCommonParty/"
#endif

const RPSysSceneCreator::SceneAttr RPSysSceneCreator::SCENE_ATTR_TABLE[] = {
    // clang-format off
    {ESceneID_RPSysBootScene,               EPackID_AllPack,    ECreateType_Standard, ELoadType_Blank, EExitType_Sibling,  EAudioType_Common, RP_COMMON_DIR,           /* Logo                         */ "ロゴ"},
    {ESceneID_RPSysPlayerSelectScene,       EPackID_AllPack,    ECreateType_Standard, ELoadType_Blank, EExitType_2,        EAudioType_Common, RP_COMMON_DIR,           /* Player Select                */ "プレイヤーセレクト"},
    {ESceneID_RPSysNunchukCheckScene,       EPackID_AllPack,    ECreateType_Sibling,  ELoadType_Blank, EExitType_3,        EAudioType_Scene,  RP_COMMON_DIR,           /* Nunchuk Check                */ "ヌンチャクチェック"},

#if defined(PACK_SPORTS)
    {ESceneID_RPBsbScene,                   EPackID_SportsPack, ECreateType_Sibling,  ELoadType_Blank, EExitType_Standard, EAudioType_Scene,  "RPBsbScene/",           /* Baseball                     */ "野球"},
    {ESceneID_RPBowScene,                   EPackID_SportsPack, ECreateType_Sibling,  ELoadType_Blank, EExitType_Standard, EAudioType_Scene,  "RPBowScene/",           /* Bowling                      */ "ボウリング"},
    {ESceneID_RPGolScene,                   EPackID_SportsPack, ECreateType_Sibling,  ELoadType_Blank, EExitType_Standard, EAudioType_Scene,  "RPGolScene/",           /* Golf                         */ "ゴルフ"},
    {ESceneID_RPTnsScene,                   EPackID_SportsPack, ECreateType_Sibling,  ELoadType_Blank, EExitType_Standard, EAudioType_Scene,  "RPTnsScene/",           /* Tennis                       */ "テニス"},
    {ESceneID_RPBoxScene,                   EPackID_SportsPack, ECreateType_Sibling,  ELoadType_Blank, EExitType_Standard, EAudioType_Scene,  "RPBoxScene/",           /* Boxing                       */ "ボクシング"},
    {ESceneID_RPSportsPackTitleScene,       EPackID_SportsPack, ECreateType_Standard, ELoadType_Blank, EExitType_Sibling,  EAudioType_Common, "RPSportsTitle/",        /* Sports Pack                  */ "スポーツパック"},
    {ESceneID_RPSportsMainScene,            EPackID_SportsPack, ECreateType_Standard, ELoadType_Blank, EExitType_2,        EAudioType_Common, "RPSportsCommon/",       /* Sports Menu Select           */ "スポーツメニューセレクト"},
    {ESceneID_RPSportsTrainingMenuScene,    EPackID_SportsPack, ECreateType_Sibling,  ELoadType_Blank, EExitType_3,        EAudioType_Common, "RPSportsTrainingMenu/", /* Training Menu                */ "トレーニングメニュー"},
    {ESceneID_RPSportsPhysicalMenuScene,    EPackID_SportsPack, ECreateType_Sibling,  ELoadType_Blank, EExitType_3,        EAudioType_Common, "RPSportsPhysical/",     /* Physical Measure Menu        */ "体力測定メニュー"},
    {ESceneID_RPSportsPhysicalPreviewScene, EPackID_SportsPack, ECreateType_Sibling,  ELoadType_Blank, EExitType_3,        EAudioType_Common, "RPSportsPhysical/",     /* Physical Measure Explanation */ "体力測定説明"},
    {ESceneID_RPSportsPhysicalResultScene,  EPackID_SportsPack, ECreateType_Sibling,  ELoadType_Blank, EExitType_3,        EAudioType_Common, "RPSportsPhysical/",     /* Physical Measure Results     */ "体力測定結果"},
    {ESceneID_RPGolCourseSelectScene,       EPackID_SportsPack, ECreateType_Sibling,  ELoadType_Blank, EExitType_Standard, EAudioType_Scene,  "RPGolScene/",           /* Golf Course Select           */ "ゴルフコースセレクト"},
#endif

#if defined(PACK_PARTY)
    {ESceneID_RPFshScene,                   EPackID_PartyPack,  ECreateType_Sibling,  ELoadType_Blank, EExitType_Standard, EAudioType_Scene,  "RPFshScene/",           /* Fishing                      */ "釣り"},
    {ESceneID_RPHkyScene,                   EPackID_PartyPack,  ECreateType_Sibling,  ELoadType_Blank, EExitType_Standard, EAudioType_Scene,  "RPHkyScene/",           /* Hockey                       */ "ホッケー"},
    {ESceneID_RPDucScene,                   EPackID_PartyPack,  ECreateType_Sibling,  ELoadType_Blank, EExitType_Standard, EAudioType_Scene,  "RPDucScene/",           /* Shooting                     */ "射撃"},
    {ESceneID_RPPnpScene,                   EPackID_PartyPack,  ECreateType_Sibling,  ELoadType_Blank, EExitType_Standard, EAudioType_Scene,  "RPPnpScene/",           /* Table Tennis                 */ "卓球"},
    {ESceneID_RPBilScene,                   EPackID_PartyPack,  ECreateType_Sibling,  ELoadType_Blank, EExitType_Standard, EAudioType_Scene,  "RPBilScene/",           /* Billiards                    */ "ビリヤード"},
    {ESceneID_RPCowScene,                   EPackID_PartyPack,  ECreateType_Sibling,  ELoadType_Blank, EExitType_Standard, EAudioType_Scene,  "RPCowScene/",           /* Cow Race                     */ "牛レース"},
    {ESceneID_RPWlyScene,                   EPackID_PartyPack,  ECreateType_Sibling,  ELoadType_Blank, EExitType_Standard, EAudioType_Scene,  "RPWlyScene/",           /* Wally                        */ "ウォーリー"},
    {ESceneID_RPTnkScene,                   EPackID_PartyPack,  ECreateType_Sibling,  ELoadType_Blank, EExitType_Standard, EAudioType_Scene,  "RPTnkScene/",           /* Tank                         */ "タンク"},
    {ESceneID_RPBomScene,                   EPackID_PartyPack,  ECreateType_Sibling,  ELoadType_Blank, EExitType_Standard, EAudioType_Scene,  "RPBomScene/",           /* Bomb                         */ "爆弾"},
    {ESceneID_RPPartyPackTitleScene,        EPackID_PartyPack,  ECreateType_Standard, ELoadType_Blank, EExitType_Sibling,  EAudioType_Common, "RPPartyTitle/",         /* Party Pack                   */ "パーティパック"},
    {ESceneID_RPPartyRootScene,             EPackID_PartyPack,  ECreateType_Standard, ELoadType_Blank, EExitType_Child,    EAudioType_Common, "RPPartyCommon/",        /* Tour Menu                    */ "ツアーメニュー"},
    {ESceneID_RPPartyMainScene,             EPackID_PartyPack,  ECreateType_Sibling,  ELoadType_Blank, EExitType_3,        EAudioType_Common, "RPPartyCommon/",        /* Tour Menu                    */ "ツアーメニュー"},
#endif
    // clang-format on
};

// Wii Play: only inlined into CreateInstance
inline RPSysSceneCreator::RPSysSceneCreator(EGG::Heap* pHeap)
    : mpParentHeap(pHeap), mPrevSceneID(-1) {}

// Wii Play: inlined everywhere
inline s32 RPSysSceneCreator::getEntryIndex(ESceneID id) {
    for (s32 i = 0; i < ARRAY_SIZE(SCENE_ATTR_TABLE); i++) {
        if (SCENE_ATTR_TABLE[i].scene == id) {
            return i;
        }
    }

    return -1;
}

bool RPSysSceneCreator::changeSceneAfterFade(s32 id, bool reconfigure) {
    bool success = false;

    if (RP_GET_INSTANCE(RPSysSceneMgr)->isTaskFinished()) {
        int current = RP_GET_INSTANCE(RPSysSceneMgr)->getCurrentSceneID();

        if (reconfigure) {
            id = current;
        }

        success = RP_GET_INSTANCE(RPSysSceneMgr)
                      ->changeNextSceneAfterFade(id, reconfigure);

        if (success && current >= 0) {
            // Fade out audio before switching sound archives
            if (getAudioType(current) != getAudioType(id)) {
                u16 frames = RP_GET_INSTANCE(RPSysSceneMgr)->getFadeFrame();
                RP_GET_INSTANCE(RPSndAudioMgr)->fadeOutAllSound(frames);
            }

            if (getExitType(id) != EExitType_Child) {
                mPrevSceneID = current;
            }
        }
    }

    return success;
}

bool RPSysSceneCreator::changeSoftReset() {
#if defined(PACK_SPORTS)
    return changeSceneAfterFade(ESceneID_RPSportsPackTitleScene);
#elif defined(PACK_PARTY)
    return changeSceneAfterFade(ESceneID_RPPartyPackTitleScene);
#endif
}

EGG::Scene* RPSysSceneCreator::create(s32 id) {
    RPSysScene* pScene = NULL;

    switch (getPackID(id)) {
    case EPackID_SportsPack: {
#if defined(PACK_SPORTS)
        pScene = createSportsScene(id);
#else
        pScene = NULL;
#endif
        break;
    }

    case EPackID_PartyPack: {
#if defined(PACK_PARTY)
        pScene = createPartyScene(id);
#else
        pScene = NULL;
#endif
        break;
    }

    case EPackID_HealthPack: {
#if defined(PACK_HEALTH)
        pScene = createHealthScene(id);
#else
        pScene = NULL;
#endif
        break;
    }

    case EPackID_MusicPack: {
#if defined(PACK_MUSIC)
        pScene = createMusicScene(id);
#else
        pScene = NULL;
#endif
        break;
    }

    case EPackID_AllPack: {
        switch (id) {
        case ESceneID_RPSysBootScene: {
            pScene = new RPSysBootScene();
            break;
        }

        case ESceneID_RPSysPlayerSelectScene: {
            pScene = new RPSysPlayerSelectScene();
            break;
        }

        case ESceneID_RPSysNunchukCheckScene: {
            pScene = new RPSysNunchukCheckScene();
            break;
        }

        default: {
            pScene = NULL;
            break;
        }
        }

        break;
    }

    default: {
        break;
    }
    }

    if (pScene != NULL) {
        if (getLoadType(id) == ELoadType_Message) {
            pScene->enableLoadMessage();
        }

        if (getExitType(id) == EExitType_Child) {
            pScene->setChildScene();
        }

        pScene->setCreatorSceneID(mPrevSceneID);
    }

    return pScene;
}

void RPSysSceneCreator::destroy(s32 /* id */) {
    if (RP_GET_INSTANCE(RPSysSceneMgr)->isApplicationEndReserved()) {
        RP_GET_INSTANCE(RPSysSystem)->systemShutDown();
    }
    // Wii Play: also shuts down after a disc error
    else if (!RP_GET_INSTANCE(RPSysSceneMgr)->isNormalState() &&
             !RP_GET_INSTANCE(RPSysSystem)->checkDisk()) {
        RP_GET_INSTANCE(RPSysSystem)->systemShutDown();
    }
}

const char* RPSysSceneCreator::getResDirName(s32 id) {
    if (id < 0) {
        id = RP_GET_INSTANCE(RPSysSceneMgr)->getCurrentSceneID();
    }

    s32 index = getEntryIndex(static_cast<ESceneID>(id));

// @bug Table index is not validated
#if defined(BUG_FIX)
    if (index < 0) {
        return RP_COMMON_DIR;
    }
#endif

    return SCENE_ATTR_TABLE[index].pResDirName;
}

// Wii Play: getSportID is not in the binary (unused, dead-stripped)

s32 RPSysSceneCreator::getSceneID(ESportID sport) {
    switch (sport) {
    case ESportID_Bsb: {
        return ESceneID_RPBsbScene;
    }

    case ESportID_Tns: {
        return ESceneID_RPTnsScene;
    }

    case ESportID_Gol: {
        return ESceneID_RPGolScene;
    }

    case ESportID_Box: {
        return ESceneID_RPBoxScene;
    }

    case ESportID_Bow: {
        return ESceneID_RPBowScene;
    }

    default: {
        return -1;
    }
    }
}

// Wii Play: inlined (no out-of-line copy in the binary)
inline RPSysSceneCreator::EPackID RPSysSceneCreator::getPackID(s32 id) {
    s32 index = getEntryIndex(static_cast<ESceneID>(id));

// @bug Table index is not validated
#if defined(BUG_FIX)
    if (index < 0) {
        return EPackID_AllPack;
    }
#endif

    return SCENE_ATTR_TABLE[index].pack;
}

RPSysSceneCreator::ECreateType RPSysSceneCreator::getCreateType(s32 id) {
    s32 index = getEntryIndex(static_cast<ESceneID>(id));

// @bug Table index is not validated
#if defined(BUG_FIX)
    if (index < 0) {
        return ECreateType_Standard;
    }
#endif

    return SCENE_ATTR_TABLE[index].createType;
}

// Wii Play: inlined (no out-of-line copy in the binary)
inline RPSysSceneCreator::ELoadType RPSysSceneCreator::getLoadType(s32 id) {
    s32 index = getEntryIndex(static_cast<ESceneID>(id));

// @bug Table index is not validated
#if defined(BUG_FIX)
    if (index < 0) {
        return ELoadType_Blank;
    }
#endif

    return SCENE_ATTR_TABLE[index].loadType;
}

RPSysSceneCreator::EExitType RPSysSceneCreator::getExitType(s32 id) {
    s32 index = getEntryIndex(static_cast<ESceneID>(id));

// @bug Table index is not validated
#if defined(BUG_FIX)
    if (index < 0) {
        return EExitType_Standard;
    }
#endif

    return SCENE_ATTR_TABLE[index].exitType;
}

RPSysSceneCreator::EAudioType RPSysSceneCreator::getAudioType(s32 id) {
    s32 index = getEntryIndex(static_cast<ESceneID>(id));

// @bug Table index is not validated
#if defined(BUG_FIX)
    if (index < 0) {
        return EAudioType_Scene;
    }
#endif

    return SCENE_ATTR_TABLE[index].audioType;
}

#if defined(PACK_PARTY)
// Wii Play: the scenes' constructors are inline (only the vtables are set)
inline RPFshScene::RPFshScene() {}
inline RPHkyScene::RPHkyScene() {}
inline RPDucScene::RPDucScene() {}
inline RPPnpScene::RPPnpScene() {}
inline RPBilScene::RPBilScene() {}
inline RPCowScene::RPCowScene() {}
inline RPWlyScene::RPWlyScene() {}
inline RPTnkScene::RPTnkScene() {}
inline RPBomScene::RPBomScene() {}

RPSysScene* RPSysSceneCreator::createPartyScene(s32 id) {
    switch (id) {
    case ESceneID_RPFshScene: {
        return new RPFshScene();
    }

    case ESceneID_RPHkyScene: {
        return new RPHkyScene();
    }

    case ESceneID_RPDucScene: {
        return new RPDucScene();
    }

    case ESceneID_RPPnpScene: {
        return new RPPnpScene();
    }

    case ESceneID_RPBilScene: {
        return new RPBilScene();
    }

    case ESceneID_RPCowScene: {
        return new RPCowScene();
    }

    case ESceneID_RPWlyScene: {
        return new RPWlyScene();
    }

    case ESceneID_RPTnkScene: {
        return new RPTnkScene();
    }

    case ESceneID_RPBomScene: {
        return new RPBomScene();
    }

    case ESceneID_RPPartyPackTitleScene: {
        // Wii Play: the title scene class is RPCommonMessage in this project (vtable 80358040)
        return new RPCommonMessage();
    }

    case ESceneID_RPPartyRootScene: {
        return new RPPartyRootScene();
    }

    case ESceneID_RPPartyMainScene: {
        return new RPAvatarScene();
    }

    default: {
        return NULL;
    }
    }
}
#endif

// Wii Play: the binary has the weak ~RPSysScene right after ~RPSysSceneCreator, before
// createPartyScene. A weak copy comes out right after the first function (in output order) that
// needs it, so the original file had a function here, between createPartyScene and the
// destructor, that constructs a scene inline and that the linker dead-stripped (like Wii Sports,
// which has the same layout before createSportsScene: probably another pack's create function).
// This stand-in plays that role; nothing calls it, so it is not in the DOL.
RPSysScene* RPSysSceneCreator_DeadStripped() {
    return new RPSysBootScene();
}

RPSysSceneCreator::~RPSysSceneCreator() {}

// Tanks tread-mark textures (RPTnkFootmarkTex).
// Unit 0x80274CA4-0x80274E84 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPTnkScene/RPTnkTask.h>

#include <Pack/RPSystem/RPSysResourceManager.h>

RPTnkFootmarkTex* RPTnkFootmarkTex::sInstance;

void RPTnkFootmarkTex::CreateInstance() {
    sInstance = new RPTnkFootmarkTex();
}

void RPTnkFootmarkTex::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPTnkFootmarkTex::~RPTnkFootmarkTex() {}

void RPTnkFootmarkTex::Create() {}

void RPTnkFootmarkTex::Load() {
    const char* names[11] = {
        "tnk_footmark/tnk_mark_1p.bti",  "tnk_footmark/tnk_mark_2p.bti",
        "tnk_footmark/tnk_mark_e_1.bti", "tnk_footmark/tnk_mark_e_2.bti",
        "tnk_footmark/tnk_mark_e_3.bti", "tnk_footmark/tnk_mark_e_4.bti",
        "tnk_footmark/tnk_mark_e_5.bti", "tnk_footmark/tnk_mark_e_6.bti",
        "tnk_footmark/tnk_mark_e_7.bti", "tnk_footmark/tnk_mark_e_8.bti",
        "tnk_footmark/tnk_mark_e_9.bti",
    };
    EGG::Archive* pArchive = RPSysResourceManager::LoadGameCommonArchive();
    for (s32 i = 0; i < 11; i++) {
        mpTextures[i] = RPSysResourceManager::GetFileFromArchive(pArchive, names[i]);
    }
}

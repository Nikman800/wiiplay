// Shooting Range grass tuft (RPDucShtGrass): shake animation, hit effect and rustle sound.
// Unit 0x802368F8-0x80236FA0 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPDucScene/RPDucField.h>

#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPDucScene/RPDucUtil.h>
#include <Pack/RPSystem.h>

RPDucShtGrass::RPDucShtGrass() {
    mAnmEndFrame = 0.0f;
    mAnmState = 0;
}

RPDucShtGrass::~RPDucShtGrass() {}

void RPDucShtGrass::Create() {
    mpSndActor = new RPDucSndActor();
    mAnmEndFrame = 0.0f;
    mAnmState = 0;
    mHasSecret = false;
}

void RPDucShtGrass::Load() {
    EGG::Archive* pArchive = RPSysResourceManager::LoadGameCommonArchive();
    RPGrpModelResManager* pResMgr = RPGrpModelResManager::GetCurrent();
    RPGrpHandle file = pResMgr->CreateData(
        RPGrpModelResManager::Type_ResFile,
        RPSysResourceManager::GetFileFromArchive(pArchive, "G3D/sht_grass.brres"));
    mpModel = RPGrpModel::Construct(
        pResMgr->CreateData(RPGrpModelResManager::Type_ResMdl,
                            pResMgr->GetPtr(RPGrpModelResManager::Type_ResMdl, file, 0)),
        1, 0, 0);

    RPGrpHandle wait = RPDucCreateRes(pResMgr, RPGrpModelResManager::Type_ResAnmChr, file, "sht_grass_wait");
    RPGrpHandle shake = RPDucCreateRes(pResMgr, RPGrpModelResManager::Type_ResAnmChr, file, "sht_grass_shake");
    CreateAnmBuffer(RPGrpModelAnm::Anm_Chr, 2);
    RegistAnm(RPGrpModelAnm::Anm_Chr, 0, wait);
    RegistAnm(RPGrpModelAnm::Anm_Chr, 1, shake);
}

void RPDucShtGrass::Reset() {
    mPos = RP_vecZero;
    mScale = RP_vecOne;
    mRot = RP_mtx33Identity;
    StartAnm(RPGrpModelAnm::Anm_Chr, 0, 0.0f);
    mAnmEndFrame = GetAnmNumFrame(RPGrpModelAnm::Anm_Chr, 0);
    mAnmState = 0;
}

void RPDucShtGrass::Calculate() {
    if (mAnmState == 1) {
        if (mAnmEndFrame - 5.0f <= GetAnmFrame(RPGrpModelAnm::Anm_Chr, 1)) {
            StartAnmBlend(RPGrpModelAnm::Anm_Chr, 0, 5.0f, 0.0f);
            mAnmEndFrame = GetAnmNumFrame(RPGrpModelAnm::Anm_Chr, 0);
            mAnmState = 0;
        }
    }

    mpSndActor->set3DPosition(mPos);
}

void RPDucShtGrass::StartShake() {
    StartAnm(RPGrpModelAnm::Anm_Chr, 1, 0.0f);
    mAnmEndFrame = GetAnmNumFrame(RPGrpModelAnm::Anm_Chr, 1);
    mAnmState = 1;
}

void RPDucShtGrass::SpawnHitEffect() {
    RPSysEffect effect("Sht_grass_hit", 0, 0);
    effect.create();
    effect.setPos(nw4r::math::VEC3(mPos.x, mPos.y, mPos.z));
    effect.setScale(nw4r::math::VEC3(1.0f, 1.0f, 1.0f));
    effect.update();
}

void RPDucShtGrass::PlaySound(s32 type) {
    u32 flag = 0;
    if (type == 0) {
        flag = 1;
    } else if (type == 1) {
        flag = 2;
    }

    if (mHasSecret) {
        mpSndActor->startSoundWithRemotePlayer(3U, flag, 0);
    } else {
        mpSndActor->startSoundWithRemotePlayer(2U, flag, 0);
    }
}

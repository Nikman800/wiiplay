#include <Pack/RPParty/RPWlyScene/RPWlyAvatarMgr_2.h>
#include <Pack/RPParty/RPWlyScene/RPWlyCursor.h>

// Find Mii stage clear phase (RPWlyClearPhase). Unit 0x80280070-0x8028070C, linked before the
// stage code (RPWlyAvatarMgr_2.cpp): .rodata "Wtd_clear", the vtable and its own 1.0f.

enum WlyGameMode {
    WlyGameMode_1P,
    WlyGameMode_2P,
};

static inline WlyGameMode GetGameMode() {
    return static_cast<WlyGameMode>(RPWlyGameMgr::sInstance->mMode);
}

RPWlyClearPhase::~RPWlyClearPhase() {}

void RPWlyClearPhase::Start() {
    mStep = 1;
    mTimer = 0;
    nw4r::math::VEC3 pos = RPSimpleModel::sVecZero;
    nw4r::math::VEC3 scale(1.0f, 1.0f, 1.0f);
    RPSysEffect effect("Wtd_clear", 5, 0);
    effect.create();
    effect.setPos(pos);
    effect.setScale(scale);
    effect.update();
}

#pragma opt_common_subs off
void RPWlyClearPhase::UpdateSequence() {
    mTimer++;
    WlyGameMode mode = GetGameMode();
    if (mode == WlyGameMode_1P) {
        switch (mTimer) {
        case 10:
        case 20:
        case 30:
        case 40:
        case 50:
        case 60:
        case 70:
        case 80:
        case 90:
        case 100: {
            s32 timer = RPWlyGameMgr::sInstance->mTimer + 60;
            if (timer > 7200) {
                timer = 7200;
            } else {
                RPSndAudioMgr::instance()->startSystemSe(0U);
            }
            RPWlyGameMgr::sInstance->mTimer = timer;
            break;
        }
        case 120:
            RPWlyLayoutSet::sInstance->mpTimer->UpdateWarnAnim();
            break;
        }
    }

    if (mTimer >= 160) {
        mStep = 2;
        mTimer = 0;
        RPSceneInfo::spInstance->SetUnk1C(RPSceneInfo::spInstance->GetUnk1C() + 1);
        RPWlyGameConfig config = RPSceneInfo::spInstance->mMiiEntry.GetConfig();
        // Written here, not in an inline: i is then a named local, coloured after the three
        // walkers (r17), as in the original.
        config.mMode = RPWlyGameMgr::sInstance->mMode;
        for (int i = 0; i < 2; i++) {
            config.mScores[i] = RPWlyGameMgr::sInstance->mScores[i];
            config.mCursorPos[i] = RPWlyGameMgr::sInstance->mpCursors[i]->GetPos();
            config.mCursorAngles[i] = RPWlyGameMgr::sInstance->mpCursors[i]->mAngle;
        }
        config.mTimer = RPWlyGameMgr::sInstance->mTimer;
        RPSceneInfo::spInstance->mMiiEntry.SetConfig(config);
    }
}

#pragma opt_common_subs reset

void RPWlyClearPhase::Init() {
    mStep = 0;
}

void RPWlyClearPhase::Create() {}

void RPWlyClearPhase::Reset() {
    mTimer = 0;
    mStep = 0;
}

void RPWlyClearPhase::Calculate() {
    if (RPWlyGameMgr::sInstance->mState == 3) {
        switch (mStep) {
        case 0:
            Start();
            break;
        case 1:
            UpdateSequence();
            break;
        case 2:
            break;
        }
    }
}

void RPWlyClearPhase::Destroy() {}

void RPWlyClearPhase::Draw() {}

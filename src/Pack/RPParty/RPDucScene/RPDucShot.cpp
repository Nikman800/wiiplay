// Shooting Range gun (RPDucShot): aim from the pointer, cursor effects, ray tests against the
// grass and the targets.
// Unit 0x80242B40-0x80244648 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPDucScene/RPDucShot.h>

#include <Pack/RPAudio.h>
#include <Pack/RPGraphics.h>
#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPDucScene/RPDucCamera.h>
#include <Pack/RPParty/RPDucScene/RPDucField.h>
#include <Pack/RPParty/RPDucScene/RPDucGame.h>
#include <Pack/RPParty/RPDucScene/RPDucTarget.h>
#include <Pack/RPParty/RPDucScene/RPDucTargetMgr.h>
#include <Pack/RPParty/RPPartyCommon/RPCursorTrail.h>
#include <Pack/RPParty/RPPartyCommon/RPSceneInfo.h>
#include <Pack/RPSystem.h>

#include <egg/core.h>
#include <egg/gfxe.h>

extern nw4r::math::VEC2 RP_vec2Zero;

RPDucShot::RPDucShot() {
    mPos = RP_vecZero;
    mScale = RP_vecOne;
    mRot = RP_mtx33Identity;
    mScreenPos = RP_vec2Zero;
    mPrevPos = mPos;
    mShotPending = 0;
    mShotTimer = 0;
    mRayDir = mRot.z;
    mViewDir = mRot.z;
    mPlayer = 0;
}

RPDucShot::~RPDucShot() {}

inline f32 ShotClamp(f32 value, f32 lo, f32 hi) {
    f32 input = value;
    f32 x = lo > input ? lo : input;
    return hi < x ? hi : x;
}

inline void RPDucShot::SetScreenPosition(nw4r::math::VEC2 pos) {
    if (EGG::Screen::GetTVMode() == 1) {
        mScreenPos.x = ShotClamp(pos.x, -0.965f, 0.965f);
        mScreenPos.y = ShotClamp(pos.y, -0.54f, 0.54f);
    } else if (EGG::Screen::GetTVMode() == 0) {
        mScreenPos.x = ShotClamp(pos.x, -0.985f, 0.985f);
        mScreenPos.y = ShotClamp(pos.y, -0.735f, 0.735f);
    }
}

void RPDucShot::UpdateAim() {
    RPDucCamera* pCamera = RPDucCamera::sInstance;
    nw4r::math::VEC3 dir = pCamera->GetDir();
    nw4r::math::VEC3 side = pCamera->GetSide();
    side.x = -side.x;
    side.y = -side.y;
    side.z = -side.z;
    nw4r::math::VEC3 up;
    PSVECCrossProduct(side, dir, up);

    nw4r::math::VEC3 camPos = pCamera->GetLookPos();
    u16 screenHeight = EGG::Screen::GetSizeYMax();
    const RPGrpScreen* pScreen = RPGrpRenderer::GetCurrent()->GetView(0)->GetScreen();
    f32 offsetY;
    f32 unit;
    f32 depth;
    depth = pScreen->GetNearZ();
    depth += 0.2f;
    f32 tangent = pScreen->GetTanFovy();
    unit = (tangent * depth) / (0.5f * screenHeight);
    offsetY = pScreen->GetOffset().y;
    nw4r::math::VEC2 offset;
    mpCursorTrail->Update();
    if (mpCursorTrail->GetLatest() != -1) {
        nw4r::math::VEC2 pos = mpCursorTrail->GetPos(mpCursorTrail->GetLatest());
        SetScreenPosition(pos);
    }

    f32 unscaledX;
    f32 scaledY;
    f32 scaledX;
    mPos.x = dir.x * depth + camPos.x;
    unscaledX = mScreenPos.x * mScreenHalfWidth;
    offset.x = unscaledX;
    offset.y = mScreenPos.y * mScreenHalfWidth;
    offset.y += offsetY;
    offset.x = scaledX = offset.x * unit;
    offset.y = scaledY = offset.y * unit;
    mPos.y = dir.y * depth + camPos.y;
    mPos.z = dir.z * depth + camPos.z;
    mPos.x = mPos.x + (side.x * scaledX - up.x * scaledY);
    mPos.y = mPos.y + (side.y * scaledX - up.y * scaledY);
    mPos.z = mPos.z + (side.z * scaledX - up.z * scaledY);
    mScale.x = 0.082f;
    mScale.y = 0.082f;
    mScale.z = 0.082f;
    mRot.x = side;
    mRot.y = up;
    mRot.z.x = -dir.x;
    mRot.z.y = -dir.y;
    mRot.z.z = -dir.z;
    mRayDir.x = mPos.x - camPos.x;
    mRayDir.y = mPos.y - camPos.y;
    mRayDir.z = mPos.z - camPos.z;
    f32 len = PSVECMag(mRayDir);
    if (0.0f != len) {
        f32 inv = 1.0f / len;
        mRayDir.x *= inv;
        mRayDir.y *= inv;
        mRayDir.z *= inv;
    }
    mViewDir = dir;
}

bool RPDucShot::TestRaySphere(const nw4r::math::VEC3& rCenter, f32 radius, nw4r::math::VEC3* pHit,
                              f32* pDist) {
    nw4r::math::VEC3 diff;
    diff.x = mPos.x - rCenter.x;
    diff.y = mPos.y - rCenter.y;
    diff.z = mPos.z - rCenter.z;
    if (nw4r::math::VEC3Dot(&diff, &mRayDir) > 0.0f) {
        *pHit = RP_vecZero;
        return false;
    }
    nw4r::math::VEC3 perp(0.0f, 0.0f, 0.0f);
    RPCalcPerpendicularDir(&perp, &rCenter, &mPos, &mRayDir);
    if (0.0f == PSVECMag(perp)) {
        pHit->x = rCenter.x - mRayDir.x * radius;
        pHit->y = rCenter.y - mRayDir.y * radius;
        pHit->z = rCenter.z - mRayDir.z * radius;
        return true;
    }
    f32 dist = nw4r::math::VEC3Dot(&diff, &perp);
    if (dist < 3.2f + radius) {
        f32 t = (dist * radius) / (3.2f + radius);
        f32 h = nw4r::math::FSqrt(radius * radius - t * t);
        pHit->x = (perp.x * t + rCenter.x) - mRayDir.x * h;
        pHit->y = (perp.y * t + rCenter.y) - mRayDir.y * h;
        pHit->z = (perp.z * t + rCenter.z) - mRayDir.z * h;
        *pDist = dist;
        return true;
    }
    return false;
}

void RPDucShot::PlayShotSound() {
    RPDucGame* pGame = RPDucGame::sInstance;
    pGame->GetCurrentRound();
    switch (pGame->GetSpawnPattern()) {
    case 0:
    case 1:
        if (mPlayer == 0) {
            mpSndActor->startSound(4U);
        } else if ((u32)mPlayer == 1) {
            mpSndActor->startSound(40U);
        }
        break;
    case 2:
        if (mPlayer == 0) {
            mpSndActor->startSound(5U);
        } else if ((u32)mPlayer == 1) {
            mpSndActor->startSound(41U);
        }
        break;
    case 3:
    case 4:
        if (mPlayer == 0) {
            mpSndActor->startSound(6U);
        } else if ((u32)mPlayer == 1) {
            mpSndActor->startSound(42U);
        }
        break;
    case 5:
    case 6:
        if (mPlayer == 0) {
            mpSndActor->startSound(7U);
        } else if ((u32)mPlayer == 1) {
            mpSndActor->startSound(43U);
        }
        break;
    case 7:
        if (mPlayer == 0) {
            mpSndActor->startSound(8U);
        } else if ((u32)mPlayer == 1) {
            mpSndActor->startSound(44U);
        }
        break;
    case 9:
        if (mPlayer == 0) {
            mpSndActor->startSound(9U);
        } else if ((u32)mPlayer == 1) {
            mpSndActor->startSound(45U);
        }
        break;
    default:
        if (mPlayer == 0) {
            mpSndActor->startSound(4U);
        } else if ((u32)mPlayer == 1) {
            mpSndActor->startSound(40U);
        }
        break;
    }
}

void RPDucShot::Create() {
    mpCursorTrail = new RPCursorTrail(0, 10);
    mpPointer = new RPDucShotPointer(0, 100);
    mpSndActor = new RPDucSndActor();
}

void RPDucShot::Reset() {
    mScreenHalfWidth = (u16)((u32)EGG::Screen::GetSizeXMax() >> 1);
    mScreenPos.x = mPlayer == 0 ? -0.4f : 0.4f;
    mScreenPos.y = 0.15f;
    mPrevPos = mPos;
    mpCursorTrail->SetChannel(mPlayer);
    mpCursorTrail->Reset();
    mpPointer->SetChannel(mPlayer);
    mpPointer->Clear();
    mShotPending = 0;
    mShotTimer = 0;
    mFireCooldown = 0;
}

void RPDucShot::RestartMainEffect() {
    RPSysEffect* pEffect;
    switch (mPlayer) {
    case 0:
        pEffect = mpMainEffects[0];
        break;
    case 1:
        pEffect = mpMainEffects[1];
        break;
    }
    pEffect->forceKill();
    pEffect->create();
    UpdateCursorEffects();
}

void RPDucShot::RestartTailEffect() {
    RPSysEffect* pEffect;
    switch (mPlayer) {
    case 0:
        pEffect = mpTailEffects[0];
        break;
    case 1:
        pEffect = mpTailEffects[1];
        break;
    }
    pEffect->forceKill();
    pEffect->create();
    UpdateCursorEffects();
}

void RPDucShot::SetCursorMode(s32 mode) {
    mCursorMode = mode;
    RPSysEffect* pMain;
    RPSysEffect* pTail;
    switch (mPlayer) {
    case 0:
        pMain = mpMainEffects[0];
        pTail = mpTailEffects[0];
        break;
    case 1:
        pMain = mpMainEffects[1];
        pTail = mpTailEffects[1];
        break;
    }
    switch (mCursorMode) {
    case 0:
        pTail->setDisableDraw(true);
        pMain->setDisableDraw(true);
        break;
    case 1:
        pTail->setDisableDraw(true);
        pMain->setDisableDraw(false);
        break;
    case 2:
        pTail->setDisableDraw(false);
        pMain->setDisableDraw(false);
        break;
    }
    UpdateCursorEffects();
}

void RPDucShot::UpdateCursorEffects() {
    nw4r::math::VEC3 pos(mPos.x, mPos.y, mPos.z);
    nw4r::math::VEC3 scale(0.082f, 0.082f, 1.0f);
    switch (mPlayer) {
    case 0:
        mpTailEffects[0]->setPos(pos);
        mpTailEffects[0]->setScale(scale);
        mpTailEffects[0]->update();
        mpMainEffects[0]->setPos(pos);
        mpMainEffects[0]->setScale(scale);
        mpMainEffects[0]->update();
        break;
    case 1:
        mpTailEffects[1]->setPos(pos);
        mpTailEffects[1]->setScale(scale);
        mpTailEffects[1]->update();
        mpMainEffects[1]->setPos(pos);
        mpMainEffects[1]->setScale(scale);
        mpMainEffects[1]->update();
        break;
    }
}

// STAND-IN: dead-stripped; compiled between Calculate and UpdateCursorEffects, it puts the
// u32->f32 magic double (lbl_8043FEC8) after Calculate's 0.0656f (8043FEC4) and before 0.082f
// (8043FED0), as in the original .sdata2. Without it the double lands after 0.15f (8043FEDC).
extern "C" f32 Duc_shotPoolOrderU32(u32 x) {
    return (f32)x;
}

void RPDucShot::Calculate() {
    if (EGG::CoreControllerMgr::instance()->getNthController((u16)mPlayer)->downTrigger(0xC00) &&
        mFireCooldown == 0) {
        u32 result = RPDucAmmoMgr::sInstance->TryFire(mPlayer);
        if (result == 2) {
            mShotPending = 1;
            mShotTimer = 10;
            mFireCooldown = 7;
            mPrevPos = mPos;
            EGG::CoreControllerMgr::instance()->getNthController((u16)mPlayer)->startSimpleRumble(
                0x0FFFFFFF, 4, 0x20);
            {
                RPSysEffect effect("Sht_shot", 0, 0);
                nw4r::math::VEC3 scale;
                effect.create();
                nw4r::math::VEC3 pos(mPrevPos.x, mPrevPos.y, mPrevPos.z);
                effect.setPos(pos);
                scale.x = 0.0656f;
                scale.y = 0.0656f;
                scale.z = 0.0656f;
                effect.setScale(scale);
                effect.update();
            }
            PlayShotSound();
        } else if (result == 1) {
            mpSndActor->startSound(10U);
        }
    }
    UpdateAim();
    mpSndActor->set3DPosition(mPos);
    UpdateCursorEffects();
    if (--mShotTimer < 0) {
        mShotTimer = 0;
    }
    if (--mFireCooldown < 0) {
        mFireCooldown = 0;
    }
}

void RPDucShot::Draw() {}

void RPDucShot::UpdatePointer() {
    mpPointer->DrawDebug();
}

void RPDucShot::CreateCursorEffects(u16 player) {
    mPlayer = player;
    switch (player) {
    case 0:
        mpMainEffects[0] = new RPSysEffect("Sht_cursor_1p_main", 3, 0);
        mpTailEffects[0] = new RPSysEffect("Sht_cursor_1p_tail", 3, 0);
        break;
    case 1:
        if (RPSceneInfo::spInstance->GetPlayerNum() == 1) {
            mpMainEffects[1] = new RPSysEffect("Sht_cursor_1p_main_2", 3, 0);
            mpTailEffects[1] = new RPSysEffect("Sht_cursor_1p_tail_2", 3, 0);
        } else if (RPSceneInfo::spInstance->GetPlayerNum() == 2) {
            mpMainEffects[1] = new RPSysEffect("Sht_cursor_2p_main", 3, 0);
            mpTailEffects[1] = new RPSysEffect("Sht_cursor_2p_tail", 3, 0);
        }
        break;
    }
    u32 flag = 0;
    if (mPlayer == 0) {
        flag = 1;
    } else if ((u32)mPlayer == 1) {
        flag = 2;
    }
    mpSndActor->SetRemoteFlag(flag);
}

void RPDucShot::ProcessHits() {
    if (mCursorMode != 0 && mShotPending != 0) {
        mShotPending--;
        CheckGrassHits();
        if (RPDucGame::sInstance->GetState() - 3U <= 1) {
            FindHitTargets();
        }
    }
}

inline void RPDucShot::CheckGrassHit(RPDucShtGrass* pGrass, f32* pDist) {
    nw4r::math::VEC3 hit;
    nw4r::math::VEC3 center;
    center.x = pGrass->mPos.x;
    center.y = 5.0f + pGrass->mPos.y;
    center.z = pGrass->mPos.z;
    if (TestRaySphere(center, 5.0f, &hit, pDist)) {
        pGrass->StartShake();
        pGrass->SpawnHitEffect();
        pGrass->PlaySound(mPlayer);
    }
}

#pragma auto_inline off
void RPDucShot::CheckGrassHits() {
    f32 dist = 0.0f;
    RPDucField* pField = RPDucField::sInstance;
    s32 i = 0;
    do {
        CheckGrassHit(pField->mpGrass[i], &dist);
    } while (++i < 27);
}
#pragma auto_inline reset

/**
 * Targets hit by one shot, nearest first
 */
struct RPDucShotHit {
    RPDucTarget* mpTarget;  // at 0x0
    f32 mDepth;            // at 0x4 how deep the ray went into the hit sphere
    f32 mDist;             // at 0x8 from the gun
    nw4r::math::VEC3 mPos;  // at 0xC
};

class RPDucShotHitList {
public:
    RPDucShotHit mHits[4];
    s32 mNum;
    RPDucShotHitList() {
        Clear();
        mNum = 0;
        Clear();
    }
    virtual ~RPDucShotHitList() {}

    void Add(RPDucTarget* pTarget, f32 depth, f32 dist, nw4r::math::VEC3 pos) {
        if (mNum == 4) {
            return;
        }
        mHits[mNum].mpTarget = pTarget;
        mHits[mNum].mDepth = depth;
        mHits[mNum].mDist = dist;
        mHits[mNum].mPos = pos;
        mNum++;
    }

    void Sort() {
        s32 a = 0;
        do {
            s32 b = a + 1;
            do {
                if (mHits[a].mDist > mHits[b].mDist) {
                    RPDucShotHit tmp = mHits[a];
                    mHits[a] = mHits[b];
                    mHits[b] = tmp;
                }
            } while (++b < mNum);
        } while (++a < mNum - 1);
    }

    void Clear() {
        s32 i = 0;
        do {
            mHits[i].mpTarget = NULL;
            mHits[i].mDepth = 0.0f;
            mHits[i].mDist = 0.0f;
            mHits[i].mPos.x = RP_vecZero.x;
            mHits[i].mPos.y = RP_vecZero.y;
            mHits[i].mPos.z = RP_vecZero.z;
        } while (++i < 4);
        mNum = 0;
    }
};

inline u32 ShotRandomOrder() {
    RP_randSeedX = RP_randSeedX * 0x41C64E6D + 12345;
    if (RP_randSeedY & 1) {
        RP_randSeedY ^= 0x11020;
    }
    RP_randSeedY >>= 1;
    return (RP_randSeedX ^ RP_randSeedY) & 0x7FFF;
}

inline s32 ShotHitOrder() {
    s32 order = ShotRandomOrder();
    return order;
}

inline RPDucShotHitList* ShotHitListPtr(RPDucShotHitList* pList) {
    RPDucShotHitList* p = pList;
    return p;
}

void RPDucShot::FindHitTargets() {
    RPDucTargetMgr* pMgr;
    RPDucTargetRegistry* const pRegistry = RPDucTargetRegistry::sInstance;
    RPDucShotHitList* pList;
    s32 mgrNum = pRegistry->mMgrNum;
    s32 k;
    nw4r::math::VEC3 center;
    f32 depth;
    s32 hitNum;
    s32 order;
    RPDucShotHitList list;
    pList = ShotHitListPtr(&list);
    list.mNum = 0;
    depth = 0.0f;
    s32 num;
    s32 i;
    nw4r::math::VEC3 hitPos(0.0f, 0.0f, 0.0f);
    i = 0;
    do {
        pMgr = pRegistry->mpMgrs[i];
        if (pMgr->mType != 6) {
            num = pMgr->mPoolNum;
            s32 j = 0;
            do {
                RPDucTarget* pTarget = pMgr->mppPool[j];
                if (pTarget->mAge < 0 || !pTarget->unkA4) {
                    continue;
                }
                center.x = pTarget->mPos.x + pTarget->mHitOffset.x;
                center.y = pTarget->mPos.y + pTarget->mHitOffset.y;
                center.z = pTarget->mPos.z + pTarget->mHitOffset.z;
                if (!TestRaySphere(center, pTarget->mHitScale, &hitPos, &depth)) {
                    continue;
                }
                nw4r::math::VEC3 diff;
                diff.x = hitPos.x - mPos.x;
                diff.y = hitPos.y - mPos.y;
                diff.z = hitPos.z - mPos.z;
                f32 dist = PSVECMag(diff);
                pList->Add(pTarget, depth, dist, hitPos);
            } while (++j < num);
        }
    } while (++i < mgrNum);

    hitNum = pList->mNum;
    if (hitNum == 0) {
        RPDucScoreMgr::sInstance->AwardStreakBonus(mPlayer);
        RPDucScoreMgr::sInstance->ResetStreak(mPlayer);
        return;
    }

    if (hitNum > 1) {
        list.Sort();
    }

    mShotPending = 0;
    order = ShotHitOrder();
    k = 0;
    do {
        RPDucShotHit* pHit = &list.mHits[k];
        RPDucTarget* pTarget = pHit->mpTarget;
        s16 player = mPlayer;
        if (order < -1) {
            pTarget->mHitOrder[player] = ShotRandomOrder();
        } else {
            pTarget->mHitOrder[player] = order;
        }
        f32 radius = 3.2f + pTarget->mHitScale;
        f32 depth = pHit->mDepth;
        pTarget->mHitCount[(s16)mPlayer] = 256.0f * (1.0f - (depth / radius));
        nw4r::math::VEC3 pos = pHit->mPos;
        pTarget->mHitPos[(s16)mPlayer] = pos;
        if (!pTarget->CanShotPassThrough()) {
            break;
        }
    } while (++k < hitNum);
}

inline u32 ShotRemoteFlag() {
    if (RPDucGame::sInstance->GetMode() != 1 && !RPDucGame::sInstance->IsP2Joined())
        return 1;
    return 3;
}

void RPDucShot::PlayControllerSound() {
    u32 flag = ShotRemoteFlag();
    if (mPlayer == 0) {
        RPSndAudioMgr::instance()->startSystemSeWithRemote(54U, flag);
    } else if ((u32)mPlayer == 1) {
        RPSndAudioMgr::instance()->startSystemSeWithRemote(55U, flag);
    }
}

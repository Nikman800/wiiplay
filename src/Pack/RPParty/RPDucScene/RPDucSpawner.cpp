#include <Pack/RPParty/RPDucScene/RPDucSpawner.h>
#include <Pack/RPParty/RPDucScene/RPDucAvatar.h>
#include <Pack/RPParty/RPDucScene/RPDucGame.h>
#include <Pack/RPParty/RPDucScene/RPDucTargetMgr.h>
#include <Pack/RPParty/RPDucScene/RPDucUtil.h>
#include <Pack/RPParty/RPDucScene/RPDucTarget.h>
#include <egg/gfxe/eggScreen.h>

using nw4r::math::VEC3;

inline u32 SpawnerRandU32() {
    RP_randSeedX = RP_randSeedX * 0x41C64E6D + 12345;
    if (RP_randSeedY & 1) RP_randSeedY ^= 0x11020;
    RP_randSeedY >>= 1;
    return RP_randSeedX ^ RP_randSeedY;
}

inline f32 SpawnerUnitRandF32() {
    return 1.192093e-7f * (f32)(SpawnerRandU32() & 0x7FFFFF);
}

inline f32 SpawnerRandF32(f32 lo, f32 hi) {
    if (lo == hi) return lo;
    return lo + (hi - lo) * SpawnerUnitRandF32();
}

inline RPDucTarget* RPDucSpawner::SpawnTarget(s32 type, const VEC3& pos, u32 arg) {
    if (mRemaining == 0) return NULL;
    RPDucTarget* target = RPDucTargetRegistry::sInstance->Spawn(type, pos, arg);
    --mRemaining;
    return target;
}

inline s16 SpawnerRandPair(s16 lo, s16 hi) {
    return (SpawnerRandU32() & 0x10) ? lo : hi;
}

inline s32 SpawnerFindFreeSlot() {
    s32 slot = (SpawnerRandU32() & 0xFFFF) % 18;
    for (;;) {
        if (!RPDucCircleTargetMgr::sInstance->IsGridSlotUsed(slot)) break;
        slot = (slot + 5) % 18;
    }
    return slot;
}

RPDucSpawner* RPDucSpawner::sInstance;

u16 Duc_type3History[8];
int Duc_balloonPatternToggle;
int Duc_clayPatternSide;
u16 Duc_canPatternLast;

s32 Duc_clayPatternTable[] = {
    80,0,80,0,85,0,90,0,90,60,60,60,120,0,120,0,120,0,120,120,120,150
};
static s32 sClayVersusTimers[] = {
    80,0,80,0,80,0,80,0,60,60,60,60,120,0,120,0,120,0,120,120,120,150
};
static s32 sClaySides[] = {
    1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,0,1
};

void RPDucSpawner::CreateInstance() {
    sInstance = new RPDucSpawner;
}

void RPDucSpawner::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPDucSpawner::RPDucSpawner() {
    mPatternFuncs[0] = &RPDucSpawner::Setup0;
    mPatternFuncs[1] = &RPDucSpawner::Setup1;
    mPatternFuncs[2] = &RPDucSpawner::Setup2;
    mPatternFuncs[3] = &RPDucSpawner::Setup3;
    mPatternFuncs[4] = &RPDucSpawner::Setup4;
    mPatternFuncs[5] = &RPDucSpawner::Setup5;
    mPatternFuncs[6] = &RPDucSpawner::Setup6;
    mPatternFuncs[7] = &RPDucSpawner::Setup7;
    mPatternFuncs[8] = &RPDucSpawner::Setup8;
    mPatternFuncs[9] = &RPDucSpawner::Setup9;
    mPatternFuncs[10] = &RPDucSpawner::PatternBalloonsB;
    mPatternFuncs[11] = &RPDucSpawner::PatternBalloonsA;
    mPatternFuncs[12] = &RPDucSpawner::PatternCircleTargets;
    mPatternFuncs[13] = &RPDucSpawner::PatternClay;
    mPatternFuncs[14] = &RPDucSpawner::PatternClayPairs;
    mPatternFuncs[15] = &RPDucSpawner::PatternUfosB;
    mPatternFuncs[16] = &RPDucSpawner::PatternUfosA;
    mPatternFuncs[17] = &RPDucSpawner::PatternDucksB;
    mPatternFuncs[18] = &RPDucSpawner::PatternDucksA;
    mPatternFuncs[19] = &RPDucSpawner::PatternCans;
}

RPDucSpawner::~RPDucSpawner() {}

void RPDucSpawner::Setup0() {
    s32 counts[] = {0, 15, 5, 3};
    mStep = 30;
    mTimer = 60;
    s32 waves = RPDucGame::sInstance->GetWaves();
    mRemaining = counts[waves];
    if (waves == 3) {
        RPDucScoreMgr::sInstance->ClearLastColors();
        mTimer = 60;
        mStep = 58;
    }
}

void RPDucSpawner::Setup1() {
    mStep = 0;
    mTimer = 30;
}

void RPDucSpawner::Setup2() {
    s32 counts[] = {0, 3, 9, 300, 18, 300};
    RPDucCircleTargetMgr::sInstance->ClearGridSlots();
    mStep = 0;
    mTimer = 40;
    s32 waves = RPDucGame::sInstance->GetWaves();
    mRemaining = counts[waves];
    if (waves == 1) {
        mTimer = 120;
    } else if (waves == 5 || waves == 3) {
        RPDucScoreMgr::sInstance->ClearCircleHits();
        unk20 = true;
        unk1C = 600;
    } else if (waves == 2) {
        mDelayedSpawn = true;
        mDelayedSpawnNum = 1;
        mDelayedSpawnTimer = 60;
        RPDucGame::sInstance->PlayRemoteAlertSound(1);
    }
}

void RPDucSpawner::Setup3() {
    mStep = 0;
    mTimer = 50;
    mRemaining = 22;
}

void RPDucSpawner::Setup4() {
    mStep = 260;
    mTimer = 290;
}

void RPDucSpawner::Setup5() {
    s32 counts[] = {0, 6, 4, 4, 2};
    mStep = 0;
    mTimer = 40;
    mRemaining = counts[RPDucGame::sInstance->GetWaves()];
}

void RPDucSpawner::Setup6() {
    s32 counts[] = {0, 6, 12, 4, 4};
    mStep = 0;
    mTimer = 60;
    s32 waves = RPDucGame::sInstance->GetWaves();
    mRemaining = counts[waves];
    if (RPDucAvatarMgr::sInstance->GetActiveNum() == 0) mRemaining = 0;
    if (waves == 3) RPDucAvatarMgr::sFinalRoundFlag = true;
    if (waves == 1) {
        mTimer = 120;
        mBonusWaveDone = false;
    }
}

void RPDucSpawner::Setup7() {
    s32 counts[] = {0, 5, 2, 2, 1};
    mStep = 0;
    mTimer = 20;
    RPDucGame* game = RPDucGame::sInstance;
    mRemaining = counts[game->GetWaves()];
    game->PlayRemoteAlertSound(0);
}

void RPDucSpawner::Setup8() {
    mStep = 0;
    mTimer = 15;
}

void RPDucSpawner::Setup9() {
    mStep = 0;
    mTimer = 20;
    s32 counts[] = {0, 3, 3, 1};
    mRemaining = counts[RPDucGame::sInstance->GetWaves()];
}

void RPDucSpawner::PatternBalloonsB() {
    if (++mStep == mTimer) {
        VEC3 pos(0.0f, -20.0f, 0.0f);
        s32 waves = RPDucGame::sInstance->GetWaves();
        if (waves == 3) {
            pos.x = SpawnerRandF32(-60.0f, -20.0f);
            pos.z = SpawnerRandF32(-40.0f, -10.0f);
            if (EGG::Screen::GetTVMode() == 0) pos.x *= 0.75f;
            SpawnTarget(2, pos, 0);
            pos.x = SpawnerRandF32(-20.0f, 20.0f);
            pos.z = SpawnerRandF32(-40.0f, -10.0f);
            if (EGG::Screen::GetTVMode() == 0) pos.x *= 0.75f;
            SpawnTarget(2, pos, 0x10);
            pos.x = SpawnerRandF32(20.0f, 60.0f);
            pos.z = SpawnerRandF32(-40.0f, -10.0f);
            if (EGG::Screen::GetTVMode() == 0) pos.x *= 0.75f;
            SpawnTarget(2, pos, 0x20);
        } else if (waves == 2) {
            pos.x = SpawnerRandF32(-50.0f, -30.0f);
            pos.z = SpawnerRandF32(-40.0f, -10.0f);
            if (EGG::Screen::GetTVMode() == 0) pos.x *= 0.75f;
            SpawnTarget(2, pos, 1);
            pos.x = SpawnerRandF32(-30.0f, -10.0f);
            pos.z = SpawnerRandF32(-40.0f, -10.0f);
            if (EGG::Screen::GetTVMode() == 0) pos.x *= 0.75f;
            SpawnTarget(2, pos, 1);
            pos.x = SpawnerRandF32(-10.0f, 10.0f);
            pos.z = SpawnerRandF32(-40.0f, -10.0f);
            if (EGG::Screen::GetTVMode() == 0) pos.x *= 0.75f;
            SpawnTarget(2, pos, 0x11);
            pos.x = SpawnerRandF32(10.0f, 30.0f);
            pos.z = SpawnerRandF32(-40.0f, -10.0f);
            if (EGG::Screen::GetTVMode() == 0) pos.x *= 0.75f;
            SpawnTarget(2, pos, 0x21);
            pos.x = SpawnerRandF32(30.0f, 50.0f);
            pos.z = SpawnerRandF32(-40.0f, -10.0f);
            if (EGG::Screen::GetTVMode() == 0) pos.x *= 0.75f;
            SpawnTarget(2, pos, 0x21);
        } else if (waves == 1) {
            u32 timers[] = {30,30,20,20,20,20,20,20,10,10,10,10,10,10,10};
            u32 args[] = {0x11,0x22,1,2,0x12,0x23,4,0x12,0x13,0x22,0x13,0x24,0x20,4,2};
            u32 lanes[] = {0,1,2,1,0,2,2,1,0,0,2,1,2,1,0};
            u32 skips[] = {0,0,0,1,0,0,0,0,1,0,0,1,0,0,0};
            if (RPDucGame::sInstance->GetMode() != 1 && skips[15 - mRemaining] == 1) {
                --mRemaining;
            } else {
                if (lanes[15 - mRemaining] == 0) pos.x = SpawnerRandF32(-40.0f, -15.0f);
                else if (lanes[15 - mRemaining] == 1) pos.x = SpawnerRandF32(-15.0f, 15.0f);
                else pos.x = SpawnerRandF32(15.0f, 40.0f);
                pos.z = SpawnerRandF32(-40.0f, -10.0f);
                if (EGG::Screen::GetTVMode() == 0) pos.x *= 0.75f;
                SpawnTarget(2, pos, args[15 - mRemaining]);
            }
            mTimer = timers[14 - mRemaining];
            if (RPDucGame::sInstance->GetMode() != 1) mTimer += (mTimer / 10) * 5;
            if (mRemaining == 10) {
                mDelayedSpawn = true;
                mDelayedSpawnNum = 0;
                mDelayedSpawnTimer = 60;
                RPDucGame::sInstance->PlayRemoteAlertSound(1);
            }
        }
        mStep = 0;
    }
}

void RPDucSpawner::PatternBalloonsA() {
    if (++mStep == mTimer) {
        VEC3 pos(0.0f, -20.0f, 0.0f);
        if (Duc_balloonPatternToggle != 0) pos.x = SpawnerRandF32(0.0f, 70.0f);
        else pos.x = SpawnerRandF32(-70.0f, 0.0f);
        pos.z = SpawnerRandF32(-50.0f, -10.0f);
        SpawnTarget(2, pos, 1);
        s32 toggle = Duc_balloonPatternToggle;
        s32 remaining = mRemaining;
        Duc_balloonPatternToggle = (toggle + 1) & 1;
        if (remaining > 24) mTimer = 30;
        else if (remaining > 16) mTimer = 20;
        else mTimer = 12;
        mStep = 0;
    }
}
void RPDucSpawner::PatternCircleTargets() {
    s32 index;
    u32 arg;
    s32 active = RPDucTargetRegistry::sInstance->CountActive();
    s32 waves = RPDucGame::sInstance->GetWaves();
    VEC3 pos(0.0f, 0.0f, 0.0f);
    if (++mStep == mTimer) {
        switch (waves) {
        case 5:
            if (active < 4) {
                index = SpawnerFindFreeSlot();
                arg = index << 4;
                s32 n = mRemaining % 20;
                if (n == 0) {
                    Duc_type3History[0] = (SpawnerRandU32() & 0x10) ? 16 : 14;
                    Duc_type3History[1] = (SpawnerRandU32() & 0x10) ? 6 : 8;
                    Duc_type3History[2] = SpawnerRandPair(2, 3);
                } else if (n == Duc_type3History[0]) arg |= 0x2000;
                else if (n == Duc_type3History[1]) arg |= 0x3000;
                else if (n == Duc_type3History[2]) arg |= 0x2000;
                else if (n == 1) arg |= 0x1000;
                SpawnTarget(3, pos, arg);
            }
            mStep = 0;
            mTimer = 1;
            break;
        case 4: {
            if (mRemaining == 18) {
                u32 rand = SpawnerRandU32() >> 8;
                Duc_type3History[0] = rand & 3;
                const u16 slots[] = {0, 5, 12, 17};
                Duc_type3History[1] = slots[Duc_type3History[0]];
                Duc_type3History[2] = rand & 1;
                Duc_type3History[3] = (SpawnerRandU32() & 3) + 13;
                arg = Duc_type3History[1] << 4;
            } else if (mRemaining == 12) {
                const u16 slots[] = {11, 6, 11, 6};
                u16 slot = slots[Duc_type3History[0]];
                Duc_type3History[2] = ~Duc_type3History[2] & 1;
                Duc_type3History[1] = slot;
                arg = Duc_type3History[1] << 4;
                Duc_type3History[3] = (SpawnerRandU32() & 3) + 8;
            } else if (mRemaining == 6) {
                const u16 slots[] = {12, 17, 0, 5};
                u16 slot = slots[Duc_type3History[0]];
                Duc_type3History[2] = ~Duc_type3History[2] & 1;
                Duc_type3History[1] = slot;
                Duc_type3History[3] = SpawnerRandPair(2, 3);
                arg = Duc_type3History[1] << 4;
            } else {
                arg = Duc_type3History[1] << 4;
                if (mRemaining == 1) arg |= 0x1000;
                else if (mRemaining == Duc_type3History[3]) {
                    if (mRemaining > 12) arg |= 0x3000;
                    else if (mRemaining > 6) arg |= 0x2000;
                    else if (RPDucGame::sInstance->GetMode() != 1) arg |= 0x3000;
                }
            }
            SpawnTarget(3, pos, arg | 1);
            if (Duc_type3History[2] != 0) --Duc_type3History[1];
            else ++Duc_type3History[1];
            mStep = 0;
            if (RPDucGame::sInstance->GetMode() != 1) mTimer = 14;
            else mTimer = 10;
            break;
        }
        case 3:
            if (active < 6) {
                index = SpawnerFindFreeSlot();
                arg = index << 4;
                s32 n = mRemaining % 20;
                if (n == 0) {
                    Duc_type3History[0] = SpawnerRandPair(16, 17);
                    Duc_type3History[1] = SpawnerRandPair(11, 12);
                    Duc_type3History[2] = SpawnerRandPair(8, 9);
                    Duc_type3History[3] = SpawnerRandPair(5, 6);
                    Duc_type3History[4] = SpawnerRandPair(2, 3);
                } else if (n == Duc_type3History[0]) arg |= 0x3000;
                else if (n == Duc_type3History[1]) arg |= 0x2000;
                else if (n == Duc_type3History[2]) arg |= 0x3000;
                else if (n == Duc_type3History[3]) arg |= 0x2000;
                else if (n == Duc_type3History[4]) arg |= 0x3000;
                else if (n == 1) arg |= 0x1000;
                SpawnTarget(3, pos, arg | 2);
            }
            mStep = 0;
            mTimer = 1;
            break;
        case 2:
            if (mRemaining == 9) {
                Duc_type3History[0] = (SpawnerRandU32() >> 4) & 1;
                Duc_type3History[1] = (SpawnerRandU32() & 0xFFFF) % 6 + 1;
                Duc_type3History[2] = (SpawnerRandU32() >> 4) & 1;
                Duc_type3History[3] = (Duc_type3History[1] - 1) / 3;
                Duc_type3History[4] = (6 - Duc_type3History[1]) % 3;
                Duc_type3History[5] = (SpawnerRandU32() >> 4) & 1;
            }
            if (Duc_type3History[2] != 0) {
                if (mRemaining == 6) Duc_type3History[0] = (Duc_type3History[0] == 6) + 6;
                else if (mRemaining == 3) Duc_type3History[0] = (Duc_type3History[0] == 12) + 12;
            }
            index = 0;
            do {
                arg = Duc_type3History[0] << 4;
                if (mRemaining == Duc_type3History[1]) arg |= 0x1000;
                else if ((mRemaining - 1) / 3 == Duc_type3History[3]) {
                    s32 n = (6 - mRemaining) % 3;
                    if (Duc_type3History[4] == 0) {
                        if (n == 1) {
                            if (Duc_type3History[5] != 0) arg |= 0x2000;
                            else arg |= 0x3000;
                        } else {
                            if (Duc_type3History[5] != 0) arg |= 0x3000;
                            else arg |= 0x2000;
                        }
                    } else if (Duc_type3History[4] == 1) {
                        if (n == 0) {
                            if (Duc_type3History[5] != 0) arg |= 0x2000;
                            else arg |= 0x3000;
                        } else {
                            if (Duc_type3History[5] != 0) arg |= 0x3000;
                            else arg |= 0x2000;
                        }
                    } else {
                        if (n == 0) {
                            if (Duc_type3History[5] != 0) arg |= 0x2000;
                            else arg |= 0x3000;
                        } else {
                            if (Duc_type3History[5] != 0) arg |= 0x3000;
                            else arg |= 0x2000;
                        }
                    }
                }
                SpawnTarget(3, pos, arg | 3);
                Duc_type3History[0] += 2;
            } while (++index < 3);
            mStep = 0;
            if (RPDucGame::sInstance->GetMode() != 1) mTimer = 30;
            else mTimer = 15;
            break;
        case 1:
            if (RPDucGame::sInstance->GetMode() != 1) {
                index = (SpawnerRandU32() & 0xFFFF) % 18;
                arg = index << 4;
                SpawnTarget(3, pos, arg | 0x1007);
                s32 next;
                do { next = (SpawnerRandU32() & 0xFFFF) % 18; } while (next == index);
                arg = next << 4;
                SpawnTarget(3, pos, arg | 0x2007);
                mRemaining = 0;
            } else if (RPDucGame::sInstance->GetMode() == 1) {
              if (mRemaining == 3) {
                s32 slot = (SpawnerRandU32() & 0xFFFF) % 18;
                Duc_type3History[0] = slot;
                s32 second;
                do { second = (SpawnerRandU32() & 0xFFFF) % 18; } while (second == Duc_type3History[0]);
                arg = second << 4;
                SpawnTarget(3, pos, arg | 0x2004);
                s32 third;
                do { third = (SpawnerRandU32() & 0xFFFF) % 18; }
                while (third == Duc_type3History[0] || third == second);
                arg = third << 4;
                SpawnTarget(3, pos, arg | 0x3004);
                mTimer = 50;
            } else if (mRemaining == 1) {
                arg = Duc_type3History[0] << 4;
                SpawnTarget(3, pos, arg | 0x1007);
              }
            }
            mStep = 0;
            break;
        }
    }
    if (waves == 5 || waves == 3) {
        if (--unk1C <= 0) mRemaining = 0;
        else if (unk1C == 1 && unk20) {
            const RPDucScoreMgr* pScoreMgr = RPDucScoreMgr::sInstance;
            s32 n = pScoreMgr->GetCircleHits(0);
            if (RPDucGame::sInstance->GetMode() == 1) n = pScoreMgr->GetCircleHits(1);
            s32 extra = (n - 12) * 10;
            extra = extra < 0 ? 0 : extra;
            extra = extra > 180 ? 180 : extra;
            unk1C += extra;
            unk20 = false;
        }
    }
}
void RPDucSpawner::PatternClay() {
    int remaining;
    if (++mStep >= mTimer) {
        VEC3 pos(100.0f, 30.0f, 150.0f);
        u32 arg = 0;
        if (mRemaining <= 12) arg = 1;
        pos.y = 30.0f;
        pos.z = 150.0f;
        if (sClaySides[mRemaining - 1] == 0) {
            pos.x = (SpawnerRandU32() & 4) ? 60.0f : -60.0f;
            if (EGG::Screen::GetTVMode() == 0) pos.x *= 0.75f;
            bool side = pos.x > 0.0f;
            remaining = mRemaining;
            Duc_clayPatternSide = side;
            if (remaining != 0) {
                RPDucTargetRegistry::sInstance->Spawn(0, pos, arg);
                --mRemaining;
            }
        } else if (sClaySides[mRemaining - 1] == 1) {
            pos.x = Duc_clayPatternSide == 0 ? 60.0f : -60.0f;
            if (EGG::Screen::GetTVMode() == 0) pos.x *= 0.75f;
            SpawnTarget(0, pos, arg);
        }
        if (mRemaining == 8) {
            mDelayedSpawn = true;
            mDelayedSpawnNum = 2;
            mDelayedSpawnTimer = 60;
            RPDucGame::sInstance->PlayRemoteAlertSound(1);
        }
        if (RPDucGame::sInstance->GetMode() != 1) mTimer = Duc_clayPatternTable[mRemaining];
        else mTimer = sClayVersusTimers[mRemaining];
        mStep = 0;
    }
}

void RPDucSpawner::PatternClayPairs() {
    if (++mStep == mTimer) {
        VEC3 pos(150.0f, 10.0f, 0.0f);
        SpawnTarget(0, pos, 1);
        pos.x = -150.0f;
        SpawnTarget(0, pos, 1);
        mStep = 0;
        mTimer -= 45;
    }
}

void RPDucSpawner::PatternUfosB() {
    if (++mStep == mTimer) {
        VEC3 pos(0.0f, 20.0f, -200.0f);
        u16 lane = SpawnerRandU32() % 3;
        f32 x = 0.0f;
        f32 y = 0.0f;
        switch (lane) {
        case 0:
            x = SpawnerRandF32(-0.5f, 0.5f);
            y = nw4r::math::FSqrt(1.0f - x * x);
            break;
        case 1:
            y = SpawnerRandF32(0.0f, 0.5f);
            x = nw4r::math::FSqrt(1.0f - y * y);
            break;
        case 2:
            y = SpawnerRandF32(0.0f, 0.5f);
            x = -nw4r::math::FSqrt(1.0f - y * y);
            break;
        }
        pos.x += x * 200.0f;
        pos.y += y * 200.0f;
        u32 arg = 0;
        s32 waves = RPDucGame::sInstance->GetWaves();
        switch (waves) {
        case 4:
            arg = 0;
            break;
        case 3:
            arg = mRemaining == 1 ? 8 : 0;
            break;
        case 2:
            arg = 0x10;
            if (mRemaining == 1) arg = 0x18;
            break;
        case 1:
            arg = 0x10;
            if (mRemaining == 1) arg = 0x18;
            break;
        }
        SpawnTarget(1, pos, arg);
        mStep = 0;
        s32 timers[] = {0,1,1,1,90,0};
        mTimer = timers[waves];
    }
}

void RPDucSpawner::PatternUfosA() {
    if (++mStep == mTimer) {
        s32 waves = RPDucGame::sInstance->GetWaves();
        RPDucAvatar* avatar;
        if (waves == 1) avatar = RPDucAvatarMgr::sInstance->PickFree();
        else avatar = RPDucAvatarMgr::sInstance->PickAny();
        if (avatar != NULL) {
            VEC3 pos(0.0f, 0.0f, -150.0f);
            u16 lane = SpawnerRandU32() % 3;
            f32 angle = 0.0f;
            f32 span = 20.0f;
            switch (lane) {
            case 0: angle = SpawnerRandF32(90.0f - 3.0f * span, 90.0f - span); break;
            case 1: angle = SpawnerRandF32(90.0f - span, 90.0f + span); break;
            case 2: angle = SpawnerRandF32(90.0f + span, 90.0f + 3.0f * span); break;
            }
            f32 y = nw4r::math::SinDeg(angle);
            f32 x = nw4r::math::CosDeg(angle);
            pos.x += x * 240.0f;
            pos.y += y * 240.0f;
            u32 arg = 0x20;
            s32 timer = 90;
            switch (waves) {
            case 4:
                arg = 0x20;
                timer = 80;
                if (mRemaining == 4 || mRemaining == 2) timer = 1;
                break;
            case 3: arg = 0x21; timer = 10; break;
            case 2:
                arg = 0x22;
                if (mRemaining == 1) arg |= 8;
                timer = 40;
                break;
            case 1: arg = 0x23; timer = 1; arg |= 8; break;
            }
            RPDucTarget* target;
            if (mRemaining == 0) target = NULL;
            else {
                target = RPDucTargetRegistry::sInstance->Spawn(1, pos, arg);
                --mRemaining;
            }
            if (target != NULL) {
                target->VF_0x9C(avatar);
                avatar->SetTargeted(true);
            }
            mTimer = timer;
        } else {
            if (RPDucAvatarMgr::sInstance->GetActiveNum() == 0) mRemaining = 0;
            if (waves == 1) mRemaining = 0;
        }
        if (waves == 2 && mRemaining == 4) {
            mDelayedSpawn = true;
            mDelayedSpawnNum = 4;
            mDelayedSpawnTimer = 60;
            RPDucGame::sInstance->PlayRemoteAlertSound(1);
        }
        mStep = 0;
    }
}

void RPDucSpawner::PatternDucksB() {
    if (++mStep == mTimer) {
        VEC3 pos(0.0f, -10.0f, -10.0f);
        s32 waves = RPDucGame::sInstance->GetWaves();
        if (waves == 4) {
            pos.x = SpawnerRandF32(-70.0f, 70.0f);
            SpawnTarget(4, pos, 0x100);
            mTimer = 10;
        } else if (waves == 3) {
            pos.x = SpawnerRandF32(-70.0f, 70.0f);
            SpawnTarget(4, pos, 0x100);
            mTimer = 1;
        } else if (waves == 2) {
            pos.x = SpawnerRandF32(-90.0f, 90.0f);
            SpawnTarget(4, pos, 0);
            mTimer = 20;
        } else if (waves == 1) {
            pos.x = SpawnerRandF32(-90.0f, 90.0f);
            u32 arg = 2;
            if (mRemaining == 1) arg |= 0x10;
            SpawnTarget(4, pos, arg);
            mTimer = 1;
            if (mRemaining == 1) mTimer = 100;
        }
        mStep = 0;
    }
}

void RPDucSpawner::PatternDucksA() {
    // NONMATCHING-DEBT: volatile (and the 0x18 offset: mMode is private) keeps the mode load out of
    // r0 so the hoisted 0x4330 conversion constant takes r0 (lis r0 before the sInstance load);
    // GetMode() gives the same code with mode in r0 and the constant in r5 (5 words). Tried:
    // GetMode() into a local/inline/ternary/switch/const&, BOOL/int flag, game pointer local.
    // mwcc_trace 2026-10-29: the volatile keeps mode a named node (r35, coloured after the 0x4330
    // constant r36, so mode r4 / constant r0); GetMode()'s load is copy-propagated into a temp
    // numbered above the constant (r38), which then takes r5. Also tried: mode assigned inside the
    // compare, mode/versus declaration order, static inline helpers returning the mode or the flag
    // (all 361/366).
    // Rule check (2026-10-29): blocked by vreg numbering. A once-used load whose use is in the
    // same flowgraph node is substituted by IRO_ExpressionPropagation, so it becomes a codegen
    // temporary numbered above every optimizer @temp (the hoisted 0x4330 constant included). Only a
    // named local that survives propagation is numbered below the constant.
    s32 mode = static_cast<const volatile RPDucGame*>(RPDucGame::sInstance)->mMode;
    bool versus = false;
    if (mode == 1) versus = true;
    if (++mStep == mTimer) {
        u32 arg = 0x300;
        VEC3 pos(0.0f, -10.0f, -10.0f);
        switch (mRemaining) {
        case 10: pos.x = SpawnerRandF32(0.0f,10.0f); break;
        case 9: pos.x = SpawnerRandF32(10.0f,20.0f); break;
        case 8: pos.x = SpawnerRandF32(-20.0f,-10.0f); break;
        case 7: pos.x = SpawnerRandF32(0.0f,10.0f); break;
        case 6: pos.x = SpawnerRandF32(-40.0f,-30.0f); arg |= 0x10; break;
        case 5: pos.x = SpawnerRandF32(-20.0f,-0.1f); break;
        case 4: pos.x = SpawnerRandF32(40.0f,60.0f); arg |= 0x10; break;
        case 3: pos.x = SpawnerRandF32(-60.0f,60.0f); arg |= 0x20; break;
        case 2: pos.x = SpawnerRandF32(40.0f,60.0f); arg |= 0x10; break;
        case 1: pos.x = SpawnerRandF32(-60.0f,-20.0f); arg |= 0x20; break;
        }
        if (EGG::Screen::GetTVMode() == 0) pos.x *= 0.75f;
        SpawnTarget(4, pos, arg);
        mStep = 0;
        mTimer = ((SpawnerRandU32() >> 8) & 15) + 5;
        if (versus) mTimer = ((SpawnerRandU32() >> 8) & 15) + 4;
        else if (mRemaining == 2) mRemaining = 0;
    }
}

void RPDucSpawner::PatternCans() {
    s32 waves = RPDucGame::sInstance->GetWaves();
    if (++mStep == mTimer) {
        u32 arg = 0;
        VEC3 pos(0.0f, 20.0f, 0.0f);
        if (waves == 3) {
            Duc_canPatternLast = (SpawnerRandU32() >> 4) & 1;
            pos.x = Duc_canPatternLast != 0 ? -150.0f : 150.0f;
            pos.y = 10.0f;
            arg = 0;
        } else if (waves == 2) {
            if (mRemaining == 3) {
                pos.x = Duc_canPatternLast != 0 ? 150.0f : -150.0f;
                pos.y = 10.0f;
                arg = 0;
                mTimer = 420;
            } else {
                pos.x = SpawnerRandF32(-30.0f,30.0f);
                pos.y = 1.0f;
                pos.z = 100.0f;
                arg = 1;
                mTimer = 210;
            }
            if (mRemaining == 1) {
                mDelayedSpawn = true;
                mDelayedSpawnNum = 3;
                mDelayedSpawnTimer = 60;
                RPDucGame::sInstance->PlayRemoteAlertSound(1);
            }
        } else if (waves == 1) {
            if (RPDucGame::sInstance->GetMode() != 1) {
                if (mRemaining == 3) {
                    pos.x = SpawnerRandF32(-30.0f,-20.0f);
                    pos.y = 100.0f;
                    pos.z = 0.0f;
                    arg = 2;
                    mTimer = 2;
                } else if (mRemaining == 2) {
                    pos.x = SpawnerRandF32(20.0f,30.0f);
                    pos.y = 100.0f;
                    pos.z = 0.0f;
                    arg = 2;
                    mTimer = 120;
                } else if (mRemaining == 1) {
                    pos.x = SpawnerRandF32(-20.0f,20.0f);
                    pos.y = 1.0f;
                    pos.z = 100.0f;
                    arg = 1;
                    mTimer = 120;
                }
            } else {
                if (mRemaining == 3) pos.x = SpawnerRandF32(-60.0f,-40.0f);
                else if (mRemaining == 2) pos.x = SpawnerRandF32(-15.0f,15.0f);
                else if (mRemaining == 1) pos.x = SpawnerRandF32(40.0f,60.0f);
                pos.y = 100.0f;
                pos.z = 0.0f;
                arg = 2;
                mTimer = 2;
            }
        }
        if (EGG::Screen::GetTVMode() == 0) pos.x *= 0.75f;
        SpawnTarget(5, pos, arg);
        mStep = 0;
    }
    if (waves == 2 && mRemaining == 2 && RPDucCanMgr::sInstance->GetActiveNum() == 0 && mStep < 390) mStep = 390;
}
void RPDucSpawner::SpawnSpecialDucks(s32 num) {
    VEC3 pos(0.0f, 0.0f, 0.0f);
    u32 color = (SpawnerRandU32() >> 4) & 1;
    VEC3 offset(0.0f, 0.0f, 0.0f);
    switch (num) {
    case 0:
        pos.x = (SpawnerRandU32() & 0x10) ? 220.0f : -220.0f;
        pos.y = SpawnerRandF32(30.0f, 50.0f);
        pos.z = -90.0f;
        RPDucTargetRegistry::sInstance->Spawn(4, VEC3(pos), 0x200);
        break;
    case 1:
        pos.x = (SpawnerRandU32() & 0x10) ? 220.0f : -220.0f;
        pos.y = SpawnerRandF32(30.0f, 50.0f);
        pos.z = -90.0f;
        RPDucTargetRegistry::sInstance->Spawn(4, VEC3(pos), 0x200);
        break;
    case 2:
        offset.x = SpawnerRandF32(-40.0f, 40.0f);
        if (EGG::Screen::GetTVMode() == 0) pos.x *= 0.75f;
        offset.y = SpawnerRandF32(-30.0f, 10.0f);
        pos.x = offset.x + 230.0f;
        pos.y = offset.y + 30.0f;
        pos.z = -85.0f;
        RPDucTargetRegistry::sInstance->Spawn(4, VEC3(pos), 0x201);
        pos.x = offset.x + -230.0f;
        pos.y = offset.y + 30.0f;
        pos.z = -95.0f;
        RPDucTargetRegistry::sInstance->Spawn(4, VEC3(pos), 0x201);
        break;
    case 3:
        offset.x = SpawnerRandF32(-40.0f, 40.0f);
        if (EGG::Screen::GetTVMode() == 0) pos.x *= 0.75f;
        offset.y = SpawnerRandF32(-20.0f, 20.0f);
        pos.x = offset.x + 230.0f;
        pos.y = offset.y + 30.0f;
        pos.z = -85.0f;
        RPDucTargetRegistry::sInstance->Spawn(4, VEC3(pos), (color << 4) | 0x202);
        pos.x = offset.x + -230.0f;
        pos.y = offset.y + 30.0f;
        pos.z = -95.0f;
        RPDucTargetRegistry::sInstance->Spawn(4, VEC3(pos), ((!color) << 4) | 0x202);
        break;
    case 4:
        offset.x = SpawnerRandF32(-40.0f, 40.0f);
        if (EGG::Screen::GetTVMode() == 0) pos.x *= 0.75f;
        offset.y = SpawnerRandF32(-20.0f, 20.0f);
        pos.x = offset.x + 230.0f;
        pos.y = offset.y + 30.0f;
        pos.z = -85.0f;
        RPDucTargetRegistry::sInstance->Spawn(4, VEC3(pos), ((color ? 2 : 1) << 4) | 0x203);
        pos.x = offset.x + -230.0f;
        pos.y = offset.y + 30.0f;
        pos.z = -95.0f;
        RPDucTargetRegistry::sInstance->Spawn(4, VEC3(pos), ((color ? 1 : 2) << 4) | 0x203);
        break;
    case 5: {
        s32 leader = RPDucScoreMgr::sInstance->GetLeader();
        u32 side = 1;
        if (leader == 0) side = 1;
        else if (leader == 1) side = 0;
        else if (leader == -1) side = (SpawnerRandU32() >> 4) & 1;
        pos.x = side ? 280.0f : -280.0f;
        pos.y = 0.0f;
        pos.z = -90.0f;
        RPDucTargetRegistry::sInstance->Spawn(4, VEC3(pos), ((color ? 2 : 1) << 4) | 0x203);
        pos.x = side ? 230.0f : -230.0f;
        pos.y = 50.0f;
        pos.z = -90.0f;
        RPDucTargetRegistry::sInstance->Spawn(4, VEC3(pos), ((color ? 1 : 2) << 4) | 0x203);
        break;
    }
    }
}

void RPDucSpawner::UpdateDelayedSpawn() {
    if (mDelayedSpawn) {
        if (--mDelayedSpawnTimer <= 0) {
            SpawnSpecialDucks(mDelayedSpawnNum);
            mDelayedSpawn = false;
        }
    }
}

void RPDucSpawner::Create() {
    mActive = false;
    mPattern = 3;
    mCount = 0;
    mRemaining = 0;
    mTimer = 0;
    mStep = 0;
    unk1C = 0;
    unk20 = false;
    mDelayedSpawn = false;
    mDelayedSpawnNum = 0;
    mDelayedSpawnTimer = 0;
}

void RPDucSpawner::Reset() {
    mActive = false;
    mPattern = 3;
    mCount = 0;
    mRemaining = 0;
    mTimer = 0;
    mStep = 0;
    unk1C = 0;
    unk20 = false;
    mDelayedSpawn = false;
    mBonusWaveDone = false;
    mDelayedSpawnNum = 0;
    mDelayedSpawnTimer = 0;
}

void RPDucSpawner::StartPattern(s32 pattern, s32 count) {
    mPattern = pattern;
    mCount = count;
    mRemaining = count;
    (this->*mPatternFuncs[pattern])();
    mActive = true;
}

void RPDucSpawner::AddCount(s32 count) {
    mCount += count;
    mRemaining = mCount;
    (this->*mPatternFuncs[mPattern])();
    mActive = true;
}

void RPDucSpawner::Calculate() {
    UpdateDelayedSpawn();
    if (mActive) {
        (this->*(mPatternFuncs + 10)[mPattern])();
        if (mRemaining == 0) mActive = false;
    }
}

void RPDucSpawner::TriggerBonusWave() {
    if (!mBonusWaveDone && RPDucGame::sInstance->GetMode() == 1) {
        mDelayedSpawn = true;
        mDelayedSpawnNum = 5;
        mDelayedSpawnTimer = 40;
        RPDucGame::sInstance->PlayRemoteAlertSound(1);
        mBonusWaveDone = true;
    }
}

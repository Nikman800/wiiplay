// Shooting Range Mii avatar pool (RPDucAvatarMgr): picking, line-up and the final-round flag.
// Unit 0x80252118-0x802527D8 (dtk/game_units.tsv). Functions in reverse address order.
// RPDucAvatarMgr::SpawnTarget, the file's last function, sits just before this unit's range.
#include <Pack/RPParty/RPDucScene/RPDucTargetMgr.h>

#include <Pack/RPParty/RPDucScene/RPDucAvatar.h>
#include <Pack/RPParty/RPDucScene/RPDucUtil.h>

#include <egg/core.h>

RPDucAvatarMgr* RPDucAvatarMgr::sInstance __attribute__((aligned(8)));
bool RPDucAvatarMgr::sFinalRoundFlag __attribute__((aligned(8)));

void RPDucAvatarMgr::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPDucTarget* RPDucAvatarMgr::AllocTarget() {
    return new RPDucAvatar();
}

RPDucAvatarMgr::~RPDucAvatarMgr() {}

void RPDucAvatarMgr::Reset() {
    sFinalRoundFlag = false;
    RPDucTargetMgr::Reset();
}

inline u32 AvatarPickRandom() {
    RP_randSeedX = RP_randSeedX * 0x41C64E6D + 12345;
    if (RP_randSeedY & 1) {
        RP_randSeedY ^= 0x11020;
    }
    RP_randSeedY >>= 1;
    return RP_randSeedX ^ RP_randSeedY;
}

RPDucAvatar* RPDucAvatarMgr::PickFree() {
    RPDucAvatar* candidates[16] = {};
    s32 num = 0;

    if (mActiveNum == 0) {
        return NULL;
    }

    RPDucAvatar** ppCand = candidates;
    RPDucTarget** pp = mppPool;
    do {
        if ((*pp)->mAge >= 0) {
            RPDucAvatar* pAvatar = (*pp)->AsAvatar();
            if (num < 16 && !pAvatar->mTargeted) {
                *ppCand++ = pAvatar;
                num++;
            }
        }
        pp++;
    } while (pp <= mppPoolEnd);

    if (num == 0) {
        return NULL;
    }

    return candidates[AvatarPickRandom() % num];
}

RPDucAvatar* RPDucAvatarMgr::PickAny() {
    RPDucAvatar* candidates[16] = {};
    RPDucAvatar** ppCand;
    s32 num = 0;

    if (mActiveNum == 0) {
        return NULL;
    }

    RPDucAvatar* pFree = PickFree();
    if (pFree != NULL) {
        return pFree;
    }

    ppCand = candidates;
    RPDucTarget** pp = mppPool;
    do {
        if ((*pp)->mAge >= 0) {
            RPDucAvatar* pAvatar = (*pp)->AsAvatar();
            if (num < 16) {
                *ppCand++ = pAvatar;
                num++;
            }
        }
        pp++;
    } while (pp <= mppPoolEnd);

    if (num == 0) {
        return NULL;
    }

    return candidates[AvatarPickRandom() % num];
}

nw4r::math::VEC3 RPDucAvatarMgr::CalcLinePosition(const RPDucAvatar* pAvatar) {
    bool wide = EGG::Screen::GetTVMode() == EGG::Screen::TV_MODE_WIDE;
    f32 left = wide ? -80.0f : -60.0f;
    f32 right = wide ? 80.0f : 60.0f;
    f32 spacing = wide ? 30.0f : 24.0f;

    s32 side = pAvatar->mAnim;
    s32 order = 0;
    s32 index = pAvatar->mIndex;
    RPDucTarget** pp = mppPool;
    do {
        if ((*pp)->mAge >= 0) {
            RPDucAvatar* pOther = (*pp)->AsAvatar();
            if (pAvatar != pOther && side == pOther->mAnim && index > pOther->mIndex) {
                order++;
            }
        }
        pp++;
    } while (pp <= mppPoolEnd);

    f32 x;
    if (side == 0) {
        x = left + order * spacing;
    } else {
        x = right - order * spacing;
    }
    return nw4r::math::VEC3(x, 0.0f, -15.0f);
}

bool RPDucAvatarMgr::IsLowest(const RPDucAvatar* pAvatar) {
    nw4r::math::_VEC3 pos = pAvatar->GetPos();
    RPDucTarget** pp = mppPool;
    do {
        if ((*pp)->mAge >= 0) {
            RPDucAvatar* pOther = (*pp)->AsAvatar();
            if (pAvatar != pOther) {
                nw4r::math::VEC3 otherPos = pOther->GetPos();
                if (pos.x > otherPos.x) {
                    return false;
                }
            }
        }
        pp++;
    } while (pp <= mppPoolEnd);

    return true;
}

bool RPDucAvatarMgr::AreAllGone() {
    RPDucTarget** pp = mppPool;
    do {
        if ((*pp)->mAge >= 0 && (*pp)->AsAvatar()->mState != 10) {
            return false;
        }
        pp++;
    } while (pp <= mppPoolEnd);

    return true;
}

RPDucTargetMgr* RPDucAvatarMgr::CreateInstance(s32 num) {
    RPDucAvatarMgr* pMgr = new RPDucAvatarMgr();
    sInstance = pMgr;
    pMgr->CreatePool(num);
    return pMgr;
}

// Wii Play: the manager file's last-defined function (lowest address of the unit)
RPDucTarget* RPDucAvatarMgr::SpawnTarget(const nw4r::math::VEC3& rPos, u32 arg) {
    return sInstance->Spawn(rPos, arg);
}

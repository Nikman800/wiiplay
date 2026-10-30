#include <Pack/RPParty/RPPartyCommon/RPColliderList.h>

#include <Pack/RPParty/RPPartyCommon/RPActor.h>
#include <Pack/RPParty/RPPartyCommon/RPBBox.h>

extern "C" void RP_BBox_reset2(RPBBox* pBox);

class RPColliderOwner {
public:
    virtual bool IsEnabled(IRPActorPriority* pOther) = 0;
};

template <typename T> inline T* ColliderOutput(T* pOutput, T* pLocal) {
    return pOutput != NULL ? pLocal : NULL;
}

s32 RPColliderList::CheckHit(IRPActorPriority* pSelf, u32 mask, const EGG::Vector3f& rPos,
                             f32 radius, const EGG::Vector3f& rPrevPos,
                             EGG::Vector3f* pPosOfs, EGG::Vector3f* pRotOfs, u16* pFlags) {
    RPBBox posBox;
    RPBBox rotBox;
    RPCollider* pCollider;
    RPCollider* pCurrent;
    s32 hitCount = 0;
    u16 flagsOut;

    RP_BBox_reset2(&posBox);
    RP_BBox_reset2(&rotBox);

    pCurrent = mpHead;
    flagsOut = 0;

    if (mask != 0) {
        while (pCurrent != NULL && (pCurrent->mMask & mask) == 0) {
            pCurrent = pCurrent->mpNext;
        }
    }

    EGG::Vector3f posOfs;
    EGG::Vector3f rotOfs;
    u16 flags;

    if (mask != 0) {
        while (pCurrent != NULL && (pCurrent->mMask & mask) == 0) {
            pCurrent = pCurrent->mpNext;
        }
    }
    while (pCurrent != NULL) {
        pCollider = pCurrent;
        if (pCurrent != NULL) {
            if (mask != 0) {
                do {
                    pCurrent = pCurrent->mpNext;
                    if (pCurrent == NULL) {
                        break;
                    }
                } while ((pCurrent->mMask & mask) == 0);
            } else {
                pCurrent = pCurrent->mpNext;
            }
        }

        if (pCollider->mpOwner != NULL) {
            flags = pFlags != NULL ? *pFlags : 0;
            if (pCollider->CheckHit(radius, rPos, rPrevPos,
                                    ColliderOutput(pPosOfs, &posOfs),
                                    ColliderOutput(pRotOfs, &rotOfs),
                                    ColliderOutput(pFlags, &flags)) &&
                static_cast<RPColliderOwner*>(pCollider->mpOwner)->IsEnabled(pSelf)) {
                if (pPosOfs != NULL) {
                    posBox.Extend(posOfs);
                }
                if (pRotOfs != NULL) {
                    rotBox.Extend(rotOfs);
                }
                if (pFlags != NULL) {
                    flagsOut |= flags;
                }
                hitCount++;
            }
        }
    }

    if (hitCount != 0) {
        if (pPosOfs != NULL) {
            *pPosOfs = posBox.GetSum();
        }
        if (pRotOfs != NULL) {
            *pRotOfs = rotBox.GetSum();
        }
        if (pFlags != NULL) {
            *pFlags = flagsOut;
        }
    }
    return hitCount;
}

// RPCollider::Draw (801C5E24) is a weak copy; its name is Draw__10RPColliderFv

void RPColliderList::DrawAll() {
    for (RPCollider* pCurrent = mpHead; pCurrent != NULL;) {
        RPCollider* pCollider = pCurrent;
        if (pCurrent != NULL) {
            pCurrent = pCurrent->mpNext;
        }
        pCollider->Draw();
    }
}

// RPCPadCursor: corner-bracket cursor for D-pad selection (cPadCursor.arc).
// Functions in reverse address order.
#include <Pack/RPParty/RPPartyCommon/RPCPadCursor.h>

#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPSystem/RPSysProjectLocal.h>

#include <egg/core.h>
#include <egg/gfxe.h>
#include <egg/math.h>

#include <cstring>

namespace {

inline void ClearTarget(RPCursorTarget& rTarget) {
    rTarget.mValid = false;
    std::strcpy(rTarget.mName, "");
}

inline void SetTargetLayout(RPCursorTarget& rTarget, nw4r::math::MTX34 mtx, f32 width, f32 height) {
    rTarget.mSize.width = width;
    rTarget.mSize.height = height;
    rTarget.mTrans.x = mtx._03;
    rTarget.mTrans.y = mtx._13;
    rTarget.mScale.x = mtx._00;
    rTarget.mScale.y = mtx._11;
}
inline void SetTargetInfo(RPCursorTarget& rTarget, nw4r::lyt::Pane* pPane) {
    rTarget.mValid = true;
    std::strcpy(rTarget.mName, pPane->GetName());
    s32 baseH = pPane->GetBasePositionH();
    s32 baseV = pPane->GetBasePositionV();
    f32 width = pPane->GetSize().width;
    f32 height = pPane->GetSize().height;
    SetTargetLayout(rTarget, pPane->GetGlobalMtx(), width, height);
    rTarget.mBasePosH = baseH;
    rTarget.mBasePosV = baseV;
}

inline void CopyTarget(RPCursorTarget& rDst, RPCursorTarget src) {
    rDst.mValid = src.mValid;
    std::strcpy(rDst.mName, src.mName);
    rDst.mSize = src.mSize;
    rDst.mScale = src.mScale;
    rDst.mTrans = src.mTrans;
    rDst.mBasePosH = src.mBasePosH;
    rDst.mBasePosV = src.mBasePosV;
}

inline RPCursorTarget GetTarget(const RPCursorTarget& rTarget) {
    RPCursorTarget target = rTarget;
    return target;
}

inline bool SameTarget(nw4r::lyt::Pane* pPane, const char* pName) {
    RPCursorTarget paneTarget;
    SetTargetInfo(paneTarget, pPane);
    return std::strcmp(pPane->GetName(), pName) == 0;
}

inline f32 AbsTimer(f32 timer) {
    if (timer > 0.0f) {
        return timer;
    }
    return -timer;
}

struct CPadCursorConst {
    static const f32 sCornerSwing;
};

} // namespace

nw4r::math::VEC2 RPCPadCursor::sDefaultSize(1000.0f, 1000.0f);
nw4r::math::VEC2 RPCPadCursor::sDefaultScale(1.0f, 1.0f);
nw4r::math::VEC3 RPCPadCursor::sDefaultTrans(100.0f, 100.0f, 0.0f);

static const char* sCornerNames[3][4] = {
    {"P_cPadCuTL_00", "P_cPadCuTR_00", "P_cPadCuBL_00", "P_cPadCuBR_00"},
    {"P_cPadCuTL_01", "P_cPadCuTR_01", "P_cPadCuBL_01", "P_cPadCuBR_01"},
    {"P_cPadCuTL_02", "P_cPadCuTR_02", "P_cPadCuBL_02", "P_cPadCuBR_02"},
};

void RPCPadCursor::Load(EGG::Archive* pArchive) {
    if (RPSysProjectLocal::instance()->getPack() == 2) {
        AttachArcFile(pArchive, "HealthCPadCursor.arc", mpResAccessor);
    } else {
        AttachArcFile(pArchive, "cPadCursor.arc", mpResAccessor);
    }
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "cPadCursor_00.brlyt");
    mpCornerPanes[0] = mpLayout->findPane("N_cPadCursor_00");
    mpCornerPanes[1] = mpLayout->findPane("N_cPadCursor_01");
    mpCornerPanes[2] = mpLayout->findPane("N_cPadCursor_02");
    if (IsMissingPanes()) {
        return;
    }
    mpCornerPanes[0]->SetLocationAdjust(true);
    mpCornerPanes[1]->SetLocationAdjust(true);
    mpCornerPanes[2]->SetLocationAdjust(true);
}

RPCPadCursor::RPCPadCursor() {
    mpCornerPanes[0] = NULL;
    mpCornerPanes[1] = NULL;
    mpCornerPanes[2] = NULL;
}

void RPCPadCursor::LoadWithParent(EGG::Archive* pArchive, EGG::Heap* pHeap) {
    if (RPSysProjectLocal::instance()->getPack() == 2) {
        AttachArcFile(pArchive, "HealthCPadCursor.arc", mpResAccessor);
    } else {
        AttachArcFile(pArchive, "cPadCursor.arc", mpResAccessor);
    }
    mpLayout = RPSysLayout::create(pHeap, mpResAccessor, "cPadCursor_00.brlyt");
    mpCornerPanes[0] = mpLayout->findPane("N_cPadCursor_00");
    mpCornerPanes[1] = mpLayout->findPane("N_cPadCursor_01");
    mpCornerPanes[2] = mpLayout->findPane("N_cPadCursor_02");
    if (IsMissingPanes()) {
        return;
    }
    mpCornerPanes[0]->SetLocationAdjust(true);
    mpCornerPanes[1]->SetLocationAdjust(true);
    mpCornerPanes[2]->SetLocationAdjust(true);
}

void RPCPadCursor::Reset() {
    if (IsMissingPanes()) {
        return;
    }
    SetActive(0);
    SelectStyle(0);
    RPCursorTarget& rDraw = mDrawTarget;
    rDraw.mSize.width = sDefaultSize.x;
    rDraw.mSize.height = sDefaultSize.y;
    rDraw.mScale = sDefaultScale;
    rDraw.mTrans = sDefaultTrans;
    mMoveTimer = 0;
    mMoveFrames = 35;
    mPrevActive = 0;
    unk14 = 1;
    ClearTarget(mTarget);
    ClearTarget(mPrevTarget);
    mFadeState = 1;
    mFadeTimer = 0;
    ClearTarget(mPrevTarget);
    ClearTarget(mTarget);
    ClearTarget(mNextTarget);
}

void RPCPadCursor::Calculate() {
    if (IsMissingPanes()) {
        return;
    }
    if (unk14 == 1) {
        if (mPrevActive == 1 && mActive == 0) {
            mFadeState = 2;
            mFadeTimer = 60;
        } else if (mActive == 1 && mPrevActive == 0) {
            mFadeState = 3;
            mFadeTimer = 0;
        }
    }
    if (mActive == 1 || (u32)(mFadeState - 2) <= 1) {
        UpdateCorners(mStyle);
    }
    mpLayout->calc();
    mPrevActive = mActive;
}

void RPCPadCursor::Draw() {
    if (IsMissingPanes()) {
        return;
    }
    if (mActive == 1 || mFadeState == 2) {
        mpLayout->draw();
    }
}

void RPCPadCursor::SetActive(s32 active) {
    if (IsMissingPanes()) {
        return;
    }
    if ((u32)active == 1) {
        mActive = 1;
        return;
    }
    mActive = 0;
}

BOOL RPCPadCursor::IsActive() const {
    return mActive == 1;
}

void RPCPadCursor::SelectStyle(s32 style) {
    mStyle = style;
    mpCornerPanes[0]->SetVisible(false);
    mpCornerPanes[1]->SetVisible(false);
    mpCornerPanes[2]->SetVisible(false);
    mpCornerPanes[mStyle]->SetVisible(true);
}

// 2026-10-28 (tracer pass): the size lfs already sit after all twelve copy loads in the
// pre-colouring PCode (mwcc_trace --pcode), so register colouring cannot fix it. No change:
// base positions read later / as u8, int, const or V-first locals (CSE keeps one early lbz),
// const f32& or const Size& size parameters (lvalues or by-value getters), width/height before
// the matrix parameter or a const MTX34& matrix (other blocks worse).
// 2026-10-29 (PCode-order pass): the base-position byte is the IR CommonSubs temporary
// (@18833). Variables get the lowest vregs and are coloured last, so the lbz takes r10. The
// original's lbz is coloured right after the mod/div chain (before copy word 0x98), so it is an
// expression temporary created after the matrix words. With opt_common_subs off (whole
// function; a diagnostic, not a fix) the backend still merges the two reads into one lbz temp
// and gives the original's `subf rX, rY, rX` pairing, but the frame and the rest move (478 words).
// The scheduler's pre-order already has the size lfs before the copy words; they lose the tie
// only on "unblocks" (their target stfs waits on every pane load through the worst-case alias).
// Tried with no gain: base reads after SetTargetLayout as direct stores (re-read, 367), every
// inline parameter order of (mtx, w, h, baseH, baseV) incl. u8 parameters (130-134), a Size
// member copy with u8 parameters (224), Size by value (520), a local MTX34 before/between the
// base reads (471), and base H/V split around the matrix copy (130: CSE still applies).
// 2026-10-29 (dead-store pass, aiming at a front-end-only kill of the byte CSE @18868): no gain.
// Stores to variables never kill (baseH/baseV pre-initialised, u8 basePos[2], references or
// pointers to the size locals, all copy-propagated or scalarised before CommonSubs: 130). A
// dead field store is removed only next to its overwrite (mValid false then true), and the live
// field store that remains kills both stages (mValid / mBasePosH / mBasePosV stored between the
// reads: 377-551). A local Size or VEC2 between the reads adds a stack temporary (355-385).
// FUNCTIONAL (matching abandoned, user decision 2026-10-29): behaviour equals the original; the
// differences below are register allocation / instruction order only (docs/NONMATCHING.md).
// NONMATCHING (130 of 834 words differ; frame and temporary slots match): only the
// scheduling in the inlined SetTargetInfo copies differs, as in RPCPadCursor_3. The original
// loads the pane size (lfs f5/f4) right after the matrix words 0/3/5/7, reloads the four
// floats right after they are stored, and stores the size before the other copy words; here
// the size loads come after all twelve copy loads. Tried (2026-10-28, on both units): a local
// Size (stack temporary, frame +0x40), Size/VEC2 locals or by-value/const& Size parameters,
// f32 locals before or after the base positions, every store order in the helper, float reads
// into locals, a local MTX34 copy, const MTX34, Size(w, h) and VEC2 assignment, and the
// opt_propagation / common_subs / dead_assignments / lifetimes / scheduling pragmas.
// The copy in IsValidTarget puts GetTarget(mTarget)'s 0x88 bytes below SameTarget's (an
// inline's named local rather than a temporary), as in the original.
inline bool IsValidTarget(const RPCursorTarget& rTarget) {
    RPCursorTarget target = rTarget;
    return target.mValid;
}
void RPCPadCursor::SetTarget(nw4r::lyt::Pane* pPane) {
    if (IsMissingPanes()) {
        return;
    }
    // Two unused targets: their constructors are the four hoisted 0.0f stores at the start.
    RPCursorTarget t0;
    RPCursorTarget t1;
    if (mActive == 1 || mFadeState == 2) {
        if (unk14 == 1) {
            if (GetTarget(mPrevTarget).mValid && mFadeState == 2) {
                SetTargetInfo(mNextTarget, pPane);
                CopyTarget(mDrawTarget, mPrevTarget);
                return;
            }
            if (GetTarget(mPrevTarget).mValid && !SameTarget(pPane, mPrevTarget.mName)) {
                mFadeTimer = 0;
                CopyTarget(mTarget, mPrevTarget);
                SetTargetInfo(mPrevTarget, pPane);
                CopyTarget(mDrawTarget, mTarget);
                mFadeState = 1;
                return;
            }
            if (mFadeState == 1) {
                SetTargetInfo(mPrevTarget, pPane);
                if (mFadeTimer > 0 && IsValidTarget(mTarget)) {
                    mDrawTarget = mTarget;
                    return;
                }
                mDrawTarget = mPrevTarget;
                return;
            }
            if (GetTarget(mPrevTarget).mValid && mFadeState == 3) {
                SetTargetInfo(mDrawTarget, pPane);
                SetTargetInfo(mPrevTarget, pPane);
                SetTargetInfo(mNextTarget, pPane);
                return;
            }
        }
        SetTargetInfo(mDrawTarget, pPane);
    }
}

void RPCPadCursor::UpdateCorners(s32 style) {
    if (++mMoveTimer < 0 || mMoveTimer > mMoveFrames) {
        mMoveTimer = 0;
    }
    s32 frame = mMoveTimer;
    s32 half = mMoveFrames / 2;
    if (frame > half) {
        frame = mMoveFrames - frame;
    }
    f32 offset = -CPadCursorConst::sCornerSwing +
                 2.0f * (CPadCursorConst::sCornerSwing * ((f32)frame / (f32)half));

    RPCursorTarget& rDraw = mDrawTarget;
    nw4r::math::VEC2 center;
    nw4r::math::VEC3 pos;
    if (rDraw.mBasePosH == 0) {
        center.x = rDraw.mSize.width / 2.0f * rDraw.mScale.x + rDraw.mTrans.x;
    } else if (rDraw.mBasePosH == 1) {
        center.x = rDraw.mTrans.x;
    } else {
        center.x = rDraw.mTrans.x + -(rDraw.mSize.width / 2.0f * rDraw.mScale.x);
    }
    if (rDraw.mBasePosV == 0) {
        center.y = rDraw.mTrans.y + -(rDraw.mSize.height / 2.0f * rDraw.mScale.y);
    } else if (rDraw.mBasePosV == 1) {
        center.y = rDraw.mTrans.y;
    } else {
        center.y = rDraw.mSize.height / 2.0f * rDraw.mScale.y + rDraw.mTrans.y;
    }

    for (s32 i = 0; i < 4; i++) {
        nw4r::lyt::Pane* pCorner = mpLayout->findPane(sCornerNames[style][i]);
        if (unk14 == 1) {
            pCorner->SetAlpha(CalcFadeAlpha());
        }
        if (i % 2 == 0) {
            pos.x = center.x + -(rDraw.mSize.width / 2.0f * rDraw.mScale.x + offset);
        } else {
            pos.x = center.x + (rDraw.mSize.width / 2.0f * rDraw.mScale.x + offset);
        }
        if (i / 2 == 0) {
            pos.y = center.y + (rDraw.mSize.height / 2.0f * rDraw.mScale.y + offset);
        } else {
            pos.y = center.y + -(rDraw.mSize.height / 2.0f * rDraw.mScale.y + offset);
        }
        pos.z = 0.0f;
        pCorner->SetTranslate(pos);
    }
}

s32 RPCPadCursor::CalcFadeAlpha() {
    if (mFadeState == 1) {
        s32 alpha = 255;
        if (mFadeTimer > 0) {
            s32 duration = 0;
            alpha = (f32)(u32)alpha * ((f32)(s32)AbsTimer((f32)mFadeTimer) / (f32)duration);
            --mFadeTimer;
        }
        return alpha;
    } else if (mFadeState == 2) {
        s32 alpha = 255;
        if (mFadeTimer > 0) {
            s32 duration = 60;
            alpha = (f32)(u32)alpha * ((f32)mFadeTimer / (f32)duration);
            --mFadeTimer;
        } else {
            mFadeState = 1;
            unk14 = 1;
            ClearTarget(mTarget);
            ClearTarget(mPrevTarget);
        }
        return alpha;
    } else if (mFadeState == 3) {
        s32 alpha = 255;
        if (mFadeTimer < 120) {
            s32 duration = 120;
            alpha = (f32)(u32)alpha * ((f32)mFadeTimer / (f32)duration);
            ++mFadeTimer;
        } else {
            mFadeState = 1;
            mFadeTimer = 0;
            CopyTarget(mPrevTarget, mNextTarget);
        }
        return alpha;
    }
    return 255;
}


namespace {

// The original copies the previous target into a local whose size field is an EGG::Vector2f:
// this unit carries that local type's weak destructor and EGG::Vector2f's, which RPCursorTarget
// (nw4r::lyt::Size) does not produce.
struct PointerTarget {
    bool mValid;
    char mName[100];
    EGG::Vector2f mSize;
    nw4r::math::VEC2 mScale;
    nw4r::math::VEC3 mTrans;
    u8 mBasePosH;
    u8 mBasePosV;
};

} // namespace

void RP_getPointerScreenPos(EGG::Vector2f& rScreenPos, const RPCPadCursor& rCursor) {
    rScreenPos.x = 0.0f;
    rScreenPos.y = 0.0f;

    PointerTarget copied;
    copied = reinterpret_cast<const PointerTarget&>(rCursor.mPrevTarget);
    if (copied.mValid) {
        rScreenPos.x =
            rCursor.mPrevTarget.mTrans.x / (f32)(u16)(EGG::Screen::GetSizeXMax() / 2);
        rScreenPos.y =
            rCursor.mPrevTarget.mTrans.y / (f32)(u16)(EGG::Screen::GetSizeYMax() / 2);
    }
}

const f32 CPadCursorConst::sCornerSwing = 2.0f;

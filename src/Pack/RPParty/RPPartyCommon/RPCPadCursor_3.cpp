#include <Pack/RPParty/RPPartyCommon/RPCPadCursor.h>
#include <Pack/RPParty/RPPartyCommon/RPCPadCursor_3.h>

#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytWindow.h>

#include <cstring>

namespace {

} // namespace

nw4r::math::VEC2 RPWCursor::sDefaultSize(1000.0f, 1000.0f);
nw4r::math::VEC2 RPWCursor::sDefaultScale(1.0f, 1.0f);
nw4r::math::VEC3 RPWCursor::sDefaultTrans(100.0f, 100.0f, 0.0f);

void UpdateWindow(RPWCursor& rCursor, s32 index);

namespace {

inline bool WindowUnavailable(nw4r::lyt::Window*& rWindow) {
    nw4r::lyt::Window** ppWindows = &rWindow;
    for (s32 i = 0; i < 1; i++) {
        if (ppWindows[i] == NULL) {
            return true;
        }
    }
    return false;
}

inline f32 AbsTimer(f32 timer) {
    if (timer > 0.0f) {
        return timer;
    }
    return -timer;
}

inline void ClearTarget(RPCursorTarget& rTarget) {
    rTarget.mValid = false;
    std::strcpy(rTarget.mName, "?????");
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

} // namespace

void RPWCursor::Load(EGG::Archive* pArchive) {
    AttachArcFile(pArchive, "cCursorCircle.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "cCursorCircle_00.brlyt");
    mpWindow = mpLayout->findWindow("W_cursor_00");
    if (WindowUnavailable(mpWindow)) {
        return;
    }
    mpLayout->findPane("N_cursor_00")->SetLocationAdjust(true);
}

RPWCursor::RPWCursor() {}

void RPWCursor::Reset() {
    if (WindowUnavailable(mpWindow)) {
        return;
    }
    mActive = 0;
    if (!WindowUnavailable(mpWindow)) {
        unk10 = 0;
        mpWindow->SetVisible(false);
        (&mpWindow)[unk10]->SetVisible(true);
    }
    RPCursorTarget& rDrawTarget = mDrawTarget;
    rDrawTarget.mSize.width = sDefaultSize.x;
    rDrawTarget.mSize.height = sDefaultSize.y;
    rDrawTarget.mScale = sDefaultScale;
    rDrawTarget.mTrans = sDefaultTrans;
    mMoveTimer = 0;
    mMoveFrames = 35;
    mPrevActive = 0;
    unk14 = 1;
    ClearTarget(mTarget);
    ClearTarget(mPrevTarget);
    nw4r::ut::Color color(0xffffffff);
    color.r = 255;
    color.g = 255;
    color.b = 255;
    color.a = 255;
    mFadeState = 1;
    mFadeTimer = 0;
    mpWindow->SetVtxColor(0, color);
    ClearTarget(mPrevTarget);
    ClearTarget(mTarget);
    ClearTarget(mNextTarget);
}

void RPWCursor::Calculate() {
    if (WindowUnavailable(mpWindow)) {
        return;
    }
    if (unk14 == 1) {
        if (mPrevActive == 1 && mActive == 0) {
            mFadeState = 2;
            mFadeTimer = 0;
        } else if (mActive == 1 && mPrevActive == 0) {
            mFadeState = 3;
            mFadeTimer = 0;
        }
    }
    if (mActive == 1 || (u32)(mFadeState - 2) <= 1) {
        ::UpdateWindow(*this, unk10);
    }
    mpLayout->calc();
    mPrevActive = mActive;
}

void RPWCursor::Draw() {
    if (WindowUnavailable(mpWindow)) {
        return;
    }
    if (mActive == 1 || mFadeState == 2) {
        mpLayout->draw();
    }
}

void RPWCursor::SetActive(u32 active) {
    if (active == 1) {
        mActive = 1;
        return;
    }
    mActive = 0;
}

// 2026-10-28 (tracer pass, tested on RPCPadCursor_2): the size lfs already sit after all twelve copy loads in the
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
// FUNCTIONAL (matching abandoned, user decision 2026-10-29): behaviour equals the original; the
// differences below are register allocation / instruction order only (docs/NONMATCHING.md).
// NONMATCHING (91.0%; 130 of 824 words, frame and temporary slots match): only the
// scheduling in the eight inlined SetTargetInfo copies differs. The original loads the pane
// size (lfs f5/f4) right after the matrix words 0/3/5/7, reloads the four floats right after
// they are stored, and stores the size before the other copy words; here the size loads come
// after all twelve copy loads. Tried (2026-10-28): a local Size (stack temporary, frame
// 0x8C0), Size/VEC2 locals or by-value/const& Size parameters, f32 locals before or after the
// base positions, every store order of size/trans/scale in the helper, float reads into locals,
// a local MTX34 copy, const MTX34, Size(w, h) and VEC2 assignment, opt_propagation /
// common_subs / dead_assignments / lifetimes / scheduling pragmas.
// The copy in IsValidTarget is what puts GetTarget(mTarget)'s 0x88 bytes below SameTarget's
// (an inline's named local rather than a temporary), as in the original.
inline bool IsValidTarget(const RPCursorTarget& rTarget) {
    RPCursorTarget target = rTarget;
    return target.mValid;
}
void RPWCursor::SetTarget(nw4r::lyt::Pane* pPane) {
    if (WindowUnavailable(mpWindow)) {
        return;
    }
    // Two targets constructed on entry and never read: the original constructs them too
    // (the mSize stores at sp+0x848 and sp+0x7C0 before the first branch).
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

void UpdateWindow(RPWCursor& rCursor, s32 index) {
    nw4r::math::VEC2 scale;
    nw4r::lyt::Size size(0.0f, 0.0f);
    if (++rCursor.mMoveTimer < 0 || rCursor.mMoveTimer > rCursor.mMoveFrames) {
        rCursor.mMoveTimer = 0;
    }
    s32 frame = rCursor.mMoveTimer;
    s32 half = rCursor.mMoveFrames / 2;
    if (frame > half) {
        frame = rCursor.mMoveFrames - frame;
    }
    f32 progress = (f32)frame / (f32)half;
    if (rCursor.unk14 == 1) {
        (&rCursor.mpWindow)[index]->SetAlpha(rCursor.CalcFadeAlpha());
    }
    RPCursorTarget& rDraw = rCursor.mDrawTarget;
    rDraw.mTrans.z = -30.0f;
    nw4r::math::VEC3 trans(&rDraw.mTrans.x);
    const nw4r::math::VEC2& rScale = (&rCursor.mpWindow)[index]->GetScale();
    scale.x = rScale.x;
    scale.y = rScale.y;
    nw4r::math::VEC2 center;
    if (rDraw.mBasePosH == 0) {
        trans.x = rDraw.mTrans.x + -(rDraw.mSize.width / 2.0f * rDraw.mScale.x) / scale.x;
    } else if (rDraw.mBasePosH == 1) {
        center.x = rDraw.mTrans.x;
    } else {
        trans.x = rDraw.mTrans.x + (rDraw.mSize.width / 2.0f * rDraw.mScale.x) / scale.x;
    }
    if (rDraw.mBasePosV == 0) {
        trans.y = rDraw.mTrans.y + -(rDraw.mSize.height / 2.0f * rDraw.mScale.y) / scale.y;
    } else if (rDraw.mBasePosV == 1) {
        center.y = rDraw.mTrans.y;
    } else {
        trans.y = rDraw.mTrans.y + (rDraw.mSize.height / 2.0f * rDraw.mScale.y) / scale.y;
    }
    (&rCursor.mpWindow)[index]->SetTranslate(trans);
    size = (&rCursor.mpWindow)[index]->GetSize();
    size.width = rDraw.mSize.width * rDraw.mScale.x / scale.x;
    size.height = rDraw.mSize.height * rDraw.mScale.y / scale.y;
    (&rCursor.mpWindow)[index]->SetSize(size);
}

s32 RPWCursor::CalcFadeAlpha() {
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
            s32 duration = 0;
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
        if (mFadeTimer < 0) {
            s32 duration = 0;
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

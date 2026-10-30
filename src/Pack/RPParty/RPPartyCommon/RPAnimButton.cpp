#include <Pack/RPParty/RPPartyCommon/RPAnimButton.h>
#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>

RPAnimButton::RPAnimButton() {
    for (int i = 0; i < ANIM_MAX; i++) {
        mpAnmObjs[i] = NULL;
    }
    mpPane = NULL;
    mpAnimPane = NULL;
    mCurrentAnim = -1;
    mInheritFrame = false;
    unkB4 = 1;
    mTexturePatternOnly = true;
    mDefaultAnim = 2;
    mDefaultFrame = 0;
}

void RPAnimButton::VF_0x8() {}
void RPAnimButton::VF_0xC() {}
void RPAnimButton::VF_0x10() {}
void RPAnimButton::VF_0x14() {}

void RPAnimButton::PlayDefaultAnim(RPSysLayout* pLayout) {
    if (mpAnmObjs[mDefaultAnim] != NULL) {
        mCurrentAnim = mDefaultAnim;
        pLayout->unbindAnmObj(mpAnmObjs[mDefaultAnim]);
        mpAnmObjs[mDefaultAnim]->setFrame(mDefaultFrame);
        mpAnmObjs[mDefaultAnim]->stop();
        if (mTexturePatternOnly) {
            RPSysLayout::forceBindAnmObjToPane(mpAnmObjs[mDefaultAnim], mpAnimPane, true);
        } else {
            RPSysLayout::bindAnmObjToPane(mpAnmObjs[mDefaultAnim], mpAnimPane, true);
        }
    }
}

bool RPAnimButton::HitTest(const nw4r::math::VEC2& rPos, RPSysLayout* pLayout) const {
    if (pLayout == NULL) {
        return false;
    }
    if (mpPane == NULL) {
        return false;
    }
    return pLayout->isInsideBounding(RPSysLayout::castToBounding(mpPane),
                                    static_cast<const EGG::Vector2f&>(rPos));
}

bool RPAnimButton::PlayAnim(s32 anim, RPSysLayout* pLayout) {
    if (mCurrentAnim != anim) {
        for (int i = 0; i < ANIM_MAX; i++) {
            if (mpAnmObjs[i] != NULL) {
                mpAnmObjs[i]->setFrame(0.0f);
                mpAnmObjs[i]->stop();
            }
        }
        s32 previous = mCurrentAnim;
        mCurrentAnim = anim;
        if (anim == 2 || anim == 6) {
            pLayout->unbindAnmObj(mpAnmObjs[anim]);
            mpAnmObjs[anim]->setPlayMode(RPSysFrameCtrl::EPlayMode_Reset);
            RPSysLytAnmObj* pObj = mpAnmObjs[anim];
            pObj->play(pObj->getFrameMax() - 1.0f);
            mpAnmObjs[anim]->setRate(-1.0f);
            if (mInheritFrame == true) {
                s32 frame = mpAnmObjs[anim]->getFrameCtrl()->getFrame();
                if (previous == 1) {
                    frame = mpAnmObjs[1]->getFrameCtrl()->getFrame();
                } else if (previous == 5) {
                    frame = mpAnmObjs[5]->getFrameCtrl()->getFrame();
                }
                mpAnmObjs[anim]->setFrame(frame);
            } else if (mStartFrames.Has(anim)) {
                mpAnmObjs[anim]->setFrame(mStartFrames.Get(anim));
            }
            if (mTexturePatternOnly) {
                RPSysLayout::forceBindAnmObjToPane(mpAnmObjs[anim], mpAnimPane, true);
            } else {
                RPSysLayout::bindAnmObjToPane(mpAnmObjs[anim], mpAnimPane, true);
            }
            return true;
        } else {
            pLayout->unbindAnmObj(mpAnmObjs[anim]);
            if (mStopFrames.Has(anim)) {
                mpAnmObjs[anim]->getFrameCtrl()->setLoopFrame(mStopFrames.Get(anim));
                mpAnmObjs[anim]->setPlayMode(RPSysFrameCtrl::EPlayMode_Loop);
            } else {
                mpAnmObjs[anim]->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
            }
            mpAnmObjs[anim]->restart();
            if (mInheritFrame == true) {
                s32 frame = 0;
                if (previous == 2 && anim == 1) {
                    frame = mpAnmObjs[2]->getFrameCtrl()->getFrame();
                } else if (previous == 6 && anim == 5) {
                    frame = mpAnmObjs[6]->getFrameCtrl()->getFrame();
                }
                mpAnmObjs[anim]->setFrame(frame);
            } else if (mStartFrames.Has(anim)) {
                mpAnmObjs[anim]->setFrame(mStartFrames.Get(anim));
            } else {
                mpAnmObjs[anim]->setFrame(0.0f);
            }
            if (mTexturePatternOnly) {
                RPSysLayout::forceBindAnmObjToPane(mpAnmObjs[anim], mpAnimPane, true);
            } else {
                RPSysLayout::bindAnmObjToPane(mpAnmObjs[anim], mpAnimPane, true);
            }
            if (unkB4 == 1) {
                RPSysLayout::becomeYoungestChild(mpAnimPane);
            }
            return true;
        }
    }
    return false;
}

void RPAnimButton::SetAnim(s32 idx, RPSysLytAnmObj* pAnmObj) {
    mpAnmObjs[idx] = pAnmObj;
}

void RPAnimButton::SetLayout(nw4r::lyt::Pane* pRootPane) {
    mpAnimPane = pRootPane;
}

void RPAnimButton::SetPane(nw4r::lyt::Pane* pPane) {
    mpPane = pPane;
}

nw4r::lyt::Pane* RPAnimButton::GetPane() const {
    return mpPane;
}

s32 RPAnimButton::GetCurrentAnimLength() const {
    if (mCurrentAnim != -1) {
        return mpAnmObjs[mCurrentAnim]->getFrameCtrl()->getEndFrame() -
               mpAnmObjs[mCurrentAnim]->getFrameCtrl()->getStartFrame();
    }
    return 0;
}

void RPAnimButton::VF_0x18() {}

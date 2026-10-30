#include <Pack/RPParty/RPPartyCommon/RPAButtonPrompt.h>

#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytResAccessor.h>
#include <Pack/RPSystem/RPSysResourceManager.h>

// RP_playerIndexToMask is C-style in the binary (no mangled name)
extern "C" {

u32 RP_playerIndexToMask(s32 player) {
    if (player == 0) {
        return 1 << 0;
    }
    if (player == 1) {
        return 1 << 1;
    }
    if (player == 2) {
        return 1 << 2;
    }
    if (player == 3) {
        return 1 << 3;
    }
    return 0xF;
}
}

void RPAButtonPrompt::Load(EGG::Archive* pArchive, EGG::Heap* pHeap) {
    RPSysLytResAccessor* pAccessor = RPSysLytResAccessor::create(pHeap);
    pAccessor->mountArchive(RPSysResourceManager::GetFileFromArchive(pArchive, "AButton.arc", NULL), NULL);

    mpLayout = RPSysLayout::create(pHeap, pAccessor, "AButton.brlyt");
    mpInAnmObj = mpLayout->createAnmObj(pHeap, pAccessor, "AButton_in_00.brlan");
    mpOutAnmObj = mpLayout->createAnmObj(pHeap, pAccessor, "AButton_out_00.brlan");
    mpBlinkAnmObj = mpLayout->createAnmObj(pHeap, pAccessor, "AButton_blink_00.brlan");
    mpRootPane = mpLayout->findPane("RootPane");
    mpButtonPane = mpLayout->findPane("N_AButton_00");
}

void RPAButtonPrompt::Reset() {
    if (mpRootPane->GetParent() != NULL) {
        mpRootPane->GetParent()->RemoveChild(mpRootPane);
    }
    mpLayout->reset();
    mpLayout->unbindAllAnmObj();
    mDrawEnable = false;
    mState = 0;
    mpParentPane = NULL;
}

void RPAButtonPrompt::Calculate() {
    switch (mState) {
    case 0:
        mDrawEnable = false;
        break;
    case 1:
        if (mpInAnmObj->isFinished()) {
            mpLayout->unbindAllAnmObj();
            RPSysLayout::bindAnmObjToPane(mpBlinkAnmObj, mpButtonPane, true);
            mpBlinkAnmObj->restart();
            mState = 2;
        }
        mpLayout->calc();
        mDrawEnable = true;
        break;
    case 2:
        mpLayout->calc();
        mDrawEnable = true;
        break;
    case 3:
        mpLayout->calc();
        mDrawEnable = false;
        if (mpOutAnmObj->isFinished()) {
            mState = 0;
            if (mpParentPane != NULL) {
                mpParentPane->RemoveChild(mpRootPane);
            }
        }
        break;
    }
    mpRootPane->SetVisible(mVisible);
}

void RPAButtonPrompt::Draw() {
    if (mDrawEnable) {
        mpLayout->draw();
    }
}

void RPAButtonPrompt::Show(nw4r::lyt::Pane* pParent, f32 x, f32 y) {
    if (pParent != NULL) {
        if (mpRootPane->GetParent() != NULL) {
            mpRootPane->GetParent()->RemoveChild(mpRootPane);
        }
        mpParentPane = pParent;
        pParent->AppendChild(mpRootPane);
    }
    mpRootPane->SetTranslate(nw4r::math::VEC2(x, y));
    mpRootPane->SetLocationAdjust(true);
    mpLayout->reset();
    mpLayout->unbindAllAnmObj();
    RPSysLayout::bindAnmObjToPane(mpInAnmObj, mpButtonPane, true);
    mpInAnmObj->restart();
    mState = 1;
    mVisible = true;
}

void RPAButtonPrompt::PlayOut() {
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpOutAnmObj);
    mpOutAnmObj->restart();
    mState = 3;
}

BOOL RPAButtonPrompt::IsShown() {
    return mState == 2;
}

BOOL RPAButtonPrompt::IsHidden() {
    return mState == 0;
}

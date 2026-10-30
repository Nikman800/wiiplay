#include <Pack/RPParty/RPPartyCommon/RPMessageWindow.h>

#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytResAccessor.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPKernel/RPSysSystem.h>
#include <Pack/RPSystem/RPSysResourceManager.h>

#include <Pack/RPParty/RPPartyCommon/RPAButtonPrompt.h>

#include <MSL/internal/wstring.h>

// Used by several layouts (RPSportsMsgPanel, the menus); kept under its ledger name.
extern "C" void Lyt_setPaneColorFromTable(nw4r::lyt::Pane* pPane, u32 player) {
    nw4r::lyt::Material* pMaterial = pPane->GetMaterial();
    const GXColor& rColor = RPSysLayout::getPlayerColor(player);
    GXColorS10 color;
    color.r = rColor.r;
    color.g = rColor.g;
    color.b = rColor.b;
    color.a = rColor.a;
    pMaterial->SetTevColor(1, color);
}

RPMessageWindow* RPMessageWindow::sInstance;

// Direct access to the text colours' alpha (the original writes the bytes, no accessor call)
class RPMessageWindowTextBox : public nw4r::lyt::TextBox {
public:
    u8 GetTextAlpha() const {
        return mTextColors[nw4r::lyt::TEXTCOLOR_TOP].a;
    }
    void SetTextAlpha(u8 alpha) {
        mTextColors[nw4r::lyt::TEXTCOLOR_TOP].a = alpha;
        mTextColors[nw4r::lyt::TEXTCOLOR_BOTTOM].a = alpha;
    }
};

static inline RPMessageWindowTextBox* RPMessageWindow_TextBox(RPSysLytTextBox* pTextBox) {
    return static_cast<RPMessageWindowTextBox*>(static_cast<nw4r::lyt::TextBox*>(pTextBox));
}

RPMessageWindow* RPMessageWindow::CreateInstance(EGG::Heap* pHeap) {
    if (sInstance == NULL) {
        return sInstance = new (pHeap, 4) RPMessageWindow(pHeap);
    }
    return NULL;
}

void RPMessageWindow::Load() {
    if (mpResAccessor != NULL) {
        return;
    }

    EGG::Heap* pHeap = RPSysSystem::instance()->getResourceHeap();
    mpResAccessor = RPSysLytResAccessor::create(pHeap);
    EGG::Archive* pArchive = RPSysResourceManager::instance()->GetStaticLocalArchive();
    mpResAccessor->mountArchive(
        RPSysResourceManager::GetFileFromArchive(pArchive, "messageWindow.arc"));

    mpLayouts[0] = RPSysLayout::create(pHeap, mpResAccessor, "messageWindowB_00.brlyt");
    mpLayouts[1] = RPSysLayout::create(pHeap, mpResAccessor, "messageWindowW_00.brlyt");
    mpAnmObjs[0][0] =
        mpLayouts[0]->createAnmObj(pHeap, mpResAccessor, "messageWindowB_00_in_00.brlan");
    mpAnmObjs[0][1] =
        mpLayouts[0]->createAnmObj(pHeap, mpResAccessor, "messageWindowB_00_out_00.brlan");
    mpAnmObjs[1][0] =
        mpLayouts[1]->createAnmObj(pHeap, mpResAccessor, "messageWindowW_00_in_00.brlan");
    mpAnmObjs[1][1] =
        mpLayouts[1]->createAnmObj(pHeap, mpResAccessor, "messageWindowW_00_out_00.brlan");
    mpLayouts[0]->bindAnmObj(mpAnmObjs[0][0]);
    mpLayouts[1]->bindAnmObj(mpAnmObjs[1][0]);

    for (int i = 0; i < WINDOW_NUM; i++) {
        mpTexWindPanes[i] = mpLayouts[i]->findPane("N_texWind_00");
        mpPosPanes[i][0] = mpLayouts[i]->findPane("N_twPosition_00");
        mpPosPanes[i][1] = mpLayouts[i]->findPane("N_twPosition_01");
        mpPosPanes[i][2] = mpLayouts[i]->findPane("N_twPosition_02");
        mpTextBoxes[i] = mpLayouts[i]->findTextBox("T_message_00");
        mTextAlpha[i] = RPMessageWindow_TextBox(mpTextBoxes[i])->GetTextAlpha();
    }

    mpAButtonPrompt->Load(pArchive, pHeap);
}

void RPMessageWindow::Reset() {
    mFlags = 0;
    mWindow = 0;
    unk54 = 1;
    mAutoCloseFrames = 0;
    mAutoCloseTimer = 0;
    unk5C = 120;
    mFadeTimer = 0;
    mTextTimer = 0;
    mText[0] = L'\0';
    mNextText[0] = L'\0';
    mpMessage = NULL;
    mGroup = 0;
    mIndex = 0;
    mpNextMessage = NULL;
    mNextGroup = 0;
    mNextIndex = 0;
    unk27E = true;

    if (mpResAccessor != NULL) {
        for (int i = 0; i < WINDOW_NUM; i++) {
            mpLayouts[i]->unbindAllAnmObj();
            mpLayouts[i]->bindAnmObj(mpAnmObjs[i][0]);
            mpAnmObjs[i][0]->restart();
            mpAnmObjs[i][0]->stop();
        }
        mpAButtonPrompt->Reset();
    }
}

void RPMessageWindow::Calculate() {
    if (!(mFlags & 1)) {
        return;
    }
    if (mpResAccessor == NULL) {
        return;
    }

    if (mFlags & 2) {
        if (mpAnmObjs[mWindow][0]->isFinished()) {
            if (unk27E) {
                if (mpAButtonPrompt->IsShown()) {
                    mFlags = (mFlags & ~2) | 8;
                }
            } else {
                mFlags = (GetFlags() & ~2) | 8;
            }
        }
    } else if (mFlags & 4) {
        if (mpAnmObjs[mWindow][1]->isFinished()) {
            if (unk27E) {
                if (mpAButtonPrompt->IsHidden()) {
                    mFlags = 0;
                }
            } else {
                mFlags = 0;
            }
        }
    } else if (mFlags & 0x10) {
        u8 alpha = mTextTimer / 10.0f * mTextAlpha[mWindow];
        RPMessageWindow_TextBox(mpTextBoxes[mWindow])->SetTextAlpha(alpha);
        mTextTimer += RPSysSystem::instance()->getFrameRate();
        if (mTextTimer > 10) {
            mTextTimer = 10;
            if (unk27E) {
                if (mpAButtonPrompt->IsShown()) {
                    mTextTimer = 0;
                    mFlags = (mFlags & ~0x10) | 8;
                }
            } else {
                mTextTimer = 0;
                mFlags = (mFlags & ~0x10) | 8;
            }
        }
    } else if (mFlags & 0x20) {
        u8 alpha = (10 - mTextTimer) / 10.0f * mTextAlpha[mWindow];
        RPMessageWindow_TextBox(mpTextBoxes[mWindow])->SetTextAlpha(alpha);
        mTextTimer += RPSysSystem::instance()->getFrameRate();
        if (mTextTimer > 10) {
            mTextTimer = 10;
            if (unk27E) {
                if (mpAButtonPrompt->IsHidden()) {
                    mTextTimer = 0;
                    mFlags = (mFlags & ~0x20) | 0x10;
                    mText[0] = L'\0';
                    wcscpy(mText, mNextText);
                    mNextText[0] = L'\0';
                    mpMessage = mpNextMessage;
                    mGroup = mNextGroup;
                    mIndex = mNextIndex;
                    mpNextMessage = NULL;
                    mNextGroup = 0;
                    mNextIndex = 0;
                    mpAButtonPrompt->Show(
                                           mpLayouts[mWindow]->findPane("N_ABuPosition_00"), 0.0f,
                                           0.0f);
                }
            } else {
                mTextTimer = 0;
                mFlags = (mFlags & ~0x20) | 0x10;
                mText[0] = L'\0';
                wcscpy(mText, mNextText);
                mNextText[0] = L'\0';
                mpMessage = mpNextMessage;
                mGroup = mNextGroup;
                mIndex = mNextIndex;
                mpNextMessage = NULL;
                mNextGroup = 0;
                mNextIndex = 0;
            }
        }
    } else if (mFlags & 0x40) {
        mFadeTimer += RPSysSystem::instance()->getFrameRate();
        if (mFadeTimer > 10) {
            mFadeTimer = 0;
            mFlags = (mFlags & ~0x40) | 0x20;
            if (unk27E) {
                mpAButtonPrompt->PlayOut();
            }
        }
    } else if (mFlags & 0x80) {
        mFadeTimer += RPSysSystem::instance()->getFrameRate();
        if (mFadeTimer > 10) {
            mFadeTimer = 0;
            mFlags = (mFlags & ~0x80) | 4;
            mpLayouts[mWindow]->unbindAllAnmObj();
            mpLayouts[mWindow]->bindAnmObj(mpAnmObjs[mWindow][1]);
            mpAnmObjs[mWindow][1]->restart();
            if (unk27E) {
                mpAButtonPrompt->PlayOut();
            }
        }
    } else if (!unk27E && mAutoCloseFrames != 0) {
        mAutoCloseTimer += RPSysSystem::instance()->getFrameRate();
        if (mAutoCloseTimer > mAutoCloseFrames) {
            if (Close()) {
                mAutoCloseTimer = 0;
            }
        }
    }

    if (mText[0] != L'\0') {
        mpTextBoxes[mWindow]->setText(mText, 0);
    }
    if (mpMessage != NULL) {
        mpTextBoxes[mWindow]->setMessage(mpMessage, mGroup,
                                         mIndex, 0);
    }
    mpLayouts[mWindow]->calc();
    mpAButtonPrompt->Calculate();
}

void RPMessageWindow::Draw() {
    if ((mFlags & 1) && mpResAccessor != NULL) {
        mpLayouts[mWindow]->draw();
        mpAButtonPrompt->Draw();
    }
}

RPMessageWindow::RPMessageWindow(EGG::Heap* pHeap) : mpHeap(pHeap) {
    mFlags = 0;
    Reset();
    mpResAccessor = NULL;
    for (int i = 0; i < WINDOW_NUM; i++) {
        mpLayouts[i] = NULL;
        mpAnmObjs[i][0] = NULL;
        mpAnmObjs[i][1] = NULL;
        mpTexWindPanes[i] = NULL;
        mpPosPanes[i][0] = NULL;
        mpPosPanes[i][1] = NULL;
        mpPosPanes[i][2] = NULL;
        mpTextBoxes[i] = NULL;
        mTextAlpha[i] = 0;
    }
    mpAButtonPrompt = new (pHeap, 4) RPAButtonPrompt;
}

RPMessageWindow::~RPMessageWindow() {}

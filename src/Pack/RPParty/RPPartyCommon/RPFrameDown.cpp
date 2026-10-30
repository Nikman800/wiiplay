#include <Pack/RPParty/RPPartyCommon/RPFrameDown.h>

#include <Pack/RPAudio/RPSndAudioMgr.h>
#include <Pack/RPKernel/RPSysController.h>
#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPKernel/RPSysMessage.h>
#include <Pack/RPSystem/RPSysResourceManager.h>
#include <egg/gfxe/eggScreen.h>

const char* RPFrameDown::sBackButtonNames[] = {"B_backButton_00", "B_backButton_01"};
const char* RPFrameDown::sArcName = "backCommon.arc";
const char* RPFrameDown::sLayoutName = "backCommon_00.brlyt";
const char* RPFrameDown::sAnimNames[] = {
    "backCommon_00_outFrame_00.brlan",
    "backCommon_00_inFrame_00.brlan",
    "backCommon_00_activeBack_00.brlan",
    "backCommon_00_pushBack_00.brlan",
};
const char* RPFrameDown::sCaptionName = "T_caption_01";
// Seed the original string-pool order without retaining a writable pointer.
static const char* const sFramePaneName = "N_frameDown_00";
static const char* sCursorPaneName = "B_backButton_00";
static const u16 sCaptionMessages[][2] = {{450, 0}, {450, 1}};

extern "C" EGG::Vector2f Lyt_getPaneGlobalPos(const nw4r::lyt::Pane* pPane) DECOMP_DONT_INLINE;

// NONMATCHING-DEBT: PLAY_PANE/REVERSE_PANE kept as macros. As inline members
// RPFrameDown::PlayPane(int, const char*)/ReversePane(const char*) Calculate is 159/377 words:
// the pane names become argument temporaries computed ahead of each sequence.
#define PLAY_PANE(index, name)                                                   \
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[index], mpLayout->findPane(name), true); \
    mpAnmObjs[index]->setFrame(0.0f);                                             \
    mpAnmObjs[index]->restart();                                                 \
    mpAnmObjs[index]->setRate(1.0f);                                              \
    mpAnmObjs[index]->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime)

#define REVERSE_PANE(name)                                                       \
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[2], mpLayout->findPane(name), true);     \
    mpAnmObjs[2]->setFrame(0.0f);                                                 \
    {                                                                           \
        RPSysLytAnmObj* pAnmObj = mpAnmObjs[2];                                   \
        pAnmObj->play(pAnmObj->getFrameMax() - 1.0f);                              \
    }                                                                           \
    mpAnmObjs[2]->setRate(-1.0f);                                                 \
    mpAnmObjs[2]->setPlayMode(RPSysFrameCtrl::EPlayMode_Reset)

RPFrameDown::RPFrameDown(s32 type) : mType(type) {}

RPFrameDown::~RPFrameDown() {}

void RPFrameDown::Load(EGG::Archive* pArchive) {
    AttachArcFile(pArchive, sArcName, mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, sLayoutName);
    for (u32 i = 0; i < 4; i++) {
        mpAnmObjs[static_cast<u8>(i)] =
            mpLayout->createAnmObj(NULL, mpResAccessor,
                                   sAnimNames[static_cast<u8>(i)]);
    }
    void* pMessage = RPSysResourceManager::GetMessageResource("common_message.bmg");
    mpMessage = new RPSysMessage(pMessage, NULL);
    RPSysLytTextBox* pTextBox = mpLayout->findTextBox(sCaptionName);
    pTextBox->setMessage(mpMessage, sCaptionMessages[mType][0], sCaptionMessages[mType][1], 0);
}

void RPFrameDown::Reset() {
    mState = 0;
    mpLayout->unbindAllAnmObj();
    PLAY_PANE(1, "N_frameDown_00");
    mpAnmObjs[1]->stop();
    mRequest = 0;
    unk28 = 0;
}

void RPFrameDown::Calculate() {
    if (mRequest != mState) {
        switch (mRequest) {
        case 0:
            mState = 0;
            mpLayout->unbindAllAnmObj();
            PLAY_PANE(1, "N_frameDown_00");
            mpAnmObjs[1]->stop();
            break;
        case 1:
            mState = 1;
            mpLayout->unbindAllAnmObj();
            PLAY_PANE(1, "N_frameDown_00");
            break;
        case 2:
            mState = 2;
            mpLayout->unbindAllAnmObj();
            PLAY_PANE(0, "N_frameDown_00");
            mpAnmObjs[0]->stop();
            break;
        case 3:
            PlayHover();
            break;
        case 4:
            PlayUnhover();
            break;
        case 5:
            PlayPressed();
            break;
        case 6:
            mState = 6;
            mpLayout->unbindAllAnmObj();
            PLAY_PANE(0, "N_frameDown_00");
            break;
        }
    }
    switch (mState) {
    case 0:
        break;
    case 1:
        if (mpAnmObjs[1]->isFinished()) {
            mRequest = 2;
        }
        break;
    case 2:
    case 3:
        break;
    case 4:
        if (mpAnmObjs[2]->isFinished()) {
            mRequest = 2;
        }
        break;
    case 5:
        if (mpAnmObjs[3]->isFinished()) {
            mRequest = 2;
        }
        break;
    case 6:
        if (mpAnmObjs[0]->isFinished()) {
            mRequest = 0;
        }
        break;
    }
    mpLayout->calc();
}

void RPFrameDown::Draw() {
    mpLayout->draw();
}

void RPFrameDown::VF_0x18() {}

bool RPFrameDown::IsIdle() const {
    return mState == 2;
}

bool RPFrameDown::HitTestBackButton(const nw4r::math::VEC2& rPos) const {
    for (u32 i = 0; i < 2; i++) {
        RPSysLytBounding* pBounding = RPSysLayout::castToBounding(
            mpLayout->findPane(sBackButtonNames[static_cast<u16>(i)]));
        if (mpLayout->isInsideBounding(pBounding, static_cast<const EGG::Vector2f&>(rPos))) {
            return true;
        }
    }
    return false;
}

bool RPFrameDown::IsIdleOrHovered() const {
    return static_cast<u32>(mState - 2) <= 1;
}

void RPFrameDown::RequestIn() {
    if (mState == 0) {
        mRequest = 1;
    }
}

void RPFrameDown::RequestHover(s32 controller) {
    if (mState == 2) {
        unk28 = controller;
        mRequest = 3;
    }
}

void RPFrameDown::RequestUnhover() {
    if (mState == 3) {
        mRequest = 4;
    }
}

void RPFrameDown::RequestPressed() {
    if (mState != 2 && mState != 3) {
        return;
    }
    mRequest = 5;
}

extern "C" EGG::Vector2f Lyt_getPaneGlobalPos(const nw4r::lyt::Pane* pPane) {
    return EGG::Vector2f(pPane->GetGlobalMtx()._03, pPane->GetGlobalMtx()._13);
}

void RPFrameDown::PlayHover() {
    mState = 3;
    mpLayout->unbindAllAnmObj();
    PLAY_PANE(2, "P_captionDown_00");
    PLAY_PANE(2, "P_captionDown_01");
    PLAY_PANE(2, "T_caption_01");
    PLAY_PANE(2, "P_backButton_01");
    PLAY_PANE(2, "P_backButton_00");
    PLAY_PANE(2, "P_backMark_00");
    RPSndAudioMgr* pAudioMgr = RPSndAudioMgr::instance();
    EGG::Vector2f pos = Lyt_getPaneGlobalPos(mpLayout->findPane(sCursorPaneName));
    pos.x /= static_cast<f32>(static_cast<u16>(static_cast<u32>(EGG::Screen::GetSizeXMax()) >> 1));
    pos.y /= static_cast<f32>(static_cast<u16>(static_cast<u32>(EGG::Screen::GetSizeYMax()) >> 1));
    pAudioMgr->startPointCursorSe("RP_COMMON_SE_KEY_CURSOR_01", pos, 0.5f, 0);
    static_cast<RPSysCoreController*>(EGG::CoreControllerMgr::instance()->getNthController(unk28))
        ->rumble(RPSysCoreController::ERumblePattern_Cursor);
}

void RPFrameDown::PlayUnhover() {
    mState = 4;
    mpLayout->unbindAllAnmObj();
    REVERSE_PANE("P_captionDown_00");
    REVERSE_PANE("P_captionDown_01");
    REVERSE_PANE("T_caption_01");
    REVERSE_PANE("P_backButton_01");
    REVERSE_PANE("P_backButton_00");
    REVERSE_PANE("P_backMark_00");
}

void RPFrameDown::PlayPressed() {
    // Preserve literal-pool order before the signed frame conversion constant.
    f32 startFrame = 0.0f;
    f32 forwardRate = 1.0f;
    f32 reverseRate = -1.0f;
    mState = 5;
    mpLayout->unbindAllAnmObj();
    PLAY_PANE(3, "P_captionDown_00");
    PLAY_PANE(3, "P_captionDown_01");
    PLAY_PANE(3, "T_caption_01");
    PLAY_PANE(3, "P_backButton_01");
    PLAY_PANE(3, "P_backButton_00");
    PLAY_PANE(3, "P_backMark_00");
    RPSndAudioMgr* pAudioMgr = RPSndAudioMgr::instance();
    pAudioMgr->startSystemSeWithRemote("RP_COMMON_SE_KEY_BACK_01_TVSP", 1);
    pAudioMgr->startSystemSeWithRemote("RP_COMMON_SE_KEY_BACK_01_CTSP", 1);
}

// Game-select board (RPSelectGameLyt): locked-game balloons. Unit 0x801FE3A8-0x801FE73C.
// Functions in reverse address order.
#include <Pack/RPParty/RPPartyCommon/RPSelectGameLyt.h>
#include <Pack/RPParty/RPPartyCommon/RPSelectScene.h>
#include <Pack/RPSystem/RPSysPlayer.h>

#include <Pack/RPAudio.h>
#include <Pack/RPKernel.h>
#include <Pack/RPSystem.h>
#include <Pack/RPUtility.h>

extern "C" {
// Still named by their ledger placeholders (their units are not linked yet)
s32 Menu_slotToGameId(s32 slot);
EGG::Vector2f Lyt_getPaneGlobalPos(const nw4r::lyt::Pane* pPane);
EGG::Vector2f EGG_Vector2f_operator_plus(const EGG::Vector2f* pLhs, const EGG::Vector2f* pRhs);
}

// ---- 0x801FE3A8-0x801FE73C: locked balloons ----

void RPSelectGameLyt::EnterLockedIdle() {
    mpExpPanes[mCurLocked]->SetVisible(false);
}

void RPSelectGameLyt::CalcLockedIdle() {
    if (mLockedRequest[mCurLocked]) {
        mpLockedStates[mCurLocked]->ChangeState(1);
    }
}

void RPSelectGameLyt::EnterLockedStart() {
    mpExpPanes[mCurLocked]->SetVisible(true);
    RPSysLayout::becomeYoungestChild(mpIconPanes[mCurLocked][0]);
    RPSysLayout::becomeYoungestChild(mpGamePosPanes[mCurLocked]);
    mLockedRequest[mCurLocked] = false;
    mpExpPanes[mCurLocked]->UnbindAllAnimation(true);
    RPSysLayout::forceBindAnmObjToPane(mpExpAnmObjs[mCurLocked][0], mpExpPanes[mCurLocked], true);
    mpExpAnmObjs[mCurLocked][0]->restart();
    RPSndAudioMgr::instance()->startSystemSe("RP_COMMON_SE_Balloon_OPEN_01");
}

void RPSelectGameLyt::CalcLockedStart() {
    if (mpExpAnmObjs[mCurLocked][0]->isFinished()) {
        mpLockedStates[mCurLocked]->ChangeState(2);
    }
}

void RPSelectGameLyt::EnterLockedLoop() {}

void RPSelectGameLyt::CalcLockedLoop() {
    if (mLockedStop[mCurLocked]) {
        mpLockedStates[mCurLocked]->ChangeState(3);
    }
}

void RPSelectGameLyt::EnterLockedEnd() {
    mLockedStop[mCurLocked] = false;
    mpExpPanes[mCurLocked]->UnbindAllAnimation(true);
    RPSysLayout::forceBindAnmObjToPane(mpExpAnmObjs[mCurLocked][1], mpExpPanes[mCurLocked], true);
    mpExpAnmObjs[mCurLocked][1]->restart();
}

void RPSelectGameLyt::CalcLockedEnd() {
    if (mpExpAnmObjs[mCurLocked][1]->isFinished()) {
        mpLockedStates[mCurLocked]->ChangeState(0);
    }
}

void RPSelectGameLyt::RequestLockedAnm(s32 slot) {
    s32 game = Menu_slotToGameId(slot);
    mLockedRequest[game] = true;
    mLockedStop[game] = false;
}

void RPSelectGameLyt::StopLockedAnm(s32 slot) {
    s32 game = Menu_slotToGameId(slot);
    mLockedStop[game] = true;
    mLockedRequest[game] = false;
}


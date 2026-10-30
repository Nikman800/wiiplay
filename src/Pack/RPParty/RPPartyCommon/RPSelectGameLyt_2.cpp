// Game-select board (RPSelectGameLyt): icon wobble and per-slot highlight states. Unit 0x801FD6D4-0x801FE3A8.
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

// ---- 0x801FD6D4-0x801FE3A8: icons and slots ----

void RPSelectGameLyt::EnterSlotHighlighted() {
    mpIconPanes[mCurSlot][0]->UnbindAllAnimation(true);
    RPSysLayout::forceBindAnmObjToPane(mpSlotAnmObjs[mCurSlot][0], mpIconPanes[mCurSlot][0], true);
    mpSlotAnmObjs[mCurSlot][0]->play(mpSlotAnmObjs[mCurSlot][0]->getFrameMax());
}

void RPSelectGameLyt::CalcSlotHighlighted() {
    StartMedalEffects(mCurSlot);
}

void RPSelectGameLyt::EnterSlotHighlight() {
    RPSysLayout::becomeYoungestChild(mpIconPanes[mCurSlot][0]);
    RPSysLayout::becomeYoungestChild(mpGamePosPanes[mCurSlot]);
    mpIconPanes[mCurSlot][1]->SetVisible(true);
    mpIconPanes[mCurSlot][0]->UnbindAllAnimation(true);
    RPSysLayout::forceBindAnmObjToPane(mpSlotAnmObjs[mCurSlot][0], mpIconPanes[mCurSlot][0], true);
    mpSlotAnmObjs[mCurSlot][0]->restart();
}

void RPSelectGameLyt::CalcSlotHighlight() {
    if (mpSlotStates[mCurSlot]->GetTimer() == 7) {
        StartMedalEffects(mCurSlot);
    }

    if (mpSlotAnmObjs[mCurSlot][0]->isFinished()) {
        mpSlotStates[mCurSlot]->ChangeState(0);
    }
}

void RPSelectGameLyt::EnterSlotIdle() {
    StopMedalEffects(mCurSlot);
    mpIconPanes[mCurSlot][1]->SetVisible(false);
    mpIconPanes[mCurSlot][0]->UnbindAllAnimation(true);
    RPSysLayout::forceBindAnmObjToPane(mpSlotAnmObjs[mCurSlot][1], mpIconPanes[mCurSlot][0], true);
    mpSlotAnmObjs[mCurSlot][1]->play(mpSlotAnmObjs[mCurSlot][1]->getFrameMax());
}

void RPSelectGameLyt::CalcSlotIdle() {}

void RPSelectGameLyt::EnterSlotUnhighlight() {
    StopMedalEffects(mCurSlot);
    mpIconPanes[mCurSlot][0]->UnbindAllAnimation(true);
    RPSysLayout::forceBindAnmObjToPane(mpSlotAnmObjs[mCurSlot][1], mpIconPanes[mCurSlot][0], true);
    mpSlotAnmObjs[mCurSlot][1]->restart();
}

void RPSelectGameLyt::CalcSlotUnhighlight() {
    if (mpSlotAnmObjs[mCurSlot][1]->isFinished()) {
        mpSlotStates[mCurSlot]->ChangeState(2);
    }
}

void RPSelectGameLyt::EnterSlotDecide() {
    mpIconPanes[mCurSlot][1]->SetVisible(true);
    mpIconPanes[mCurSlot][0]->UnbindAllAnimation(true);
    RPSysLayout::forceBindAnmObjToPane(mpSlotAnmObjs[mCurSlot][2], mpIconPanes[mCurSlot][0], true);
    mpSlotAnmObjs[mCurSlot][2]->restart();
    RPSndAudioMgr::instance()->startSystemSeWithRemote("RP_COMMON_SE_KEY_A_02_CTSP",
                                                       mDecidePlayerFlag);
    RPSndAudioMgr::instance()->startSystemSeWithRemote("RP_COMMON_SE_KEY_A_02_TVSP",
                                                       mDecidePlayerFlag);
}

void RPSelectGameLyt::CalcSlotDecide() {
    if (mpSlotAnmObjs[mCurSlot][2]->isFinished()) {
        mpSlotStates[mCurSlot]->ChangeState(5);
        StopMedalEffects(mCurSlot);
    }
}

bool RPSelectGameLyt::HighlightGame(s32 slot) {
    s32 game = Menu_slotToGameId(slot);
    UnhighlightOthers(slot);

    if (!mpStateMachine->IsStateSettled(2)) {
        return false;
    }

    if (mpSlotStates[game]->IsStateOrNext(3)) {
        return false;
    }

    if (mpSlotStates[game]->IsStateOrNext(5)) {
        mpSlotStates[game]->ChangeState(0);
    } else if (!mpSlotStates[game]->IsStateOrNext(0) && !mpSlotStates[game]->IsStateOrNext(1) &&
               !mpSlotStates[game]->IsStateOrNext(4)) {
        mpSlotStates[game]->ChangeState(1);
    }

    return true;
}

void RPSelectGameLyt::UnhighlightOthers(s32 slot) {
    for (int i = 0; i < GAME_NUM; i++) {
        if (i != slot) {
            s32 game = Menu_slotToGameId(i);
            if (!mpSlotStates[game]->IsStateOrNext(1) && !mpSlotStates[game]->IsStateOrNext(2) &&
                !mpSlotStates[game]->IsStateOrNext(3)) {
                mpSlotStates[game]->ChangeState(3);
            }
        }
    }
}

void RPSelectGameLyt::UnhighlightAll() {
    for (int i = 0; i < GAME_NUM; i++) {
        s32 game = Menu_slotToGameId(i);
        if (!mpSlotStates[game]->IsStateOrNext(1) && !mpSlotStates[game]->IsStateOrNext(2) &&
            !mpSlotStates[game]->IsStateOrNext(3)) {
            mpSlotStates[game]->ChangeState(3);
        }
    }
}

nw4r::lyt::Pane* RPSelectGameLyt::GetGameIconPane(s32 slot, s32 idx) {
    return mpIconBoundings[Menu_slotToGameId(slot)][idx];
}

EGG::Vector2f RPSelectGameLyt::GetGameIconPos(s32 slot) {
    s32 game = Menu_slotToGameId(slot);
    return Lyt_getPaneGlobalPos(mpIconPanes[game][0]);
}

void RPSelectGameLyt::AnimateIcons() {
    for (int i = 0; i < GAME_NUM; i++) {
        if (mUnlocked[i] &&
            (mpSlotStates[i]->GetState() == 2 || mpSlotStates[i]->GetState() == 3)) {
            if (mIconTimer[i] > mIconTimerEnd[i]) {
                mIconVelocity[i].x = 0.05f * RPUtlRandom::getF32();
                if (RPUtlRandom::getU32(2) == 0) {
                    mIconVelocity[i].x = -mIconVelocity[i].x;
                }
                mIconVelocity[i].y = 0.05f * RPUtlRandom::getF32();
                if (RPUtlRandom::getU32(2) == 0) {
                    mIconVelocity[i].y = -mIconVelocity[i].y;
                }
                mIconTimerEnd[i] = RPUtlRandom::getU32(180) + 120;
                mIconTimer[i] = 0;
            } else {
                mIconTimer[i]++;
            }

            mIconOffset[i].x += mIconVelocity[i].x;
            mIconOffset[i].y += mIconVelocity[i].y;

            f32 x = mIconOffset[i].x;
            if (x < -3.0f) {
                x = -3.0f;
            } else if (x > 3.0f) {
                x = 3.0f;
            }
            mIconOffset[i].x = x;

            f32 y = mIconOffset[i].y;
            if (y < -3.0f) {
                y = -3.0f;
            } else if (y > 3.0f) {
                y = 3.0f;
            }
            mIconOffset[i].y = y;

            EGG::Vector2f pos = EGG_Vector2f_operator_plus(&mIconPos[i], &mIconOffset[i]);
            mpIconPanes[i][0]->SetTranslate(pos);
        }
    }
}

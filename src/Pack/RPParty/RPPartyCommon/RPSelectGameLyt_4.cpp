// Game-select board (RPSelectGameLyt): record text. Unit 0x801FE73C-0x801FE978.
// Functions in reverse address order.
#include <Pack/RPParty/RPPartyCommon/RPSelectGameLyt.h>
#include <Pack/RPParty/RPPartyCommon/RPSelectScene.h>
#include <Pack/RPParty/RPPartyCommon/RPCommon.h>
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

// ---- 0x801FE73C-0x801FE978: record text ----

void RPSelectGameLyt::EnterExpIdle() {}

void RPSelectGameLyt::CalcExpIdle() {
    if (mExpPending) {
        mpExpState->ChangeState(1);
    }
}

void RPSelectGameLyt::EnterExpChange() {
    mpScoreTextBox->UnbindAllAnimation(true);
    RPSysLayout::bindAnmObjToPane(mpScoreAnmObjs[0], mpScoreTextBox, true);
    mpScoreAnmObjs[0]->restart();
}

void RPSelectGameLyt::CalcExpChange() {
    RPSysFrameCtrl* pFrameCtrl = mpScoreAnmObjs[0]->getFrameCtrl();
    if (pFrameCtrl->getFrame() == pFrameCtrl->getEndFrame() / 2) {
        if (mExpNext == GAME_NUM) {
            mHighlighted = mExpNext;
            mpScoreTextBox->setMessage(RPSelectScene::sInstance->mpMessage, 1000, 0, 0);
        } else if (RPSysSaveDataMgr::instance()->isAsyncFinish()) {
            RPPartyPlayerData* pData = RPSysPlayerMgr::instance()->getPlayer(0).mpPartyPlayerData;
            mHighlighted = mExpNext;
            s32 record =
                pData->getMyRecord(static_cast<RPPartyPlayerData::EGame>(mExpNext), 0);
            wchar_t number[20];
            RPSysStringUtility::GetStringHalfSizeNumber(record, number, 20, false);
            s32 gameIdx, plural;
            RPCommon::GetGameMsgIndex(record, mHighlighted, &gameIdx, &plural);
            mpScoreTextBox->setMessage(RPSelectScene::sInstance->mpMessage, gameIdx + 0x206C,
                                       plural, 1, number);
        }

        mExpPending = false;
    }

    if (mpScoreAnmObjs[0]->isFinished()) {
        mpExpState->ChangeState(0);
    }
}

void RPSelectGameLyt::RequestExpChange(s32 game) {
    bool change = false;
    if (mExpPending && game != mExpNext) {
        change = true;
    }
    if (!mExpPending && game != mHighlighted) {
        change = true;
    }

    if (change) {
        mExpNext = game;
        mExpPending = true;
    }
}


#include <Pack/RPParty/RPPartyCommon/RPABPress.h>
#include <Pack/RPParty/RPPartyCommon/RPKokeshiMgr.h>
#include <Pack/RPParty/RPPartyCommon/RPButtonSeq.h>

#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytResAccessor.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPAudio/RPSndAudioMgr.h>
#include <Pack/RPSystem/RPSysPlayerMgr.h>
#include <Pack/RPSystem/RPSysResourceManager.h>

#include <egg/core/eggController.h>

#include <cstdio>


extern "C" void Lyt_setPaneColorFromTable(nw4r::lyt::Pane* pPane, u32 player);

namespace {

inline void setFrameCtrlStart(RPSysLytAnmObj* pAnmObj) {
    pAnmObj->mFrameCtrl.mFrame = pAnmObj->mFrameCtrl.mStartFrame;
    pAnmObj->mFrameCtrl.mFlags = 0;
    pAnmObj->mFrameCtrl.mIsEnabled = 1;
}

inline void setAnmRate(RPSysLytAnmObj* pAnmObj, f32 rate) {
    pAnmObj->mFrameCtrl.mUpdateRate = rate;
}

} // namespace

RPButtonSeq::RPButtonSeq(s32 channel, const u16* pSeq)
    : mChannel(channel), mpSeq(pSeq), mIndex(0) {}

void RPButtonSeq::Update() {
    if (mpSeq[mIndex] == 0) {
        return;
    }

    EGG::CoreController* pController =
        EGG::CoreControllerMgr::instance()->getNthController(mChannel);
    u32 trigger = pController->getCoreStatus()->getTrigger();
    if (trigger == 0) {
        return;
    }

    if ((trigger & mpSeq[mIndex]) != 0) {
        mIndex++;
    } else {
        mIndex = 0;
    }
}

bool RPButtonSeq::IsComplete() const {
    return mpSeq[mIndex] == 0;
}

void RPABPress::Load(EGG::Archive* pArchive, RPSysMessage* pMessage, bool arg3) {
    mpMessage = pMessage;
    RPSysLytResAccessor* pAccessor = RPSysLytResAccessor::create(NULL);
    pAccessor->mountArchive(
        RPSysResourceManager::GetFileFromArchive(
            pArchive,
            "ABPress.arc", NULL),
        NULL);

    mpLayout = RPSysLayout::create(NULL, pAccessor, "ABPress_00.brlyt");

    for (s32 i = 0; i < 2; i++) {
        mAnims[i].mpInAnmObj =
            mpLayout->createAnmObj(NULL, pAccessor,
                                           "ABPress_00_in_00.brlan");
        mAnims[i].mpOutAnmObj =
            mpLayout->createAnmObj(NULL, pAccessor,
                                           "ABPress_00_out_00.brlan");
    }

    mAnims[0].mpPane = mpLayout->findPane("N_bg_00");
    mAnims[1].mpPane = mpLayout->findPane("N_main_00");

    char name[256];
    for (s32 i = 0; i < RPABPress::PLAYER_NUM; i++) {
        RPABPress::Player& player = mPlayers[i];

        std::sprintf(name, "N_player_%02d", i);
        player.mpPane = mpLayout->findPane(name);
        std::sprintf(name, "P_pColor1_%02d", i);
        player.mpColor1 = mpLayout->findPicture(name);
        std::sprintf(name, "P_pColor2_%02d", i);
        player.mpColor2 = mpLayout->findPicture(name);
        std::sprintf(name, "N_LED_%02d", i);
        player.mpLEDPane = mpLayout->findPane(name);

        for (s32 j = 0; j < 4; j++) {
            std::sprintf(name, "P_P%dLED_%02d", j + 1, i);
            player.mpLEDPanes[j] = mpLayout->findPane(name);
            player.mpLEDAnmObjs[j] =
                mpLayout->createAnmObj(NULL, pAccessor,
                                               "ABPress_00_LED_00.brlan");
        }

        std::sprintf(name, "N_ok_%02d", i);
        player.mpOKPane = mpLayout->findPane(name);
        player.mpOKInAnmObj =
            mpLayout->createAnmObj(NULL, pAccessor,
                                           "ABPress_00_inOK_00.brlan");
        player.mpOKInAnmObj2 =
            mpLayout->createAnmObj(NULL, pAccessor,
                                           "ABPress_00_inOK_00.brlan");

        std::sprintf(name, "T_pName_%02d", i);
        player.mpNameTextBox = mpLayout->findTextBox(name);
        if (!arg3) {
            player.mpHead = NULL;
        }

        std::sprintf(name, "T_ok_%02d", i);
        mpLayout->findTextBox(name)->setMessage(
            mpMessage, 0x367, 0, 0);
    }

    mpLayout->findTextBox("T_ABPress_00")->setMessage(
        mpMessage, 0x370, 0, 0);
}

void RPABPress::Reset() {
    mState = 0;
    mTimer = -1;
    mpLayout->reset();
    mState = 0;
    mpLayout->unbindAllAnmObj();
    mPlayer = 0;
    mDone = false;
}

void RPABPress::Calculate() {
    if (mState == 0) {
        return;
    }

    if (mTimer < 0) {
        mTimer = 0;
    }

    switch (mState) {
    case 0:
        break;
    case 1:
        mpLayout->reset();
        mpLayout->unbindAllAnmObj();

        for (s32 i = 0; i < 2; i++) {
            RPSysLayout::bindAnmObjToPane(
                mAnims[i].mpInAnmObj, mAnims[i].mpPane, true);
            setFrameCtrlStart(mAnims[i].mpInAnmObj);
        }

        for (s32 i = 0; i < RPABPress::PLAYER_NUM; i++) {
            RPABPress::Player& player = mPlayers[i];

            RPSysLayout::forceBindAnmObjToPane(
                player.mpOKInAnmObj, player.mpLEDPane, true);
            RPSysLayout::forceBindAnmObjToPane(
                player.mpOKInAnmObj2, player.mpOKPane, true);
            setFrameCtrlStart(player.mpOKInAnmObj);
            setFrameCtrlStart(player.mpOKInAnmObj2);

            for (s32 j = 0; j < 4; j++) {
                RPSysLayout::forceBindAnmObjToPane(
                    player.mpLEDAnmObjs[j], player.mpLEDPanes[j], true);
                setFrameCtrlStart(player.mpLEDAnmObjs[j]);
            }

            player.unk40 = 0;
            player.unk3C = 0;
        }

        mState = 2;
        mTimer = -1;
        break;
    case 2:
        if (mAnims[0].mpInAnmObj->isFinished()) {
            mState = 3;
            mTimer = -1;
        }
        break;
    case 4:
        if (mTimer == 60) {
            for (s32 i = 0; i < 2; i++) {
                RPSysLayout::unbindAnmObjToPane(
                    mAnims[i].mpInAnmObj, mAnims[i].mpPane,
                    true);
                RPSysLayout::unbindAnmObjToPane(
                    mAnims[i].mpOutAnmObj, mAnims[i].mpPane,
                    true);
                RPSysLayout::bindAnmObjToPane(
                    mAnims[i].mpOutAnmObj, mAnims[i].mpPane,
                    true);

                for (s32 j = 0; j < RPABPress::PLAYER_NUM; j++) {
                    RPSysLayout::unbindAnmObjToPane(
                        mAnims[i].mpOutAnmObj,
                        mPlayers[j].mpLEDPane, true);
                    RPSysLayout::unbindAnmObjToPane(
                        mAnims[i].mpOutAnmObj,
                        mPlayers[j].mpOKPane, true);
                }

                if (mPlayer != -1) {
                    RPSysFrameCtrl& frameCtrl =
                        mAnims[i].mpOutAnmObj->mFrameCtrl;
                    frameCtrl.mFrame = frameCtrl.mStartFrame;
                    frameCtrl.mFlags = 0;
                    frameCtrl.mIsEnabled = 1;
                }
            }
        }

        if (mTimer > 60) {
            if (mPlayer == -1) {
                mDone = true;
            } else if (mAnims[0].mpOutAnmObj->isFinished()) {
                mState = 0;
                mTimer = -1;
                mDone = true;
            }
        }
        break;
    }

    mTimer++;

    bool allComplete = true;
    for (s32 i = 0; i < RPABPress::PLAYER_NUM; i++) {
        RPABPress::Player& player = mPlayers[i];
        s32 playerIndex = mPlayer == -1 ? i : mPlayer;

        if (i >= RPSysPlayerMgr::instance()->getPlayerNum() || (i != 0 && mPlayer >= 0)) {
            player.mpPane->SetVisible(false);
            continue;
        }
        player.mpPane->SetVisible(true);

        s32 channel = RPSysPlayerMgr::instance()->getChannel(playerIndex);
        s32 devType = EGG::CoreControllerMgr::instance()->getNthController(channel)->getChannelID();

        Lyt_setPaneColorFromTable(player.mpColor1, playerIndex);
        Lyt_setPaneColorFromTable(player.mpColor2, playerIndex);

        for (s32 j = 0; j < 4; j++) {
            player.mpLEDAnmObjs[j]->setFrame(static_cast<f32>(devType));
        }

        if (player.unk3C == 0) {
            player.unk3C = 1;
        }

        if (mState == 3 && static_cast<s32>(player.unk3C) != 2 &&
            (EGG::CoreControllerMgr::instance()->getNthController(channel)
                 ->getCoreStatus()
                 ->getHold() &
             0x800) != 0 &&
            (EGG::CoreControllerMgr::instance()->getNthController(channel)
                 ->getCoreStatus()
                 ->getHold() &
             0x400) != 0) {
            player.unk3C = 2;
            const u32 remoteFlags[4] = {
                1 << 0,
                1 << 1,
                1 << 2,
                1 << 3,
            };
            RPSndAudioMgr::instance()->startSystemSeWithRemote("SE_TR_COMMON_Push_AB_CTSP", remoteFlags[channel]);
            RPSndAudioMgr::instance()->startSystemSeWithRemote("SE_TR_COMMON_Push_AB_TVSP", remoteFlags[channel]);
        }

        if (static_cast<s32>(player.unk3C) != 2) {
            allComplete = false;
        }
        if (static_cast<s32>(player.unk3C) != 2) {
            player.mpOKPane->SetVisible(false);
            player.mpOKInAnmObj->setRate(0.0f);
            player.mpOKInAnmObj2->setRate(0.0f);
        } else {
            player.mpOKPane->SetVisible(true);
            player.mpOKInAnmObj->setRate(1.0f);
            player.mpOKInAnmObj2->setRate(1.0f);
        }

        player.mpNameTextBox->setMessage(
            mpMessage, 0x366, 0, 1,
            mPlayers[playerIndex].mpHead->mAdditionalInfo.name);
    }

    if (mState == 3 && allComplete) {
        mState = 4;
        mTimer = -1;

        if (mPlayer == -1) {
            for (s32 i = 0; i < RPSysPlayerMgr::instance()->getPlayerNum(); i++) {
                RPSysPlayerMgr::instance()->setABPress(i);
            }
        } else {
            RPSysPlayerMgr::instance()->setABPress(mPlayer);
        }
    }

    if (mState != 0) {
        mpLayout->calc();
    }
}

void RPABPress::Draw() {
    if (mState == 0) {
        return;
    }

    mpLayout->draw();
}

void RPABPress::Start(s32 player) {
    s32 index;
    bool needed = true;
    if (player == -1) {
        for (index = 0; index < RPSysPlayerMgr::instance()->getPlayerNum();
             index++) {
            if (!RPSysPlayerMgr::instance()->isABPress(index)) {
                needed = false;
            }
        }
    } else {
        if (!RPSysPlayerMgr::instance()->isABPress(player)) {
            needed = false;
        }
    }

    if (needed) {
        mDone = true;
    } else {
        mState = 1;
        mTimer = -1;
        mPlayer = player;
        mDone = false;
    }

}

BOOL RPABPress::IsNeeded(s32 player) {
    u32 needed = 1;
    if (player == -1) {
        for (s32 i = 0; i < RPSysPlayerMgr::instance()->getPlayerNum(); i++) {
            if (!RPSysPlayerMgr::instance()->isABPress(i)) {
                needed = false;
            }
        }
    } else if (!RPSysPlayerMgr::instance()->isABPress(player)) {
        needed = false;
    }

    return static_cast<u32>(__cntlzw(static_cast<u32>(needed))) >> 5;
}

bool RPABPress::IsDone() {
    return mDone;
}

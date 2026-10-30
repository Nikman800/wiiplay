// Pose Mii pose selector (RPBomLytPoseSel, poseManAB_00 / poseManAB_01).
#include <Pack/RPParty/RPBomScene/RPBomLyt_8.h>

#include <Pack/RPKernel/RPSysEffectBase.h>
#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>

#include <egg/core.h>
#include <nw4r/lyt.h>

// Shows the active pose animation on pose pane `frame` once the frame animation has ended
inline void RPBomLytPoseSel::ShowActivePose(s32 frame) {
    const char* poseNames[3] = {"P_poseL_00", "P_poseL_01", "P_poseL_02"};
    nw4r::lyt::Pane* pPane = mpLayout->findPane(poseNames[frame]);
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[frame + 1][8], pPane, true);
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[frame + 1][9], pPane, true);
    RPSysLayout::forceBindAnmObjToPane(mpAnmObjs[frame + 1][8], pPane, false);
    mpAnmObjs[frame + 1][8]->restart();
    mpAnmObjs[frame + 1][8]->setRate(1.0f);
    mPoseFrom = mPoseFrame;
}

// Shows the inactive pose animation on pose pane `frame`
inline void RPBomLytPoseSel::ShowInactivePose(s32 frame) {
    const char* poseNames[3] = {"P_poseL_00", "P_poseL_01", "P_poseL_02"};
    nw4r::lyt::Pane* pPane = mpLayout->findPane(poseNames[frame]);
    s32 set = frame + 1;
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[set][8], pPane, true);
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[set][9], pPane, true);
    RPSysLayout::forceBindAnmObjToPane(mpAnmObjs[set][9], pPane, true);
    mpAnmObjs[set][9]->restart();
    mpAnmObjs[set][9]->setRate(1.0f);
}

RPBomLytPoseSel::RPBomLytPoseSel(s32 player) {
    mPlayer = player;
    if (player == 0) {
        mpEffects[0] = new RPSysEffect("Bom_posechange_1p_a", 4, 0);
        mpEffects[1] = new RPSysEffect("Bom_posechange_1p_b", 4, 0);
    } else {
        mpEffects[0] = new RPSysEffect("Bom_posechange_2p_a", 4, 0);
        mpEffects[1] = new RPSysEffect("Bom_posechange_2p_b", 4, 0);
    }
}

RPBomLytPoseSel::~RPBomLytPoseSel() {}

#pragma push
#pragma auto_inline off
void RPBomLytPoseSel::ResetPanes() {
    mpLayout->unbindAllAnmObj();
    RPSysLayout::forceBindAnmObjToPane(mpAnmObjs[0][0], mpLayout->findPane("N_all_00"), false);
    mpAnmObjs[0][0]->restart();
    mpAnmObjs[0][0]->stop();
    mpAnmObjs[0][0]->setFrame(0.0f);
    mpLayout->calc();

    mpLayout->unbindAllAnmObj();
    RPSysLayout::forceBindAnmObjToPane(mpAnmObjs[1][9], mpLayout->findPane("P_poseL_00"), false);
    mpAnmObjs[1][9]->restart();
    RPSysLayout::forceBindAnmObjToPane(mpAnmObjs[2][9], mpLayout->findPane("P_poseL_01"), false);
    mpAnmObjs[2][9]->restart();
    RPSysLayout::forceBindAnmObjToPane(mpAnmObjs[3][9], mpLayout->findPane("P_poseL_02"), false);
    mpAnmObjs[3][9]->restart();
    mpLayout->calc();
}

void RPBomLytPoseSel::PlayPoseFrameAnim(s32 pose, BOOL reverse) {
    int anm;
    nw4r::lyt::Pane* pFrame;
    nw4r::lyt::Pane* pBg;
    int i;
    anm = 2;
    switch (pose) {
    case 0:
        anm = 4;
        if (!reverse) {
            anm = 5;
        }
        break;
    case 1:
        anm = 3;
        if (!reverse) {
            anm = 7;
        }
        break;
    case 2:
        anm = 2;
        if (!reverse) {
            anm = 6;
        }
        break;
    }

    pFrame = mpLayout->findPane("N_frame_00");
    pBg = mpLayout->findPane("P_bgL_00");
    for (i = 2; i <= 7; i++) {
        RPSysLayout::unbindAnmObjToPane(mpAnmObjs[0][i], pFrame, true);
        RPSysLayout::unbindAnmObjToPane(mpAnmObjs[0][i], pBg, true);
    }

    RPSysLayout::forceBindAnmObjToPane(mpAnmObjs[0][anm], pFrame, true);
    RPSysLayout::forceBindAnmObjToPane(mpAnmObjs[0][anm], pBg, true);
    mpAnmObjs[0][anm]->restart();
    mpAnmObjs[0][anm]->setRate(1.0f);

    if (mPoseFrom != -1) {
        ShowInactivePose(mPoseFrom);
    }
}

void RPBomLytPoseSel::UpdatePoseTransition() {
    if (mPoseFrame == 0) {
        if (mPoseFrom == -1 && mpAnmObjs[0][5]->isFinished()) {
            ShowActivePose(0);
        } else if (mPoseFrom == 2 && mpAnmObjs[0][4]->isFinished()) {
            ShowActivePose(0);
        } else if (mPoseFrom == 1 && mpAnmObjs[0][5]->isFinished()) {
            ShowActivePose(0);
        }
    } else if (mPoseFrame == 1) {
        if (mPoseFrom == -1 && mpAnmObjs[0][6]->isFinished()) {
            ShowActivePose(1);
        } else if (mPoseFrom == 2 && mpAnmObjs[0][6]->isFinished()) {
            ShowActivePose(1);
        } else if (mPoseFrom == 0 && mpAnmObjs[0][2]->isFinished()) {
            ShowActivePose(1);
        }
    } else if (mPoseFrame == 2) {
        if (mPoseFrom == -1 && mpAnmObjs[0][7]->isFinished()) {
            ShowActivePose(2);
        } else if (mPoseFrom == 0 && mpAnmObjs[0][7]->isFinished()) {
            ShowActivePose(2);
        } else if (mPoseFrom == 1 && mpAnmObjs[0][3]->isFinished()) {
            ShowActivePose(2);
        }
    }
}

void RPBomLytPoseSel::UpdateEffectPositions() {
    nw4r::math::VEC3 pos(0.0f, 0.0f, 0.0f);
    nw4r::math::VEC3 scale(1.0f, 1.0f, 1.0f);

    {
        nw4r::math::MTX34 mtx = mpLayout->findPane("P_AbgL_00")->GetGlobalMtx();
        pos.x = mtx._03;
        pos.y = mtx._13;
        pos.z = mtx._23;
        mpEffects[0]->setPos(pos);
        mpEffects[0]->setScale(scale);
        mpEffects[0]->update();
    }
    {
        nw4r::math::MTX34 mtx = mpLayout->findPane("P_BbgL_00")->GetGlobalMtx();
        pos.x = mtx._03;
        pos.y = mtx._13;
        pos.z = mtx._23;
        mpEffects[1]->setPos(pos);
        mpEffects[1]->setScale(scale);
        mpEffects[1]->update();
    }
}

#pragma pop

void RPBomLytPoseSel::Load(EGG::Archive* pArchive) {
    const char* layoutNames[2] = {"poseManAB_00.brlyt", "poseManAB_01.brlyt"};
    const char* anmNames[2][10] = {
        {"poseManAB_00_inAll_00.brlan", "poseManAB_00_outAll_00.brlan",
         "poseManAB_00_frame01_00.brlan", "poseManAB_00_frame12_00.brlan",
         "poseManAB_00_frame20_00.brlan", "poseManAB_00_frame10_00.brlan",
         "poseManAB_00_frame21_00.brlan", "poseManAB_00_frame02_00.brlan",
         "poseManAB_00_activePose_00.brlan", "poseManAB_00_activePose_01.brlan"},
        {"poseManAB_01_inAll_00.brlan", "poseManAB_01_outAll_00.brlan",
         "poseManAB_01_frame01_00.brlan", "poseManAB_01_frame12_00.brlan",
         "poseManAB_01_frame20_00.brlan", "poseManAB_01_frame10_00.brlan",
         "poseManAB_01_frame21_00.brlan", "poseManAB_01_frame02_00.brlan",
         "poseManAB_01_activePose_00.brlan", "poseManAB_01_activePose_01.brlan"},
    };

    AttachArcFile(pArchive, "poseMan.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, layoutNames[mPlayer]);

    int i = 0;
    do {
        int j = 0;
        do {
            mpAnmObjs[i][j] = mpLayout->createAnmObj(NULL, mpResAccessor, anmNames[mPlayer][j]);
            mpAnmObjs[i][j]->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
        } while (++j < 10);
    } while (++i < 4);
}

void RPBomLytPoseSel::Reset() {
    mState = 1;
    mPoseFrom = -1;
    mPoseFrame = -1;
    mDone = false;

    mpLayout->reset();

    int i = 0;
    do {
        int j = 0;
        do {
            mpAnmObjs[i][j]->resetFrame();
            mpAnmObjs[i][j]->stop();
            mpAnmObjs[i][j]->setFrame(0.0f);
        } while (++j < 10);
    } while (++i < 4);

    ResetPanes();
}

void RPBomLytPoseSel::Calculate() {
    UpdatePoseTransition();

    if (mPoseFrom == 2) {
        mpLayout->findPane("P_arrow_00")->SetVisible(true);
        mpLayout->findPane("P_arrow_01")->SetVisible(false);
    } else if (mPoseFrom == 0) {
        mpLayout->findPane("P_arrow_00")->SetVisible(false);
        mpLayout->findPane("P_arrow_01")->SetVisible(true);
    } else {
        mpLayout->findPane("P_arrow_00")->SetVisible(true);
        mpLayout->findPane("P_arrow_01")->SetVisible(true);
    }

    if (mState == 2) {
        if (mpAnmObjs[0][0]->isFinished()) {
            mpLayout->unbindAllAnmObj();
            mState = 0;
        }
    } else if (mState == 3) {
        if (mpAnmObjs[0][1]->isFinished()) {
            mState = 1;
            mDone = true;
        }
    }

    mpLayout->calc();
    UpdateEffectPositions();
}

void RPBomLytPoseSel::Draw() {
    mpLayout->draw();
}

void RPBomLytPoseSel::VF_0x18() {}

void RPBomLytPoseSel::SetPose(s32 pose) {
    s32 frames[3] = {0, 2, 1};
    mPoseFrame = frames[pose];
    PlayPoseFrameAnim(pose, FALSE);
}

void RPBomLytPoseSel::Show() {
    if (mState != 1) {
        return;
    }

    mDone = false;
    mpLayout->unbindAnmObj(mpAnmObjs[0][0]);
    mpLayout->unbindAnmObj(mpAnmObjs[0][1]);
    RPSysLayout::forceBindAnmObjToPane(mpAnmObjs[0][0], mpLayout->findPane("N_all_00"), false);
    mpAnmObjs[0][0]->restart();
    mState = 2;
}

void RPBomLytPoseSel::PlayOut() {
    if (mState == 1) {
        return;
    }

    mpLayout->unbindAnmObj(mpAnmObjs[0][0]);
    mpLayout->unbindAnmObj(mpAnmObjs[0][1]);
    RPSysLayout::forceBindAnmObjToPane(mpAnmObjs[0][1], mpLayout->findPane("N_all_00"), false);
    mpAnmObjs[0][1]->restart();
    mState = 3;
}

void RPBomLytPoseSel::ChangePose(s32 player, s32 pose, BOOL reverse) {
    s32 frames[3] = {0, 2, 1};
    mPoseFrame = frames[pose];
    PlayPoseFrameAnim(pose, reverse);

    if (reverse) {
        mpEffects[1]->create();
        UpdateEffectPositions();
    } else {
        mpEffects[0]->create();
        UpdateEffectPositions();
    }
}

void RPBomLytPoseSel::PlayBlocked(s32 player, BOOL reverse) {
    if (reverse) {
        mpEffects[1]->create();
        UpdateEffectPositions();
    } else {
        mpEffects[0]->create();
        UpdateEffectPositions();
    }

    EGG::CoreControllerMgr::instance()->getNthController((u16)player)->startSimpleRumble(
        0xFFFFFFFF, 5, 0x20);
}

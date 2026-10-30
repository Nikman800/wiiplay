#include <Pack/RPParty/RPTnkScene/RPTnkLyt.h>
#include <Pack/RPParty/RPTnkScene/RPTnkGameManager.h>
#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPKernel/RPSysStringUtility.h>
#include <Pack/RPAudio/RPSndAudioMgr.h>
#include <egg/core/eggController.h>

// Pane and animation literals share the original result-name string pool.
struct RPTnkResultNames { const char* names[9]; };
struct RPTnkResultAnmNames { const char* names[8]; };


// Keep the Start initializer tables first in the read-only pool.
inline RPTnkResultNames ResultNames1P() {
    RPTnkResultNames names = {{ "T_No00_00", "T_No00_01", "T_No00_02", "T_No00_03", "T_No00_04", "T_No00_05", "T_No00_06", "T_No00_07", "T_No00_08" }};
    return names;
}

inline RPTnkResultNames ResultNames2P1() {
    RPTnkResultNames names = {{ "T_No01_00", "T_No01_01", "T_No01_02", "T_No01_03", "T_No01_04", "T_No01_05", "T_No01_06", "T_No01_07", "T_No01_08" }};
    return names;
}

inline RPTnkResultNames ResultNames2P2() {
    RPTnkResultNames names = {{ "T_No02_00", "T_No02_01", "T_No02_02", "T_No02_03", "T_No02_04", "T_No02_05", "T_No02_06", "T_No02_07", "T_No02_08" }};
    return names;
}

inline RPTnkResultNames ResultPositions1P() {
    RPTnkResultNames names = {{ "N_p00_00", "N_p00_01", "N_p00_02", "N_p00_03", "N_p00_04", "N_p00_05", "N_p00_06", "N_p00_07", "N_p00_08" }};
    return names;
}

inline RPTnkResultNames ResultPositions2P() {
    RPTnkResultNames names = {{ "N_p01_00", "N_p01_01", "N_p01_02", "N_p01_03", "N_p01_04", "N_p01_05", "N_p01_06", "N_p01_07", "N_p01_08" }};
    return names;
}

inline RPTnkResultNames ResultPanes1P() {
    RPTnkResultNames names = {{ "N_r00_00", "N_r00_01", "N_r00_02", "N_r00_03", "N_r00_04", "N_r00_05", "N_r00_06", "N_r00_07", "N_r00_08" }};
    return names;
}

inline RPTnkResultNames ResultPanes2P() {
    RPTnkResultNames names = {{ "N_r01_00", "N_r01_01", "N_r01_02", "N_r01_03", "N_r01_04", "N_r01_05", "N_r01_06", "N_r01_07", "N_r01_08" }};
    return names;
}

inline RPTnkResultAnmNames ResultAnimations() {
    RPTnkResultAnmNames names = {{ "tnkResult_00_inList_00.brlan", "tnkResult_00_inScore_00.brlan", "tnkResult_00_inTotal_00.brlan", "tnkResult_00_win1P_00.brlan", "tnkResult_00_win2P_00.brlan", "tnkResult_00_outList_00.brlan", "tnkResult_00_outTotal_00.brlan", "tnkResult_00_upTotal_00.brlan" }};
    return names;
}

RPTnkResultLyt::RPTnkResultLyt() {}
RPTnkResultLyt::~RPTnkResultLyt() {}

void RPTnkResultLyt::HideAllPanes() {
    mpLayout->findPane("N_list_00")->SetVisible(false);
    mpLayout->findPane("N_list_01")->SetVisible(false);
    mpLayout->findPane("N_r00_00")->SetVisible(false);
    mpLayout->findPane("N_r00_01")->SetVisible(false);
    mpLayout->findPane("N_r00_02")->SetVisible(false);
    mpLayout->findPane("N_r00_03")->SetVisible(false);
    mpLayout->findPane("N_r00_04")->SetVisible(false);
    mpLayout->findPane("N_r00_05")->SetVisible(false);
    mpLayout->findPane("N_r00_06")->SetVisible(false);
    mpLayout->findPane("N_r00_07")->SetVisible(false);
    mpLayout->findPane("N_r00_08")->SetVisible(false);
    mpLayout->findPane("N_r01_00")->SetVisible(false);
    mpLayout->findPane("N_r01_01")->SetVisible(false);
    mpLayout->findPane("N_r01_02")->SetVisible(false);
    mpLayout->findPane("N_r01_03")->SetVisible(false);
    mpLayout->findPane("N_r01_04")->SetVisible(false);
    mpLayout->findPane("N_r01_05")->SetVisible(false);
    mpLayout->findPane("N_r01_06")->SetVisible(false);
    mpLayout->findPane("N_r01_07")->SetVisible(false);
    mpLayout->findPane("N_r01_08")->SetVisible(false);
    mpLayout->findPane("N_totalAll_00")->SetVisible(false);
    mpLayout->findPane("N_totalAll_01")->SetVisible(false);
}

void RPTnkResultLyt::RevealRow1P(s32 type, s32 row) {
    RPTnkResultNames panes = ResultPanes1P();
    nw4r::lyt::Pane* pane = mpLayout->findPane(panes.names[type]);
    pane->SetVisible(true);
    RPTnkResultNames positions = ResultPositions1P();
    nw4r::lyt::Pane* position = mpLayout->findPane(positions.names[row]);
    nw4r::math::VEC3 pos = position->GetTranslate();
    nw4r::math::VEC3 trans = pane->GetTranslate();
    trans.y = pos.y;
    pane->SetTranslate(trans);
    mpLayout->unbindAnmObj(mpAnmObjs[0]);
    mpLayout->unbindAnmObj(mpAnmObjs[1]);
    RPSysLayout::forceBindAnmObjToPane(mpAnmObjs[1], pane, true);
    mpAnmObjs[1]->restart();
    PlayRowSound(row);
    mState = 2;
}

void RPTnkResultLyt::RevealRow2P(s32 type, s32 row) {
    RPTnkResultNames panes = ResultPanes2P();
    nw4r::lyt::Pane* pane = mpLayout->findPane(panes.names[type]);
    pane->SetVisible(true);
    RPTnkResultNames positions = ResultPositions2P();
    nw4r::lyt::Pane* position = mpLayout->findPane(positions.names[row]);
    nw4r::math::VEC3 pos = position->GetTranslate();
    nw4r::math::VEC3 trans = pane->GetTranslate();
    trans.y = pos.y;
    pane->SetTranslate(trans);
    mpLayout->unbindAnmObj(mpAnmObjs[0]);
    mpLayout->unbindAnmObj(mpAnmObjs[1]);
    RPSysLayout::forceBindAnmObjToPane(mpAnmObjs[1], pane, true);
    mpAnmObjs[1]->restart();
    PlayRowSound(row);
    mState = 2;
}

void RPTnkResultLyt::CalcSequence1P() {
    if (mState == 1) {
        if (!unk3C && mpAnmObjs[0]->isFinished()) {
            unk38 = 20;
            unk3C = true;
        } else if (unk3C) {
            if (--unk38 <= 0) {
                mRow = 0;
                RevealRow1P(RPTnkGameStats::sInstance->GetTypeOrder(0), 0);
                unk38 = 0;
                unk3C = false;
            }
        }
    }
    if (mState == 2) {
        if (!unk3C && mpAnmObjs[1]->isFinished()) {
            unk38 = 20;
            unk3C = true;
        } else if (unk3C) {
            if (--unk38 <= 0) {
                s32 types = RPTnkGameStats::sInstance->GetTypeNum();
                ++mRow;
                if (mRow == 9 || mRow == types) {
                    mpLayout->findPane("N_totalAll_00")->SetVisible(true);
                    mpLayout->unbindAnmObj(mpAnmObjs[2]);
                    mpLayout->bindAnmObjToPane(mpAnmObjs[2], "N_totalAll_00", true);
                    mpAnmObjs[2]->restart();
                    RPSndAudioMgr::instance()->startSystemSe(0x2b);
                    mState = 3;
                } else {
                    RevealRow1P(RPTnkGameStats::sInstance->GetTypeOrder(mRow), mRow);
                }
                unk38 = 0;
                unk3C = false;
            }
        }
    }
    if (mState == 3) {
        if (mpAnmObjs[2]->isFinished()) {
            mpLayout->unbindAllAnmObj();
            mpLayout->unbindAnmObj(mpAnmObjs[3]);
            mpLayout->bindAnmObjToPane(mpAnmObjs[3], "N_total_00", true);
            mpAnmObjs[3]->restart();
            mState = 4;
        }
    } else if (mState == 6) {
        if (mpAnmObjs[5]->isFinished() && mpAnmObjs[6]->isFinished()) {
            mState = 0;
            unk3D = true;
        }
    }
}

void RPTnkResultLyt::CalcSequence2P() {
    if (mState == 1) {
        if (!unk3C && mpAnmObjs[0]->isFinished()) {
            unk38 = 20;
            unk3C = true;
        } else if (unk3C) {
            if (--unk38 <= 0) {
                mRow = 0;
                RevealRow2P(RPTnkGameStats::sInstance->GetTypeOrder(0), 0);
                unk38 = 0;
                unk3C = false;
            }
        }
    }
    if (mState == 2) {
        if (!unk3C && mpAnmObjs[1]->isFinished()) {
            unk38 = 20;
            unk3C = true;
        } else if (unk3C) {
            if (--unk38 <= 0) {
                s32 types = RPTnkGameStats::sInstance->GetTypeNum();
                ++mRow;
                if (mRow == 9 || mRow == types) {
                    mpLayout->findPane("N_totalAll_01")->SetVisible(true);
                    mpLayout->unbindAnmObj(mpAnmObjs[2]);
                    mpLayout->bindAnmObjToPane(mpAnmObjs[2], "N_totalAll_01", true);
                    mpAnmObjs[2]->restart();
                    RPSndAudioMgr::instance()->startSystemSe(0x2b);
                    mState = 3;
                } else {
                    RevealRow2P(RPTnkGameStats::sInstance->GetTypeOrder(mRow), mRow);
                }
                unk38 = 0;
                unk3C = false;
            }
        }
    }
    if (mState == 3) {
        if (mpAnmObjs[2]->isFinished()) {
            mState = 4;
        }
    } else if (mState == 5) {
        if (mpAnmObjs[7]->isFinished()) {
            int winner = RPTnkGameStats::sInstance->GetWinner();
            if (winner != -1) {
                mpLayout->unbindAllAnmObj();
                if (winner == 0) {
                    mpLayout->unbindAnmObj(mpAnmObjs[3]);
                    mpLayout->bindAnmObjToPane(mpAnmObjs[3], "N_total_01", true);
                    mpAnmObjs[3]->restart();
                } else if (winner == 1) {
                    mpLayout->unbindAnmObj(mpAnmObjs[4]);
                    mpLayout->bindAnmObjToPane(mpAnmObjs[4], "N_total_02", true);
                    mpAnmObjs[4]->restart();
                }
            }
            mState = 6;
        }
    } else if (mState == 6 && mpAnmObjs[5]->isFinished()) {
        mState = 0;
        unk3D = true;
    }
}

void RPTnkResultLyt::PlayRowSound(s32 row) {
    switch (row) {
    case 0: RPSndAudioMgr::instance()->startSystemSe(0x34); break;
    case 1: RPSndAudioMgr::instance()->startSystemSe(0x33); break;
    case 2: RPSndAudioMgr::instance()->startSystemSe(0x32); break;
    case 3: RPSndAudioMgr::instance()->startSystemSe(0x31); break;
    case 4: RPSndAudioMgr::instance()->startSystemSe(0x30); break;
    case 5: RPSndAudioMgr::instance()->startSystemSe(0x2f); break;
    case 6: RPSndAudioMgr::instance()->startSystemSe(0x2e); break;
    case 7: RPSndAudioMgr::instance()->startSystemSe(0x2d); break;
    case 8: RPSndAudioMgr::instance()->startSystemSe(0x2c); break;
    }
}

void RPTnkResultLyt::Load(EGG::Archive* pArchive) {
    RPTnkResultAnmNames names = ResultAnimations();
    AttachArcFile(pArchive, "tank.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "tnkResult_00.brlyt");
    for (int i = 0; i < 8; i++) {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, names.names[i]);
        if (static_cast<u32>(i - 3) <= 1) {
            mpAnmObjs[i]->setPlayMode(RPSysFrameCtrl::EPlayMode_Loop);
        } else {
            mpAnmObjs[i]->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
        }
    }
}

#pragma opt_unroll_loops off
void RPTnkResultLyt::Reset() {
    mState = 0;
    mTimer = 0;
    mRow = 0;
    unk38 = 0;
    unk3C = false;
    unk3D = false;
    int i = 0;
    do {
        int j = 0;
        do {
            mCountText[0][i][j] = 0;
            mCountText[1][i][j] = 0;
        } while (++j < 4);
    } while (++i < 9);
    int total = 0;
    do {
        mTotalText[0][total] = 0;
        mTotalText[1][total] = 0;
    } while (++total < 4);
    mpLayout->reset();
    int anm = 0;
    do {
        mpAnmObjs[anm]->resetFrame();
        mpAnmObjs[anm]->stop();
        mpAnmObjs[anm]->setFrame(0.0f);
    } while (++anm < 8);
    RPSysMessage* msg = RPTnkLayoutManager::sInstance->GetMessage();
    mpLayout->findTextBox("T_result00_00")->setMessage(msg, 0xb67, 0, 0);
    mpLayout->findTextBox("T_result00_01")->setMessage(msg, 0xb67, 0, 0);
    mpLayout->findTextBox("T_result01_00")->setMessage(msg, 0xb68, 0, 0);
    mpLayout->findTextBox("T_result01_01")->setMessage(msg, 0xb68, 0, 0);
    mpLayout->findTextBox("T_player00_00")->setMessage(msg, 0xb65, 0, 0);
    mpLayout->findTextBox("T_player01_00")->setMessage(msg, 0xb64, 0, 0);
    mpLayout->findTextBox("T_player01_01")->setMessage(msg, 0xb64, 1, 0);
    mpLayout->findTextBox("T_player00_01")->setMessage(msg, 0xb66, 0, 0);
    mpLayout->findTextBox("T_player00_02")->setMessage(msg, 0xb66, 1, 0);
    mpLayout->unbindAllAnmObj();
    HideAllPanes();
}

#pragma opt_unroll_loops on
void RPTnkResultLyt::Calculate() {
    if (RPTnkGameManager::sInstance->GetModeIdx() == 0) {
        CalcSequence1P();
    } else {
        CalcSequence2P();
    }
    if (mState == 4) {
        ++mTimer;
        if (mTimer >= 20) {
            EGG::CoreController* controller = EGG::CoreControllerMgr::instance()->getNthController(0);
            bool pressed;
            if (controller->downTrigger(0x800) || controller->downTrigger(0x400)) {
                pressed = true;
            } else {
                if (RPTnkGameManager::sInstance->GetModeIdx() == 1) {
                    controller = EGG::CoreControllerMgr::instance()->getNthController(1);
                    if (controller->downTrigger(0x800) || controller->downTrigger(0x400)) {
                        pressed = true;
                        goto inputDone;
                    }
                }
                pressed = false;
            }
inputDone:
            if (pressed) mTimer = 300;
        }
        if (mTimer >= 300) {
            if (RPTnkGameManager::sInstance->GetModeIdx() == 0) CloseList1P();
            else CloseList2P();
        }
    }
    mpLayout->calc();
}
void RPTnkResultLyt::Draw() { mpLayout->draw(); }
void RPTnkResultLyt::VF_0x18() {}
// The kill-count texts are set in inline members: their locals get the original's stack slots
// (names1 below names2) and registers. Each count is read into a named local before the text
// pointer (kills, text): the loop counter is then the inline's top temporary, coloured before the
// kill-table walker, which comes before the text and name walkers, as in the original.
inline void RPTnkResultLyt::SetCountTexts1P() {
    RPTnkResultNames names = ResultNames1P();
    for (int i = 0; i < 9; i++) {
        s32 kills = RPTnkGameStats::sInstance->GetKillsByType(0, i);
        wchar_t* text = mCountText[0][i];
        RPSysStringUtility::GetStringFullSizeNumber(kills, text, 4, false);
        mpLayout->findTextBox(names.names[i])->setText(text, 0);
    }
}

inline void RPTnkResultLyt::SetCountTexts2P() {
    RPTnkResultNames names1 = ResultNames2P1();
    RPTnkResultNames names2 = ResultNames2P2();
    for (int i = 0; i < 9; i++) {
        s32 kills1 = RPTnkGameStats::sInstance->GetKillsByType(0, i);
        wchar_t* text1 = mCountText[0][i];
        RPSysStringUtility::GetStringFullSizeNumber(kills1, text1, 4, false);
        mpLayout->findTextBox(names1.names[i])->setText(text1, 0);
        s32 kills2 = RPTnkGameStats::sInstance->GetKillsByType(1, i);
        wchar_t* text2 = mCountText[1][i];
        RPSysStringUtility::GetStringFullSizeNumber(kills2, text2, 4, false);
        mpLayout->findTextBox(names2.names[i])->setText(text2, 0);
    }
}

void RPTnkResultLyt::Start() {
    if (mState == 0) {
        mTimer = -20;
        mRow = 0;
        unk38 = 0;
        unk3C = false;
        unk3D = false;
        HideAllPanes();
        if (RPTnkGameManager::sInstance->GetModeIdx() == 0) {
            SetCountTexts1P();
            RPSysStringUtility::GetStringHalfSizeNumber(RPTnkGameStats::sInstance->GetKills(0), mTotalText[0], 4, false);
            mpLayout->findTextBox("T_score00_00")->setText(mTotalText[0], 0);
            mpLayout->findTextBox("T_score01_00")->setText(mTotalText[0], 0);
            mpLayout->findPane("N_list_00")->SetVisible(true);
            mpLayout->unbindAnmObj(mpAnmObjs[0]);
            mpLayout->bindAnmObjToPane(mpAnmObjs[0], "N_list_00", true);
            mpAnmObjs[0]->restart();
        } else {
            SetCountTexts2P();
            RPSysStringUtility::GetStringHalfSizeNumber(RPTnkGameStats::sInstance->GetKills(0), mTotalText[0], 4, false);
            mpLayout->findTextBox("T_score00_01")->setText(mTotalText[0], 0);
            mpLayout->findTextBox("T_score01_01")->setText(mTotalText[0], 0);
            RPSysStringUtility::GetStringHalfSizeNumber(RPTnkGameStats::sInstance->GetKills(1), mTotalText[1], 4, false);
            mpLayout->findTextBox("T_score00_02")->setText(mTotalText[1], 0);
            mpLayout->findTextBox("T_score01_02")->setText(mTotalText[1], 0);
            mpLayout->findPane("N_list_01")->SetVisible(true);
            mpLayout->unbindAnmObj(mpAnmObjs[0]);
            mpLayout->bindAnmObjToPane(mpAnmObjs[0], "N_list_01", true);
            mpAnmObjs[0]->restart();
        }
        RPSndAudioMgr::instance()->startSystemSe(0u);
        mState = 1;
    }
}
void RPTnkResultLyt::CloseList1P() {
    mpLayout->unbindAllAnmObj();
    if (RPTnkGameManager::sInstance->GetModeIdx() == 0) {
        mpLayout->unbindAnmObj(mpAnmObjs[5]);
        mpLayout->bindAnmObjToPane(mpAnmObjs[5], "N_list_00", true);
        mpAnmObjs[5]->restart();
        mpLayout->unbindAnmObj(mpAnmObjs[6]);
        mpLayout->bindAnmObjToPane(mpAnmObjs[6], "N_totalAll_00", true);
        mpAnmObjs[6]->restart();
        mpLayout->bindAnmObjToPane(mpAnmObjs[3], "N_total_00", true);
    } else {
        mpLayout->unbindAnmObj(mpAnmObjs[5]);
        mpLayout->bindAnmObjToPane(mpAnmObjs[5], "N_list_01", true);
        mpLayout->unbindAnmObj(mpAnmObjs[6]);
        mpLayout->bindAnmObjToPane(mpAnmObjs[6], "N_totalAll_01", true);
    }
    mState = 6;
}

#pragma auto_inline off
void RPTnkResultLyt::CloseList2P() {
    if (mState == 4) {
        mpLayout->unbindAnmObj(mpAnmObjs[5]);
        if (RPTnkGameManager::sInstance->GetModeIdx() == 0) {
            mpLayout->bindAnmObjToPane(mpAnmObjs[5], "N_list_00", true);
        } else {
            mpLayout->bindAnmObjToPane(mpAnmObjs[5], "N_list_01", true);
        }
        mpAnmObjs[5]->restart();
        if (RPTnkGameManager::sInstance->GetModeIdx() == 1) {
            mpLayout->unbindAnmObj(mpAnmObjs[7]);
            mpLayout->bindAnmObjToPane(mpAnmObjs[7], "N_totalAll_01", true);
            mpAnmObjs[7]->restart();
            mState = 5;
        } else {
            mState = 6;
        }
    }
}

#pragma auto_inline on
void RPTnkResultLyt::ShowTotals() {
    mpLayout->unbindAnmObj(mpAnmObjs[6]);
    if (RPTnkGameManager::sInstance->GetModeIdx() == 0) {
        mpLayout->bindAnmObjToPane(mpAnmObjs[6], "N_totalAll_00", true);
    } else {
        mpLayout->bindAnmObjToPane(mpAnmObjs[6], "N_totalAll_01", true);
    }
    mpAnmObjs[6]->restart();
    int winner = RPTnkGameStats::sInstance->GetWinner();
    if (winner != -1) {
        if (winner == 0) {
            mpLayout->unbindAnmObj(mpAnmObjs[3]);
            mpLayout->bindAnmObjToPane(mpAnmObjs[3], "N_total_01", true);
        } else if (winner == 1) {
            mpLayout->unbindAnmObj(mpAnmObjs[4]);
            mpLayout->bindAnmObjToPane(mpAnmObjs[4], "N_total_02", true);
        }
    }
    mState = 6;
}

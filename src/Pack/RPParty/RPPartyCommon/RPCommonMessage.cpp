#include <Pack/RPParty/RPPartyCommon/RPCommonMessage.h>

#include <Pack/RPKernel/RPSysKokeshiManager.h>
#include <Pack/RPKernel/RPSysStringUtility.h>
#include <Pack/RPKernel/RPSysSystemWinMgr.h>
#include <Pack/RPSystem/RPSysSaveDataMgr.h>
#include <Pack/RPSystem/RPSysSceneMgr.h>

void RPUtlLoadSaveDataScene::BindSystems() {
    mpSaveDataMgr = RPSysSaveDataMgr::instance();
    mpSystemWinMgr = RPSysSystemWinMgr::instance();
    unk48 = NULL;
    mpSaveDataMgr->setSaveDisable(false);
}

void RPUtlLoadSaveDataScene::LoadResource() {}

void RPUtlLoadSaveDataScene::Reset() {}

void RPUtlLoadSaveDataScene::CalcCurrentState() {
    switch (mSaveState) {
    case 0: CalcState0(); break;
    case 1: CalcState1(); break;
    case 2: CalcState2(); break;
    case 3: CalcState3(); break;
    case 4: CalcState4(); break;
    case 5: CalcState5(); break;
    case 6: CalcState6(); break;
    case 7: CalcState7(); break;
    case 8: CalcState8(); break;
    case 9: CalcState9(); break;
    case 10: CalcState10(); break;
    case 11: CalcState11(); break;
    case 12: CalcState12(); break;
    case 13: CalcState13(); break;
    case 14: CalcState14(); break;
    case 15: CalcState15(); break;
    case 16: CalcState16(); break;
    }
}

void RPUtlLoadSaveDataScene::UserDraw() {}

void RPUtlLoadSaveDataScene::Exit() {}

void RPUtlLoadSaveDataScene::StartSaveCheck() {
    EnterState0();
}

void RPUtlLoadSaveDataScene::EnterState0() {
    mSaveState = 0;
}

void RPUtlLoadSaveDataScene::EnterState1() {
    mSaveState = 1;
    mpSaveDataMgr->existPackFileAsync();
}

void RPUtlLoadSaveDataScene::EnterState2() {
    mSaveState = 2;
    mpSaveDataMgr->loadAsync();
}

void RPUtlLoadSaveDataScene::EnterState3() {
    mSaveState = 3;
}

void RPUtlLoadSaveDataScene::EnterState4() {
    mSaveState = 4;
}

void RPUtlLoadSaveDataScene::EnterState5() {
    mSaveState = 5;
    mpSaveDataMgr->existNandMemoryAsync();
}

void RPUtlLoadSaveDataScene::EnterState6() {
    mSaveState = 6;
    mpSystemWinMgr->setSystemWindow(RPSysSystemWinMgr::E_WINDOW_TYPE_TEXT, 105, NULL, 1,
                                    unk48);
}

void RPUtlLoadSaveDataScene::EnterState7() {
    mSaveState = 7;
    if (mpSaveDataMgr->existPackFileSync() && mpSaveDataMgr->isPackFileExist()) {
        mpSaveDataMgr->deletePackFileSync();
    }
    mpSaveDataMgr->createPackFileAsync();
}

void RPUtlLoadSaveDataScene::EnterState8() {
    mSaveState = 8;
    mpSystemWinMgr->setSystemWindow(RPSysSystemWinMgr::E_WINDOW_TYPE_TEXT, 107, NULL, 0);
}

void RPUtlLoadSaveDataScene::EnterState9() {
    mSaveState = 9;
    mpSystemWinMgr->setSystemWindow(RPSysSystemWinMgr::E_WINDOW_TYPE_ERROR, 160, NULL, 0);
}

void RPUtlLoadSaveDataScene::EnterState10() {
    mSaveState = 10;
    mpSystemWinMgr->setSystemWindow(RPSysSystemWinMgr::E_WINDOW_TYPE_ERROR, 53, NULL, 0);
}

void RPUtlLoadSaveDataScene::EnterState11() {
    mSaveState = 11;
    wchar_t number[16];
    RPSysStringUtility::GetStringHalfSizeNumber(mpSaveDataMgr->getUserBlockSize(), number,
                                               16, false);
    mpSystemWinMgr->setSystemWindow(RPSysSystemWinMgr::E_WINDOW_TYPE_ERROR, 104, NULL, 1,
                                    number);
}

void RPUtlLoadSaveDataScene::EnterState12() {
    mSaveState = 12;
    wchar_t number[16];
    RPSysStringUtility::GetStringHalfSizeNumber(mpSaveDataMgr->getSaveFileNum(), number,
                                               16, false);
    mpSystemWinMgr->setSystemWindow(RPSysSystemWinMgr::E_WINDOW_TYPE_ERROR, 110, NULL, 1,
                                    number);
}

void RPUtlLoadSaveDataScene::EnterState13() {
    mSaveState = 13;
    mpSystemWinMgr->setSystemWindow(RPSysSystemWinMgr::E_WINDOW_TYPE_ERROR, 101, NULL, 0);
}

void RPUtlLoadSaveDataScene::EnterState14() {
    mSaveState = 14;
    mpSystemWinMgr->setSystemWindow(RPSysSystemWinMgr::E_WINDOW_TYPE_ERROR, 100, NULL, 0);
}

void RPUtlLoadSaveDataScene::EnterState15() {
    mSaveState = 15;
    mpSystemWinMgr->setSystemWindow(RPSysSystemWinMgr::E_WINDOW_TYPE_ERROR, 170, NULL, 0);
}

void RPUtlLoadSaveDataScene::EnterState16() {
    mSaveState = 16;
}

void RPUtlLoadSaveDataScene::CalcState0() {
    switch (RPSysKokeshiManager::instance()->getLoadError()) {
    case 0: EnterState1(); break;
    case 1: EnterState9(); break;
    case 2: EnterState15(); break;
    }
}

void RPUtlLoadSaveDataScene::CalcState1() {
    if (mpSaveDataMgr->isNandAccessFinish()) {
        if (mpSaveDataMgr->isPackFileExist()) {
            EnterState2();
        } else if (mpSaveDataMgr->isErrorOccured()) {
            if (mpSaveDataMgr->getErrorCode() == -3) {
                EnterState15();
            } else if (mpSaveDataMgr->getErrorCode() == -4) {
                EnterState10();
            } else {
                EnterState14();
            }
        } else {
            EnterState5();
        }
    }
}

void RPUtlLoadSaveDataScene::CalcState2() {
    if (mpSaveDataMgr->isNandAccessFinish()) {
        if (mpSaveDataMgr->isErrorOccured()) {
            if (mpSaveDataMgr->getErrorCode() == -3) {
                EnterState15();
            } else if (mpSaveDataMgr->getErrorCode() == -4) {
                EnterState10();
            } else {
                EnterState14();
            }
        } else {
            EnterState3();
        }
    }
}

void RPUtlLoadSaveDataScene::CalcState3() {
    if (mpSaveDataMgr->isSaveFileBroken()) {
        EnterState14();
    } else {
        EnterState4();
    }
}

void RPUtlLoadSaveDataScene::CalcState5() {
    if (mpSaveDataMgr->isNandAccessFinish()) {
        if (mpSaveDataMgr->isNandMemoryExist()) {
            EnterState6();
        } else if (mpSaveDataMgr->isErrorOccured()) {
            if (mpSaveDataMgr->getErrorCode() == -3) {
                EnterState15();
            } else {
                EnterState10();
            }
        } else if ((mpSaveDataMgr->getNandCheckAnswer() & 1) ||
                   (mpSaveDataMgr->getNandCheckAnswer() & 4)) {
            EnterState11();
        } else {
            EnterState12();
        }
    }
}

void RPUtlLoadSaveDataScene::CalcState6() {
    switch (mpSystemWinMgr->getResult()) {
    case RPSysSystemWinMgr::RESULT_YES: EnterState7(); break;
    case RPSysSystemWinMgr::RESULT_NO: EnterState13(); break;
    }
}

void RPUtlLoadSaveDataScene::CalcState7() {
    if (mpSaveDataMgr->isNandAccessFinish()) {
        if (mpSaveDataMgr->isPackFileExist()) {
            EnterState8();
        } else if (mpSaveDataMgr->isErrorOccured()) {
            if (mpSaveDataMgr->getErrorCode() == -3) {
                EnterState15();
            } else {
                EnterState10();
            }
        } else {
            EnterState10();
        }
    }
}

void RPUtlLoadSaveDataScene::CalcState8() {
    switch (mpSystemWinMgr->getResult()) {
    case RPSysSystemWinMgr::RESULT_CONTINUE: EnterState4(); break;
    }
}

void RPUtlLoadSaveDataScene::CalcState9() {
    switch (mpSystemWinMgr->getResult()) {
    case RPSysSystemWinMgr::RESULT_CONTINUE: EnterState16(); break;
    }
}

void RPUtlLoadSaveDataScene::CalcState10() {
    switch (mpSystemWinMgr->getResult()) {
    case RPSysSystemWinMgr::RESULT_YES:
        mpSaveDataMgr->setSaveDisable(true);
        EnterState4();
        break;
    case RPSysSystemWinMgr::RESULT_NO: EnterState16(); break;
    }
}

void RPUtlLoadSaveDataScene::CalcState11() {
    switch (mpSystemWinMgr->getResult()) {
    case RPSysSystemWinMgr::RESULT_YES: EnterState16(); break;
    case RPSysSystemWinMgr::RESULT_NO: EnterState13(); break;
    }
}

void RPUtlLoadSaveDataScene::CalcState12() {
    switch (mpSystemWinMgr->getResult()) {
    case RPSysSystemWinMgr::RESULT_YES: EnterState16(); break;
    case RPSysSystemWinMgr::RESULT_NO: EnterState13(); break;
    }
}

void RPUtlLoadSaveDataScene::CalcState13() {
    switch (mpSystemWinMgr->getResult()) {
    case RPSysSystemWinMgr::RESULT_YES:
        mpSaveDataMgr->setSaveDisable(true);
        EnterState4();
        break;
    case RPSysSystemWinMgr::RESULT_NO: EnterState16(); break;
    }
}

void RPUtlLoadSaveDataScene::CalcState14() {
    switch (mpSystemWinMgr->getResult()) {
    case RPSysSystemWinMgr::RESULT_YES: EnterState7(); break;
    case RPSysSystemWinMgr::RESULT_NO: EnterState13(); break;
    }
}

void RPUtlLoadSaveDataScene::CalcState15() {
    switch (mpSystemWinMgr->getResult()) {
    case RPSysSystemWinMgr::RESULT_CONTINUE: EnterState16(); break;
    }
}

void RPUtlLoadSaveDataScene::CalcState16() {
    RPSysSceneMgr::instance()->returnToMenu(false);
}

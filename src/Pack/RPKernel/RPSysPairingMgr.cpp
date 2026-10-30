#include <Pack/RPKernel.h>
// Ported from doldecomp/ogws (CC0) src/Pack/RPKernel/RPSysPairingMgr.cpp @ f5a1bb5e; Wii Play changes marked "Wii Play:".

#include <revolution/WPAD.h>

/**
 * @brief Whether pairing is ongoing
 */
bool RPSysPairingMgr::sIsBusy = false;

/**
 * @brief WPAD simple sync callback
 *
 * @param result WPAD sync result
 * @param num Number of synced controllers
 */
void RPSysPairingMgr::simpleSyncCallback(s32 result, s32 /* num*/) {
    switch (result) {
    case WPAD_SYNC_WAITING: {
        break;
    }

    case WPAD_SYNC_BUSY: {
        sIsBusy = false;
        break;
    }

    case WPAD_SYNC_DONE: {
        sIsBusy = false;
        break;
    }

    default: {
        break;
    }
    }
}

/**
 * @brief Constructor
 */
RPSysPairingMgr::RPSysPairingMgr()
    : mPairingState(EPairingState_Start), mIsEnabled(false) {

    WPADSetSimpleSyncCallback(simpleSyncCallback);
}

/**
 * @brief Updates the pairing state
 */
void RPSysPairingMgr::calc() {
    switch (mPairingState) {
    case EPairingState_Start: {
        if (!mIsEnabled) {
            break;
        }

        if (WPADStartSimpleSync()) {
            mPairingState = EPairingState_Stop;
            sIsBusy = true;
        }
        break;
    }

    case EPairingState_Stop: {
        if (mIsEnabled) {
            break;
        }

        if (WPADStopSimpleSync()) {
            mPairingState = EPairingState_Wait;
        }
        break;
    }

    case EPairingState_Wait: {
        if (!sIsBusy) {
            mPairingState = EPairingState_Start;
        }
        break;
    }
    }
}

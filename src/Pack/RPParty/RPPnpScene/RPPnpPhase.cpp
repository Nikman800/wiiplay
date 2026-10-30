// Table Tennis point phase: fault / point messages after a rally and the two-player score.
// Unit 0x8022AA04-0x8022B074 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPPnpScene/RPPnpPhase.h>

#include <Pack/RPAudio.h>
#include <Pack/RPParty/RPPnpScene/RPPnpBall.h>
#include <Pack/RPParty/RPPnpScene/RPPnpGame.h>
#include <Pack/RPParty/RPPnpScene/RPPnpHud.h>
#include <Pack/RPParty/RPPnpScene/RPPnpLyt.h>

s32 RPPnpPointPhase::sServeChangePoints = 2;
s32 RPPnpPointPhase::sServeTurnPoints = 4;

RPPnpPointPhase::~RPPnpPointPhase() {}

void RPPnpPointPhase::Start() {
    mTimer = 0;

    if (!RPPnpGame::GetInstance()->IsMultiPlayer()) {
        mStep = 2;

        switch (RPPnpGame::GetInstance()->GetBall()->GetState()) {
        case 9:
        case 10:
        case 12:
            mStep = 1;
            RPPnpHud::GetInstance()->GetFaultMsgLyt()->Show();
            break;
        case 11:
        case 13:
            mStep = 2;
            break;
        default:
            return;
        }

        s32 rally = RPPnpGame::GetInstance()->GetBall()->GetHitCount() / 2;
        if (rally >= 1) {
            if (rally < 50) {
                RPSndAudioMgr::instance()->startSystemSe(0x3B);
            } else if (rally < 100) {
                RPSndAudioMgr::instance()->startSystemSe(0x3C);
            } else if (RPPnpGame::sRallyLimit != rally) {
                RPSndAudioMgr::instance()->startSystemSe(0x3C);
            }
        }
    } else {
        switch (RPPnpGame::GetInstance()->GetBall()->GetState()) {
        case 9:
        case 10:
        case 12:
            mStep = 1;
            RPPnpHud::GetInstance()->GetFaultMsgLyt()->Show();
            break;
        case 11:
        case 13:
            mStep = 2;
            RPPnpHud::GetInstance()->GetPointMsgLyt()->Show(mLastScorer);
            break;
        }
    }
}

void RPPnpPointPhase::Create() {
    mWinPoints = 11;
    mStep = 0;
    mTimer = 0;
    mLastScorer = 2;
    mScores[0] = 0;
    mScores[1] = 0;
}

void RPPnpPointPhase::Load() {}

void RPPnpPointPhase::Reset() {
    mStep = 0;
    mTimer = 0;
    mLastScorer = 2;
    mScores[0] = 0;
    mScores[1] = 0;
}

void RPPnpPointPhase::Calculate() {
    switch (mStep) {
    case 0:
        Start();
        break;
    case 1:
        if (RPPnpHud::GetInstance()->GetFaultMsgLyt()->IsDone()) {
            if (++mTimer >= 40) {
                mStep = 2;
                mTimer = 0;
                if (RPPnpGame::GetInstance()->IsMultiPlayer() == TRUE) {
                    RPPnpHud::GetInstance()->GetPointMsgLyt()->Show(mLastScorer);
                }
            }
        }
        break;
    case 2:
        if (!RPPnpGame::GetInstance()->IsMultiPlayer() ||
            RPPnpHud::GetInstance()->GetPointMsgLyt()->IsDone()) {
            if (++mTimer >= 40) {
                mStep = 3;
                mTimer = 0;
            }
        }
        break;
    case 3:
        break;
    }
}

void RPPnpPointPhase::Exit() {}

void RPPnpPointPhase::Draw() {}

s32 RPPnpPointPhase::GetServerSide() const {
    if (!RPPnpGame::GetInstance()->IsMultiPlayer()) {
        return 1;
    }

    s32 side = (mScores[0] + mScores[1]) % (sServeChangePoints * 2) >= sServeChangePoints;
    if (RPPnpGame::GetInstance()->IsCourtSwapped()) {
        side = !side;
    }

    return side;
}

bool RPPnpPointPhase::IsServeTurnOdd() const {
    bool odd = false;
    if (GetServeTurn() % 2 != 0) {
        odd = true;
    }

    return odd;
}

// Preserve the separate score loads and their original register allocation.
#pragma opt_propagation off
bool RPPnpPointPhase::IsCourtChangePoint() const {
    if (RPPnpGame::GetInstance()->IsMultiPlayer() == TRUE) {
        s32 points, first, second;
        bool change;
        first = mScores[0];
        change = false;
        second = mScores[1];
        points = first + second;
        if (points != 0 && points % sServeChangePoints == 0) {
            change = true;
        }

        return change;
    }

    return false;
}
#pragma opt_propagation reset

bool RPPnpPointPhase::IsServeChangePoint() const {
    if (RPPnpGame::GetInstance()->IsMultiPlayer() == TRUE) {
        bool change = false;
        s32 points = mScores[0] + mScores[1];
        if (points != 0 && points % sServeTurnPoints == 0) {
            change = true;
        }

        return change;
    }

    return false;
}

s32 RPPnpPointPhase::GetServeTurn() const {
    if (RPPnpGame::GetInstance()->IsMultiPlayer() == TRUE) {
        s32 turn = (mScores[0] + mScores[1]) / sServeTurnPoints;
        if (RPPnpGame::GetInstance()->IsCourtSwapped()) {
            turn++;
        }

        return turn;
    }

    return 0;
}

bool RPPnpPointPhase::IsGameOver() const {
    bool over = false;
    if (RPPnpGame::GetInstance()->IsMultiPlayer() == TRUE) {
        if (mScores[0] >= mWinPoints) {
            over = true;
        }

        if (mScores[1] >= mWinPoints) {
            over = true;
        }
    }

    return over;
}

void RPPnpPointPhase::AddPoint(s32 side) {
    if (RPPnpGame::GetInstance()->IsMultiPlayer() == TRUE) {
        switch (side) {
        case 0:
            if (++mScores[0] > 99) {
                mScores[0] = 99;
            }
            mLastScorer = 0;
            RPPnpHud::GetInstance()->GetScoreLyt()->PlayPointSwitch(0);
            break;
        case 1:
            if (++mScores[1] > 99) {
                mScores[1] = 99;
            }
            mLastScorer = 1;
            RPPnpHud::GetInstance()->GetScoreLyt()->PlayPointSwitch(1);
            break;
        }
    }
}

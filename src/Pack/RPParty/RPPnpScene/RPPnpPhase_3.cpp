// Table Tennis serve phase: court / server change, help messages until the serve, server marker.
// Unit 0x8022BC8C-0x8022C478 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPPnpScene/RPPnpPhase.h>

#include <Pack/RPAudio.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>
#include <Pack/RPParty/RPPnpScene/RPPnpBall.h>
#include <Pack/RPParty/RPPnpScene/RPPnpField.h>
#include <Pack/RPParty/RPPnpScene/RPPnpGame.h>
#include <Pack/RPParty/RPPnpScene/RPPnpHud.h>
#include <Pack/RPParty/RPPnpScene/RPPnpLyt.h>
#include <Pack/RPParty/RPPnpScene/RPPnpPlayer.h>

#include <cmath>

s32 RPPnpServePhase::sServeWaitFrames = 80;

RPPnpServePhase::~RPPnpServePhase() {}

void RPPnpServePhase::Start() {
    if (RPPnpGame::GetInstance()->GetPointPhase()->IsServeChangePoint()) {
        mStep = 1;
        RPPnpHud::GetInstance()->GetScoreLyt()->PlayOut();
    } else {
        mStep = 2;
    }

    if (RPPnpGame::GetInstance()->GetPointPhase()->IsCourtChangePoint()) {
        mStep = 1;

        if (RPPnpGame::GetInstance()->GetPointPhase()->GetServerSide() == 1) {
            RPSndAudioMgr::instance()->startSystemSe(0x34);
        } else {
            RPSndAudioMgr::instance()->startSystemSe(0x33);
        }

        s32 side = RPPnpGame::GetInstance()->GetPointPhase()->GetServerSide();
        if (RPPnpGame::GetInstance()->GetPointPhase()->GetServeTurn() % 2 != 0) {
            side = !side;
        }
        RPPnpHud::GetInstance()->GetServeMsgLyt()->Show(side);
    }

    for (int i = 0; i < 2; i++) {
        RPPnpGame::GetInstance()->GetPlayer(i)->PickCpuServeX();
    }

    mTimer = 0;
    mPrevServeInput = 3;
    mServeInput = 3;
}

void RPPnpServePhase::CalcCourtChange() {
    if (!RPPnpGame::GetInstance()->GetPointPhase()->IsCourtChangePoint() ||
        RPPnpHud::GetInstance()->GetServeMsgLyt()->IsDone()) {
        if (RPPnpGame::GetInstance()->GetPointPhase()->IsServeChangePoint()) {
            if (mTimer == 0) {
                RPSndAudioMgr::instance()->startSystemSe(0x35);
            }

            if (RPPnpGame::GetInstance()->GetPointPhase()->GetServeTurn() % 2 == 0) {
                RPPnpGame::GetInstance()->GetCamera()->SetTarget(
                    0.0f, 80.0f,
                    RPInterpolate(RPPnpGame::sCameraOffsetFar, RPPnpGame::sCameraOffsetNear,
                                  mTimer, sServeWaitFrames, 0));
                RPPnpGame::GetInstance()->GetCamera()->SetRotation(
                    -33.0f, RPInterpolate(-180.0f, 0.0f, mTimer, sServeWaitFrames, 0), 0.0f);
                RPPnpGame::GetInstance()->GetNetShadow()->RestartAnim(
                    RPInterpolate(10.0f, 0.0f, mTimer, sServeWaitFrames, 0));
            } else {
                RPPnpGame::GetInstance()->GetCamera()->SetTarget(
                    0.0f, 80.0f,
                    RPInterpolate(RPPnpGame::sCameraOffsetNear, RPPnpGame::sCameraOffsetFar,
                                  mTimer, sServeWaitFrames, 0));
                RPPnpGame::GetInstance()->GetCamera()->SetRotation(
                    -33.0f, RPInterpolate(0.0f, 180.0f, mTimer, sServeWaitFrames, 0), 0.0f);
                RPPnpGame::GetInstance()->GetNetShadow()->RestartAnim(
                    RPInterpolate(0.0f, 10.0f, mTimer, sServeWaitFrames, 0));
            }

            if (mTimer++ >= sServeWaitFrames) {
                mStep = 2;
                mTimer = 0;
                RPPnpHud::GetInstance()->GetScoreLyt()->PlayIn();
            }
        } else {
            mStep = 2;
            mTimer = 0;
        }
    }
}

// opt_dead_assignments off fixes an r6/r7 swap (game pointer vs &mpPlayers)
#pragma push
#pragma opt_dead_assignments off
void RPPnpServePhase::CalcWaitServe() {
    s32 server = RPPnpGame::GetInstance()->GetPointPhase()->GetServerSide();
    nw4r::math::VEC3 pos = RPPnpGame::GetInstance()->GetPlayer(server)->GetPos();

    bool ready;
    if (!RPPnpGame::GetInstance()->IsMultiPlayer()) {
        ready = RPPnpGame::GetInstance()->GetPlayer(0)->IsBalloonOut();
    } else {
        ready = false;
        if (RPPnpGame::GetInstance()->GetPlayer(0)->IsBalloonOut() &&
            RPPnpGame::GetInstance()->GetPlayer(1)->IsBalloonOut()) {
            ready = true;
        }
    }

    s32 input = 0;
    if (ready) {
        input = 3;
    }
    mServeInput = input;

    if (!RPPnpGame::GetInstance()->IsMultiPlayer()) {
        mTimer = sServeWaitFrames;
    } else if (RPPnpGame::GetInstance()->GetPlayer(server)->IsCpu() != TRUE) {
        mTimer = sServeWaitFrames;
    }

    if (ready) {
        if (++mTimer >= sServeWaitFrames) {
            mTimer = sServeWaitFrames;

            if ((f32)std::fabs(pos.z) >=
                RPPnpGame::GetInstance()->GetTable()->GetLength() / 3.0f) {
                mServeInput = 1;
            } else {
                mServeInput = 2;
            }

            RPPnpGame::GetInstance()->GetBall()->PrepareServe();
        }
    } else {
        mTimer = 0;
    }

    switch (mServeInput) {
    case 0:
        if (mServeInput != mPrevServeInput) {
            RPPnpHud::GetInstance()->GetHelpMsgLyt()->SetMessage(0);
        }
        break;
    case 1:
        if (mServeInput != mPrevServeInput) {
            if (!RPPnpGame::GetInstance()->IsMultiPlayer()) {
                RPPnpHud::GetInstance()->GetHelpMsgLyt()->SetMessage(2);
            } else {
                RPPnpHud::GetInstance()->GetHelpMsgLyt()->SetMessage(1);
            }
        }
        break;
    case 2:
        if (mServeInput != mPrevServeInput) {
            RPPnpHud::GetInstance()->GetHelpMsgLyt()->SetMessage(3);
        }
        break;
    case 3:
        RPPnpHud::GetInstance()->GetHelpMsgLyt()->RequestHide();
        break;
    }

    if (RPPnpGame::GetInstance()->GetPlayer(server)->IsHidden()) {
        RPPnpHud::GetInstance()->GetServerLyt()->Show(
            RPPnpGame::GetInstance()->GetPointPhase()->GetServeTurn() % 2 != 0 ? !server : server,
            server);
    } else {
        RPPnpHud::GetInstance()->GetServerLyt()->Hide();
    }
}
#pragma pop

void RPPnpServePhase::Create() {
    mStep = 0;
    mTimer = 0;
    mPrevServeInput = 3;
    mServeInput = 3;
}

void RPPnpServePhase::Load() {}

void RPPnpServePhase::Reset() {
    mStep = 0;
    mTimer = 0;
    mPrevServeInput = 3;
    mServeInput = 3;
}

void RPPnpServePhase::Calculate() {
    if (RPPnpGame::GetInstance()->GetState() == RPPnpGame::STATE_SERVE) {
        switch (mStep) {
        case 0:
            Start();
            break;
        case 1:
            CalcCourtChange();
            break;
        case 2:
            CalcWaitServe();
            break;
        }

        mPrevServeInput = mServeInput;
    }
}

void RPPnpServePhase::Exit() {}

void RPPnpServePhase::Draw() {}

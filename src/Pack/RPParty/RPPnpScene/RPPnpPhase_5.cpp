// Table Tennis intro phase: camera fly-in to the table, balloons and the start message.
// Unit 0x802313D0-0x802317BC (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPPnpScene/RPPnpPhase.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPParty/RPPartyCommon/RPCommon.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>
#include <Pack/RPParty/RPPartyCommon/RPPauseText.h>
#include <Pack/RPParty/RPPnpScene/RPPnpField.h>
#include <Pack/RPParty/RPPnpScene/RPPnpGame.h>
#include <Pack/RPParty/RPPnpScene/RPPnpHud.h>
#include <Pack/RPParty/RPPnpScene/RPPnpLyt.h>

RPPnpIntroPhase::~RPPnpIntroPhase() {}

void RPPnpIntroPhase::CalcCamera() {
    f32 rotY = RPInterpolate(-90.0f, 0.0f, ++mTimer, 120, 2);
    f32 shadowFrame = RPInterpolate(5.0f, 0.0f, mTimer, 120, 0);

    if (RPPnpGame::GetInstance()->IsCourtSwapped()) {
        rotY = RPInterpolate(90.0f, 180.0f, mTimer, 120, 2);
        shadowFrame = RPInterpolate(5.0f, 10.0f, mTimer, 120, 0);
    }

    RPPnpGame::GetInstance()->GetCamera()->SetRotation(
        RPInterpolate(-55.0f, -33.0f, mTimer, 120, 2), rotY, 0.0f);
    RPPnpGame::GetInstance()->GetNetShadow()->RestartAnim(shadowFrame);
    RPPnpGame::GetInstance()->GetScreen()->SetFovy(
        RPInterpolate(18.0f, 15.8f, mTimer, 120, 2));

    if (mTimer >= 120) {
        mStep = 2;
        mTimer = 0;

        if (RPPnpGame::GetInstance()->IsCourtSwapped()) {
            RPPnpGame::GetInstance()->GetCamera()->SetRotation(-33.0f, 180.0f, 0.0f);
        } else {
            RPPnpGame::GetInstance()->GetCamera()->SetRotation(-33.0f, 0.0f, 0.0f);
        }

        RPPnpGame::GetInstance()->GetScreen()->SetFovy(15.8f);
    }

    if (mTimer == 40) {
        RPPnpHud::GetInstance()->GetStartMsgLyt()->Show();
    }
}

void RPPnpIntroPhase::Create() {}

void RPPnpIntroPhase::Load() {}

void RPPnpIntroPhase::Reset() {
    mStep = 0;
    mTimer = 0;
}

void RPPnpIntroPhase::Calculate() {
    if (RPPnpGame::GetInstance()->GetState() == RPPnpGame::STATE_INTRO) {
        switch (mStep) {
        case 0:
            mStep = 1;
            mTimer = 0;
            RPPnpGame::GetInstance()->GetCamera()->SetRotation(-55.0f, 0.0f, 0.0f);
            RPPnpGame::GetInstance()->GetScreen()->SetFovy(18.0f);
            RPPnpHud::GetInstance()->GetBalloonLyt()->Show();
            if (!RPPnpGame::GetInstance()->IsMultiPlayer()) {
                RPCommon::GetInstance()->GetPauseText()->SetEnabled(1, true);
            }
            break;
        case 1:
            CalcCamera();
            break;
        case 2:
            break;
        }
    }
}

void RPPnpIntroPhase::Exit() {}

void RPPnpIntroPhase::Draw() {}

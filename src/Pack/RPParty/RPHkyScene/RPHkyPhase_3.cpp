// Laser Hockey result phase: medal result (one player) or winner screen (two players), then
// the result menu.
// Unit 0x80220034-0x802202BC (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPHkyScene/RPHkyPhase.h>

#include <Pack/RPParty/RPHkyScene/RPHkyGame.h>
#include <Pack/RPParty/RPHkyScene/RPHkyHud.h>
#include <Pack/RPParty/RPHkyScene/RPHkyLyt.h>
#include <Pack/RPParty/RPHkyScene/RPHkyPaddle.h>
#include <Pack/RPParty/RPPartyCommon/RPCommon.h>
#include <Pack/RPParty/RPPartyCommon/RPMedalResult.h>
#include <Pack/RPParty/RPPartyCommon/RPResultMenu.h>
#include <Pack/RPParty/RPPartyCommon/RPVsWinner.h>

RPHkyResultPhase::~RPHkyResultPhase() {}

void RPHkyResultPhase::Start() {
    mTimer = 0;

    s32 score0 = RPHkyGame::GetInstance()->GetPaddle(0)->GetScore();
    s32 score1 = RPHkyGame::GetInstance()->GetPaddle(1)->GetScore();

    mWinner = -1;
    if (score0 > score1) {
        mWinner = 0;
    } else if (score0 < score1) {
        mWinner = 1;
    }

    if (!RPHkyGame::GetInstance()->IsMultiPlayer()) {
        mStep = 1;
        RPCommon::GetInstance()->GetMedalResult()->Start(score0 <= score1 ? 0 : score0);
        RPHkyHud::GetInstance()->GetScoreLyt()->PlayOut();
    } else {
        mStep = 2;
        RPCommon::GetInstance()->GetVsWinner()->Start(mWinner);
    }
}

void RPHkyResultPhase::Create() {}

void RPHkyResultPhase::Load() {}

void RPHkyResultPhase::Reset() {
    mStep = 0;
    mTimer = 0;
    mWinner = -1;
    mRetry = true;
}

void RPHkyResultPhase::Calculate() {
    if (RPHkyGame::GetInstance()->GetState() == 5) {
        switch (mStep) {
        case 0:
            Start();
            break;
        case 1:
            if (RPCommon::GetInstance()->GetMedalResult()->GetState() == 4) {
                mStep = 3;
                mTimer = 0;
                RPCommon::GetInstance()->GetResultMenu()->Start(1, -1);
            }
            break;
        case 2:
            if (RPCommon::GetInstance()->GetVsWinner()->GetState() == 5) {
                mStep = 3;
                mTimer = 0;
                RPCommon::GetInstance()->GetResultMenu()->Start(2, mWinner);
                RPHkyHud::GetInstance()->GetScoreLyt()->PlayOut();
            }
            break;
        case 3:
            if (RPCommon::GetInstance()->GetResultMenu()->GetState() == 4) {
                if (RPCommon::GetInstance()->GetResultMenu()->GetChoice() == 0) {
                    mRetry = true;
                } else {
                    mRetry = false;
                }
                mStep = 4;
                mTimer = 0;
            }
            break;
        case 4:
            break;
        }
    }
}

void RPHkyResultPhase::Exit() {}

void RPHkyResultPhase::Draw() {}

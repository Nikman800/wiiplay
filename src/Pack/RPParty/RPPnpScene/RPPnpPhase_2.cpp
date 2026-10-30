// Table Tennis result phase: camera pull-back, rally fanfare and confetti (one player) or the
// winner screen (two players), then the result menu.
// Unit 0x8022B074-0x8022BC8C (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPPnpScene/RPPnpPhase.h>

#include <Pack/RPAudio.h>
#include <Pack/RPParty/RPPartyCommon/RPCommon.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>
#include <Pack/RPParty/RPPartyCommon/RPMedalResult.h>
#include <Pack/RPParty/RPPartyCommon/RPResultMenu.h>
#include <Pack/RPParty/RPPartyCommon/RPVsWinner.h>
#include <Pack/RPParty/RPPnpScene/RPPnpAudience.h>
#include <Pack/RPParty/RPPnpScene/RPPnpBgmCtrl.h>
#include <Pack/RPParty/RPPnpScene/RPPnpGame.h>
#include <Pack/RPParty/RPPnpScene/RPPnpHud.h>
#include <Pack/RPParty/RPPnpScene/RPPnpLyt.h>

s32 RPPnpResultPhase::sResultFrames = 180;
f32 RPPnpResultPhase::sCameraDistance = -1350.0f;

RPPnpResultPhase::~RPPnpResultPhase() {}

void RPPnpResultPhase::Start() {
    mStep = 1;
    mTimer = 0;

    s32 score0 = RPPnpGame::GetInstance()->GetPointPhase()->GetScore(0);
    s32 score1 = RPPnpGame::GetInstance()->GetPointPhase()->GetScore(1);

    mWinner = -1;
    if (score0 > score1) {
        mWinner = 0;
    } else if (score0 < score1) {
        mWinner = 1;
    }

    mStartCameraValue = RPPnpGame::GetInstance()->GetCamera()->GetDistance();

    if (!RPPnpGame::GetInstance()->IsMultiPlayer()) {
        RPPnpReadyPhase::FadeOutBgm();
        RPPnpHud::GetInstance()->GetResultLyt()->Show();

        s32 rally = RPPnpGame::GetInstance()->GetRallyCount();
        if (rally < 100) {
            RPPnpAudience::GetInstance()->PlayLoopAnimA(-1);
            RPSndAudioMgr::instance()->startSystemSe(0x6F);
        } else if (rally == 100 && RPPnpGame::sRallyLimit == 100) {
            RPSndAudioMgr::instance()->startSystemSe(0x3D);
            RPPnpAudience::GetInstance()->PlayLoopAnimB(-1);

            nw4r::math::VEC3 pos(0.0f, 0.0f, 0.0f);
            nw4r::math::VEC3 scale(1.0f, 1.0f, 1.0f);
            mpEffects[0]->create();
            mpEffects[0]->setPos(pos);
            mpEffects[0]->setScale(scale);
            mpEffects[0]->update();

            RPSndAudioMgr::instance()->startSystemSe(0x6D);
        } else if ((rally > 105 && rally < 200) || (rally > 205 && rally < 300) ||
                   (rally > 305 && rally < 400) || (rally > 405 && rally < 500) ||
                   (rally > 505 && rally < 600) || (rally > 605 && rally < 700) ||
                   (rally > 705 && rally < 800) || (rally > 805 && rally < 900) ||
                   (rally > 905 && rally < 999)) {
            RPSndAudioMgr::instance()->startSystemSe(0x38);
            RPPnpAudience::GetInstance()->PlayLoopAnimB(-1);

            nw4r::math::VEC3 pos(0.0f, 0.0f, 0.0f);
            nw4r::math::VEC3 scale(1.0f, 1.0f, 1.0f);
            mpEffects[0]->create();
            mpEffects[0]->setPos(pos);
            mpEffects[0]->setScale(scale);
            mpEffects[0]->update();

            RPSndAudioMgr::instance()->startSystemSe(0x6E);
        } else if (rally == 999) {
            RPSndAudioMgr::instance()->startSystemSe(0x3D);
            RPPnpAudience::GetInstance()->PlayLoopAnimB(-1);

            nw4r::math::VEC3 pos(0.0f, 0.0f, 0.0f);
            nw4r::math::VEC3 scale(1.0f, 1.0f, 1.0f);
            mpEffects[0]->create();
            mpEffects[0]->setPos(pos);
            mpEffects[0]->setScale(scale);
            mpEffects[0]->update();
            mpEffects[1]->create();
            mpEffects[1]->setPos(pos);
            mpEffects[1]->setScale(scale);
            mpEffects[1]->update();

            RPSndAudioMgr::instance()->startSystemSe(0x6D);
        } else {
            RPPnpAudience::GetInstance()->PlayLoopAnimA(-1);
            RPSndAudioMgr::instance()->startSystemSe(0x6E);
        }

        if (rally < 100) {
            if (rally >= 40 && RPCommon::GetInstance()->GetMedalResult()->GetRankForScore(rally) >= 0) {
                RPSndAudioMgr::instance()->startSystemSe(0x6E);
            } else {
                RPSndAudioMgr::instance()->startSystemSe(0x6F);
            }
        } else if (rally < 999) {
            RPSndAudioMgr::instance()->startSystemSe(0x70);
        } else {
            RPSndAudioMgr::instance()->startSystemSe(0x71);
        }
    } else {
        RPPnpHud::GetInstance()->GetScoreLyt()->PlayOut();
        RPPnpAudience::GetInstance()->PlayLoopAnimB(-1);
    }
}

void RPPnpResultPhase::Create() {
    mpEffects[0] = new RPSysEffect("Pnp_fubuki", 2, 0);
    mpEffects[1] = new RPSysEffect("Pnp_fubuki", 2, 0);
}

void RPPnpResultPhase::Load() {}

void RPPnpResultPhase::Reset() {
    mStep = 0;
    mTimer = 0;
    mWinner = -1;
    mRetry = true;
}

void RPPnpResultPhase::Calculate() {
    if (RPPnpGame::GetInstance()->GetState() == RPPnpGame::STATE_RESULT) {
        switch (mStep) {
        case 0:
            Start();
            break;
        case 1:
            mTimer++;
            RPPnpGame::GetInstance()->GetCamera()->SetDistance(sCameraDistance);
            RPPnpGame::GetInstance()->GetCamera()->SetDistance(
                RPInterpolate(mStartCameraValue, sCameraDistance, mTimer, sResultFrames, 2));

            if (mTimer >= sResultFrames) {
                mStep = 2;
                mTimer = 0;
                RPPnpGame::GetInstance()->GetCamera()->SetDistance(sCameraDistance);
                RPPnpHud::GetInstance()->GetScoreLyt()->PlayIn();
            }
            break;
        case 2:
            if (++mTimer >= 90) {
                if (!RPPnpGame::GetInstance()->IsMultiPlayer()) {
                    mStep = 3;
                    RPCommon::GetInstance()->GetMedalResult()->Start(
                        RPPnpGame::GetInstance()->GetRallyCount());
                    RPPnpHud::GetInstance()->GetResultLyt()->PlayOut();
                    RPPnpGame::GetInstance()->GetBgmCtrl()->Stop();
                    RPSndAudioMgr::instance()->startBgm(0x72);
                } else {
                    mStep = 4;
                    RPCommon::GetInstance()->GetVsWinner()->Start(mWinner);
                }
            }
            break;
        case 3:
            if (RPCommon::GetInstance()->GetMedalResult()->GetState() == 4) {
                mStep = 5;
                mTimer = 0;
                RPCommon::GetInstance()->GetResultMenu()->Start(1, -1);
            }
            break;
        case 4:
            if (RPCommon::GetInstance()->GetVsWinner()->GetState() == 5) {
                mStep = 5;
                mTimer = 0;
                RPCommon::GetInstance()->GetResultMenu()->Start(2, mWinner);
                RPPnpHud::GetInstance()->GetScoreLyt()->PlayOut();
            }
            break;
        case 5:
            if (RPCommon::GetInstance()->GetResultMenu()->GetState() == 4) {
                if (RPCommon::GetInstance()->GetResultMenu()->GetChoice() == 0) {
                    mRetry = true;
                } else {
                    mRetry = false;
                }
                mStep = 6;
                mTimer = 0;
            }
            break;
        case 6:
            break;
        }
    }
}

void RPPnpResultPhase::Exit() {}

void RPPnpResultPhase::Draw() {}

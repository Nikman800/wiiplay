#include <Pack/RPParty/RPTnkScene/RPTnkLyt.h>

#include <Pack/RPGraphics/RPGrpCamera.h>
#include <Pack/RPGraphics/RPGrpRenderer.h>
#include <Pack/RPGraphics/RPGrpScreen.h>
#include <Pack/RPGraphics/RPGrpView.h>
#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPKernel/RPSysStringUtility.h>
#include <Pack/RPParty/RPPartyCommon/RPLytUtil.h>
#include <Pack/RPParty/RPPartyCommon/RPMiiIconSet.h>
#include <Pack/RPParty/RPTnkScene/RPTnkCamera.h>
#include <Pack/RPParty/RPTnkScene/RPTnkGameManager.h>
#include <Pack/RPParty/RPTnkScene/RPTnkTank.h>
#include <Pack/RPParty/RPTnkScene/RPTnkTankManager.h>

// This camera helper retains its original C symbol until its owning unit is matched.

// Screen position (in units of half the screen) of the point above a tank where its balloon goes.
inline nw4r::math::VEC2 TnkHudProjectTank(const RPTnkTank* pTank) {
    nw4r::math::VEC3 pos = pTank->GetPos();
    pos.y += 11.0f * pTank->GetScale().y;
    nw4r::math::VEC2 screen;
    RPTnkCamera::sInstance->ProjectToScreen(&screen, &pos);
    return screen;
}

struct HudPaneNames {
    const char* name[4];
};

static const HudPaneNames sBalloonP1 = {{"N_balloon_00", "N_balloon_03", "N_balloon_01", "N_balloon_02"}};
static const HudPaneNames sBalloonP2 = {{"N_balloon_04", "N_balloon_07", "N_balloon_05", "N_balloon_06"}};
static const HudPaneNames sPlayerSingle = {{"N_player_00", "N_player_03", "N_player_01", "N_player_02"}};
static const HudPaneNames sPlayerP1 = {{"N_player_00", "N_player_03", "N_player_01", "N_player_02"}};
static const HudPaneNames sPlayerP2 = {{"N_player_04", "N_player_07", "N_player_05", "N_player_06"}};

RPTnkHud::RPTnkHud() {}

RPTnkHud::~RPTnkHud() {}

void RPTnkHud::Load(EGG::Archive* pArchive) {
    const char* names[] = {
        "tnkUI_00_in_00.brlan", "tnkUI_00_point1P_00.brlan",
        "tnkUI_00_point2P_00.brlan", "tnkUI_00_out_00.brlan"
    };
    AttachArcFile(pArchive, "tank.arc", mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "tnkUI_00.brlyt");
    for (s32 i = 0; i < 4; i++) {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, names[i]);
        mpAnmObjs[i]->setPlayMode(RPSysFrameCtrl::EPlayMode_OneTime);
    }
}

void RPTnkHud::Reset() {
    mState = 1;
    s32 i = 0;
    do {
        mScoreText[0][i] = 0;
        mScoreText[1][i] = 0;
        mStageText[i] = 0;
        mEnemyText[i] = 0;
    } while (++i < 4);
    mShowBalloons = false;
    mpLayout->reset();
    s32 j = 0;
    do {
        mpAnmObjs[j]->resetFrame();
        mpAnmObjs[j]->stop();
        mpAnmObjs[j]->setFrame(0.0f);
    } while (++j < 4);
    RPSysMessage* message = RPTnkLayoutManager::sInstance->GetMessage();
    mpLayout->findTextBox("T_player_00")->setMessage(message, 0xb5f, 0, 0);
    mpLayout->findTextBox("T_player_01")->setMessage(message, 0xb5f, 1, 0);
    mpLayout->findTextBox("T_balloon00_00")->setMessage(message, 0xb60, 0, 0);
    mpLayout->findTextBox("T_balloon01_00")->setMessage(message, 0xb60, 0, 0);
    mpLayout->findTextBox("T_balloon00_01")->setMessage(message, 0xb60, 0, 0);
    mpLayout->findTextBox("T_balloon01_01")->setMessage(message, 0xb60, 0, 0);
    mpLayout->findTextBox("T_balloon00_02")->setMessage(message, 0xb60, 0, 0);
    mpLayout->findTextBox("T_balloon01_02")->setMessage(message, 0xb60, 0, 0);
    mpLayout->findTextBox("T_balloon00_03")->setMessage(message, 0xb60, 0, 0);
    mpLayout->findTextBox("T_balloon01_03")->setMessage(message, 0xb60, 0, 0);
    mpLayout->findTextBox("T_balloon00_04")->setMessage(message, 0xb60, 1, 0);
    mpLayout->findTextBox("T_balloon01_04")->setMessage(message, 0xb60, 1, 0);
    mpLayout->findTextBox("T_balloon00_05")->setMessage(message, 0xb60, 1, 0);
    mpLayout->findTextBox("T_balloon01_05")->setMessage(message, 0xb60, 1, 0);
    mpLayout->findTextBox("T_balloon00_06")->setMessage(message, 0xb60, 1, 0);
    mpLayout->findTextBox("T_balloon01_06")->setMessage(message, 0xb60, 1, 0);
    mpLayout->findTextBox("T_balloon00_07")->setMessage(message, 0xb60, 1, 0);
    mpLayout->findTextBox("T_balloon01_07")->setMessage(message, 0xb60, 1, 0);
    mpLayout->unbindAllAnmObj();
    mpLayout->unbindAnmObj(mpAnmObjs[0]);
    mpLayout->bindAnmObjToPane(mpAnmObjs[0], "N_tnUIl_00", true);
    mpAnmObjs[0]->restart();
    mpAnmObjs[0]->stop();
    mpAnmObjs[0]->setFrame(0.0f);
}

void RPTnkHud::Calculate() {
    RPSysStringUtility::GetStringFullSizeNumber(
        RPTnkGameManager::sInstance->GetStage() + 1, mStageText, 4, false);
    RPSysMessage* message = RPTnkLayoutManager::sInstance->GetMessage();
    mpLayout->findTextBox("T_mission_00")->setMessage(message, 0xb54, 0, 1, mStageText);
    mpLayout->findTextBox("T_mission_01")->setMessage(message, 0xb54, 0, 1, mStageText);
    RPSysStringUtility::GetStringFullSizeNumber(
        RPTnkTankManager::sInstance->GetEnemyAliveNum(), mEnemyText, 4, false);
    mpLayout->findTextBox("T_enemyNo_00")->setText(mEnemyText, 0);
    mpLayout->findTextBox("T_enemyNo_01")->setText(mEnemyText, 0);
    RPSysStringUtility::GetStringFullSizeNumber(
        RPTnkGameStats::sInstance->GetKills(0), mScoreText[0], 4, false);
    mpLayout->findTextBox("T_score00_00")->setText(mScoreText[0], 0);
    mpLayout->findTextBox("T_score01_00")->setText(mScoreText[0], 0);
    if (RPTnkGameManager::sInstance->GetModeIdx() == 1) {
        RPSysStringUtility::GetStringFullSizeNumber(
            RPTnkGameStats::sInstance->GetKills(1), mScoreText[1], 4, false);
        mpLayout->findTextBox("T_score00_01")->setText(mScoreText[1], 0);
        mpLayout->findTextBox("T_score01_01")->setText(mScoreText[1], 0);
    }
    struct {
        HudPaneNames p1;
        HudPaneNames p2;
    } names;
    names.p1 = sBalloonP1;
    names.p2 = sBalloonP2;
    s32 i = 0;
    const char** p1 = names.p1.name;
    const char** p2 = names.p2.name;
    do {
        mpLayout->findPane(*p1)->SetVisible(false);
        mpLayout->findPane(*p2)->SetVisible(false);
        i++;
        p1++;
        p2++;
    } while (i < 4);
    if (mShowBalloons) {
        mpLayout->findPane(names.p1.name[static_cast<s32>(mBalloonX / 90.0f)])->SetVisible(true);
        if (RPTnkGameManager::sInstance->GetModeIdx() == 1) {
            mpLayout->findPane(names.p2.name[static_cast<s32>(mBalloonY / 90.0f)])->SetVisible(true);
        }
    }
    if (mState == 2) {
        if (mpAnmObjs[0]->isFinished()) {
            mState = 0;
        }
    } else if (mState == 3 && mpAnmObjs[3]->isFinished()) {
        mState = 1;
    }
    mpLayout->calc();
}

void RPTnkHud::Draw() {
    struct {
        HudPaneNames p1;
        HudPaneNames p2;
        HudPaneNames single;
    } names;
    mpLayout->draw();
    if (mShowBalloons) {
        if (RPTnkGameManager::sInstance->GetModeIdx() == 0) {
            s32 quadrant = static_cast<s32>(mBalloonX / 90.0f);
            RPGrpScreen* screen = RPGrpRenderer::GetCurrent()->GetView(2)->GetScreen();
            names.single = sPlayerSingle;
            nw4r::lyt::Pane* pane = mpLayout->findPane(names.single.name[quadrant]);
            RPDrawKokeshiIcon(RPMiiIconSet::sInstance->GetIcon(0), pane, false, screen);
        } else {
            s32 quadrant = static_cast<s32>(mBalloonX / 90.0f);
            RPGrpScreen* screen = RPGrpRenderer::GetCurrent()->GetView(2)->GetScreen();
            names.p1 = sPlayerP1;
            nw4r::lyt::Pane* pane = mpLayout->findPane(names.p1.name[quadrant]);
            RPDrawKokeshiIcon(RPMiiIconSet::sInstance->GetIcon(0), pane, false, screen);
            s32 quadrant2 = static_cast<s32>(mBalloonY / 90.0f);
            names.p2 = sPlayerP2;
            pane = mpLayout->findPane(names.p2.name[quadrant2]);
            RPDrawKokeshiIcon(RPMiiIconSet::sInstance->GetIcon(1), pane, false, screen);
        }
    }
}

void RPTnkHud::VF_0x18() {}

void RPTnkHud::PlayIn() {
    if (mState != 0) {
        const char* const names[] = {"N_score_00", "N_score_01"};
        if (RPTnkGameManager::sInstance->GetMode() != RPTnkGameManager::MODE_2P) {
            mpLayout->findPane(names[0])->SetVisible(true);
            mpLayout->findPane(names[1])->SetVisible(false);
        } else {
            mpLayout->findPane(names[0])->SetVisible(true);
            mpLayout->findPane(names[1])->SetVisible(true);
        }
        mpLayout->unbindAllAnmObj();
        mpLayout->bindAnmObjToPane(mpAnmObjs[0], "N_tnUIl_00", true);
        mpAnmObjs[0]->restart();
        mState = 2;
    }
}

void RPTnkHud::PlayOut() {
    mpLayout->unbindAllAnmObj();
    mpLayout->bindAnmObj(mpAnmObjs[3]);
    mpAnmObjs[3]->restart();
    mState = 3;
}

void RPTnkHud::PlayScoreAnim(s32 player) {
    if (mState == 0) {
        if (player == 0) {
            mpLayout->unbindAnmObj(mpAnmObjs[1]);
            mpLayout->bindAnmObjToPane(mpAnmObjs[1], "N_score_00", true);
            mpAnmObjs[1]->restart();
        } else {
            mpLayout->unbindAnmObj(mpAnmObjs[2]);
            mpLayout->bindAnmObjToPane(mpAnmObjs[2], "N_score_01", true);
            mpAnmObjs[2]->restart();
        }
    }
}

void RPTnkHud::PlaceBalloonP1(f32 angle) {
    const char* names[] = {"N_balloon_00", "N_balloon_03", "N_balloon_01", "N_balloon_02"};
    s32 quadrant = static_cast<s32>(angle / 90.0f);
    mBalloonX = angle;
    EGG::Vector2f normalized =
        TnkHudProjectTank(RPTnkTankManager::sInstance->GetPlayerTank(0));
    normalized.x /= static_cast<f32>(static_cast<u16>(static_cast<u32>(EGG::Screen::GetSizeXMax()) >> 1));
    normalized.y /= static_cast<f32>(static_cast<u16>(static_cast<u32>(EGG::Screen::GetSizeYMax()) >> 1));
    nw4r::lyt::Pane* pane = mpLayout->findPane(names[quadrant]);
    EGG::Vector3f trans(pane->GetTranslate().x, pane->GetTranslate().y, pane->GetTranslate().z);
    trans.x = normalized.x * (0.5f * EGG::Screen::GetSizeXMax(EGG::Screen::TV_MODE_STD));
    trans.y = normalized.y * (0.5f * EGG::Screen::GetSizeYMax(EGG::Screen::TV_MODE_STD));
    pane->SetTranslate(trans);
    // The original builds a new rotation but never applies it to the pane.
    EGG::Vector3f rotate(pane->GetRotate().x, pane->GetRotate().y, pane->GetRotate().z);
    EGG::Vector3f newRotate(0.0f, 0.0f, angle);
}

void RPTnkHud::PlaceBalloonP2(f32 angle) {
    const char* names[] = {"N_balloon_04", "N_balloon_07", "N_balloon_05", "N_balloon_06"};
    s32 quadrant = static_cast<s32>(angle / 90.0f);
    mBalloonY = angle;
    EGG::Vector2f normalized =
        TnkHudProjectTank(RPTnkTankManager::sInstance->GetPlayerTank(1));
    normalized.x /= static_cast<f32>(static_cast<u16>(static_cast<u32>(EGG::Screen::GetSizeXMax()) >> 1));
    normalized.y /= static_cast<f32>(static_cast<u16>(static_cast<u32>(EGG::Screen::GetSizeYMax()) >> 1));
    nw4r::lyt::Pane* pane = mpLayout->findPane(names[quadrant]);
    EGG::Vector3f trans(pane->GetTranslate().x, pane->GetTranslate().y, pane->GetTranslate().z);
    trans.x = normalized.x * (0.5f * EGG::Screen::GetSizeXMax(EGG::Screen::TV_MODE_STD));
    trans.y = normalized.y * (0.5f * EGG::Screen::GetSizeYMax(EGG::Screen::TV_MODE_STD));
    pane->SetTranslate(trans);
    EGG::Vector3f rotate(pane->GetRotate().x, pane->GetRotate().y, pane->GetRotate().z);
    EGG::Vector3f newRotate(0.0f, 0.0f, angle);
}

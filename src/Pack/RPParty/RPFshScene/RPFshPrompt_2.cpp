// Fishing aiming arrow (RPFshAimArrow): one fishingArrow_00 layout per player, drawn at the rod
// grip while the player aims.
// Unit 0x80213248-0x802136E0 (dtk/game_units.tsv; its last function, Fsh_getStartSoundTable at
// 0x802136D4, belongs to the next file). Functions in reverse address order.
#include <Pack/RPParty/RPFshScene/RPFshPrompt.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPFshScene/RPFshPlayer.h>
#include <Pack/RPParty/RPFshScene/RPFshPlayerMgr.h>
#include <Pack/RPParty/RPFshScene/RPFshRod.h>
#include <Pack/RPParty/RPFshScene/RPFshStartLayout.h>
#include <Pack/RPParty/RPPartyCommon/RPLytObjBase.h>
#include <Pack/RPSystem.h>

RPFshAimArrow* RPFshAimArrow::sInstance;

inline RPFshAimArrow::RPFshAimArrow() {
    mpResAccessor = RPSysLytResAccessor::create(NULL);
    RPGrpRenderer::GetCurrent()->AppendDrawObject(this);
}

void RPFshAimArrow::CreateInstance() {
    sInstance = new RPFshAimArrow();
}

void RPFshAimArrow::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPFshAimArrow::~RPFshAimArrow() {}

void RPFshAimArrow::Create() {
    RPLytObjBase::AttachArcFile(RPSysResourceManager::LoadGameLocalArchive(), "fishing.arc",
                                mpResAccessor);

    int i;
    const char* pLayoutName = "fishingArrow_00.brlyt";
    const char* pRootName = "RootPane";
    for (i = 0; i < PLAYER_MAX; i++) {
        mpLayouts[i] = RPSysLayout::create(NULL, mpResAccessor, pLayoutName);
        mpRootPanes[i] = mpLayouts[i]->findPane(pRootName);

        const char* anmNames[3] = {"fishingArrow_00_arrowUpDown_00.brlan",
                                   "fishingArrow_00_arrowBlue_00.brlan",
                                   "fishingArrow_00_arrowRed_00.brlan"};
        for (int j = 0; j < 3; j++) {
            mpAnmObjs[i][j] = mpLayouts[i]->createAnmObj(NULL, mpResAccessor, anmNames[j]);
        }
    }
}

void RPFshAimArrow::Reset() {
    RPFshPlayerMgr* pPlayerMgr = RPFshPlayerMgr::GetInstance();
    for (int i = 0; i < pPlayerMgr->GetPlayerNum(); i++) {
        mpLayouts[i]->unbindAllAnmObj();

        for (int j = 0; j < 3; j++) {
            if (j == 0 || (j == 1 && i == 0) || (j == 2 && i == 1)) {
                mpLayouts[i]->bindAnmObj(mpAnmObjs[i][j]);
                mpAnmObjs[i][j]->restart();
            }
        }
    }
}

void RPFshAimArrow::Calculate() {
    if (RPFshStartLayout::GetInstance()->IsStarted()) {
        RPGrpView* pView = RPGrpRenderer::GetCurrent()->GetView(1);
        RPGrpCamera* pCamera = pView->GetCamera();
        RPGrpScreen* pScreen = pView->GetScreen();

        RPFshPlayerMgr* pPlayerMgr = RPFshPlayerMgr::GetInstance();
        for (int i = 0; i < pPlayerMgr->GetPlayerNum(); i++) {
            RPFshPlayer* pPlayer = pPlayerMgr->GetPlayer(i);
            if (pPlayer->GetState() == RPFshPlayer::STATE_AIM) {
                nw4r::math::VEC3 screenPos;
                pCamera->GetWorldToScreen(&screenPos, pPlayer->GetRod()->GetGripPos(), pScreen);
                mpRootPanes[i]->SetTranslate(screenPos);
                mpLayouts[i]->calc();
            }
        }
    }
}

void RPFshAimArrow::UserDraw() {
    if (RPGrpRenderer::GetCurrentViewNo() == 2 && RPGrpRenderer::GetDrawStep() == 5 &&
        RPFshStartLayout::GetInstance()->IsStarted()) {
        RPFshPlayerMgr* pPlayerMgr = RPFshPlayerMgr::GetInstance();
        for (int i = 0; i < pPlayerMgr->GetPlayerNum(); i++) {
            if (pPlayerMgr->GetPlayer(i)->GetState() == RPFshPlayer::STATE_AIM) {
                mpLayouts[i]->draw();
            }
        }
    }
}

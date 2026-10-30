// Fishing "yank!" prompt (RPFshYankPrompt): one pane per player, fishingYank_00.brlyt.
// Unit 0x80212DF8-0x80213248 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPFshScene/RPFshPrompt.h>

#include <Pack/RPGraphics.h>
#include <Pack/RPKernel.h>
#include <Pack/RPParty/RPFshScene/RPFshPlayerMgr.h>
#include <Pack/RPParty/RPPartyCommon/RPCls803572E0.h>
#include <Pack/RPParty/RPPartyCommon/RPLytObjBase.h>
#include <Pack/RPSystem.h>

RPFshYankPrompt* RPFshYankPrompt::sInstance;

inline RPFshYankPrompt::RPFshYankPrompt() {
    mpResAccessor = RPSysLytResAccessor::create(NULL);
    RPGrpRenderer::GetCurrent()->AppendDrawObject(this);
}

void RPFshYankPrompt::CreateInstance() {
    sInstance = new RPFshYankPrompt();
}

void RPFshYankPrompt::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPFshYankPrompt::~RPFshYankPrompt() {}

void RPFshYankPrompt::Create() {
    RPLytObjBase::AttachArcFile(RPSysResourceManager::LoadGameLocalArchive(), "fishing.arc",
                                mpResAccessor);
    mpLayout = RPSysLayout::create(NULL, mpResAccessor, "fishingYank_00.brlyt");

    const char* pAnmName = "fishingYank_00_waitAll_00.brlan";
    int i;
    const char* paneNames[PLAYER_MAX] = {"N_L_00", "N_R_00"};

    for (i = 0; i < PLAYER_MAX; i++) {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, mpResAccessor, pAnmName);
        mpLayout->bindAnmObjToPane(mpAnmObjs[i], paneNames[i], true);
    }

    RPSysMessage* pMessage = RPCls803572E0::GetInstance()->GetMessage();
    const char* textNames[PLAYER_MAX][2] = {{"T_yankL_00", "T_yankL_01"},
                                            {"T_yankR_00", "T_yankR_01"}};
    for (int i = 0; i < PLAYER_MAX; i++) {
        for (int j = 0; j < 2; j++) {
            mpLayout->findTextBox(textNames[i][j])->setMessage(pMessage, 0x7D1, 0, 0);
        }
    }

    for (int i = 0; i < PLAYER_MAX; i++) {
        mpPanes[i] = mpLayout->findPane(paneNames[i]);
    }
}

void RPFshYankPrompt::Reset() {
    for (int i = 0; i < PLAYER_MAX; i++) {
        mState[i] = 0;
        mpAnmObjs[i]->resetFrame();
        mpAnmObjs[i]->stop();
        mpAnmObjs[i]->setFrame(0.0f);
    }

    for (int i = 0; i < PLAYER_MAX; i++) {
        mpPanes[i]->SetVisible(false);
    }
}

void RPFshYankPrompt::Calculate() {
    bool shown = false;
    for (int i = 0; i < RPFshPlayerMgr::GetInstance()->GetPlayerNum(); i++) {
        if (mState[i] == 1) {
            shown = true;
        }
    }

    if (shown) {
        mpLayout->calc();
    }
}

void RPFshYankPrompt::Show(s32 player) {
    if (mState[player] == 0) {
        mState[player] = 1;
        mpPanes[player]->SetVisible(true);
        mpAnmObjs[player]->restart();
    }
}

void RPFshYankPrompt::Hide(s32 player) {
    if (mState[player] == 1) {
        mState[player] = 0;
        mpPanes[player]->SetVisible(false);
    }
}

void RPFshYankPrompt::UserDraw() {
    if (RPGrpRenderer::GetCurrentViewNo() == 4 && RPGrpRenderer::GetDrawStep() == 5) {
        bool shown = false;
        for (int i = 0; i < RPFshPlayerMgr::GetInstance()->GetPlayerNum(); i++) {
            if (mState[i] == 1) {
                shown = true;
            }
        }

        if (shown) {
            mpLayout->draw();
        }
    }
}

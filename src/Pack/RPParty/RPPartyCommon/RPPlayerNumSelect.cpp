// Player count / remote entry layout (RPRimoconEntryLyt, rimoconEntry.arc). Unit 0x801D3450-0x801D4AA8.
// Functions in reverse address order.
#include <Pack/RPParty/RPPartyCommon/RPPlayerNumSelect.h>
#include <Pack/RPKernel/RPSysFrameCtrl.h>
#include <Pack/RPKernel/RPSysLytAnmObj.h>

#include <Pack/RPKernel/RPSysLayout.h>
#include <Pack/RPKernel/RPSysLytResAccessor.h>
#include <Pack/RPKernel/RPSysLytTextBox.h>
#include <Pack/RPKernel/RPSysMessage.h>
#include <Pack/RPSystem/RPSysProjectLocal.h>
#include <Pack/RPSystem/RPSysResourceManager.h>

#include <egg/core/eggController.h>
#include <egg/math/eggVector.h>

namespace {
inline bool IsAnmPlaying(const RPSysLytAnmObj* pAnmObj) {
    if (pAnmObj->mFrameCtrl.mIsEnabled == 1) {
        return true;
    }
    return false;
}

inline bool CountSelect(const RPRimoconEntryLyt* pLyt) {
    return pLyt->mCountSelect;
}
} // namespace

RPRimoconEntryLyt::RPRimoconEntryLyt(bool countSelect) : mCountSelect(countSelect) {
    for (s32 i = 0; i < BUTTON_NUM; i++) {
        mButtonStates[i] = 3;
    }

    for (s32 i = 0; i < 4; i++) {
        mEntryAnims[i] = -1;
        unk34[i] = 0;
    }

    mpLayout = NULL;

    for (s32 i = 0; i < ANIM_NUM; i++) {
        mpAnmObjs[i] = NULL;
    }
}

void RPRimoconEntryLyt::Load(EGG::Archive* pArchive) {
    RPSysLytResAccessor* pAccessor = RPSysLytResAccessor::create(NULL);

    void* pArc;
    if (RPSysProjectLocal::instance()->getPack() == RPSysSceneCreator::EPackID_PartyPack) {
        pArc = RPSysResourceManager::GetFileFromArchive(pArchive, "rimoconEntry_pt.arc", NULL);
    } else {
        pArc = RPSysResourceManager::GetFileFromArchive(pArchive, "rimoconEntry.arc", NULL);
    }
    pAccessor->mountArchive(pArc, NULL);

    mpLayout = RPSysLayout::create(NULL, pAccessor, "rimoconEntry_00.brlyt");

    const char* anmNames[ANIM_NUM] = {
        "rimoconEntry_00_inScene_00.brlan",       "rimoconEntry_00_outScene_00.brlan",
        "rimoconEntry_00_activeButton_00.brlan",  "rimoconEntry_00_activeButton_00.brlan",
        "rimoconEntry_00_activeButton_00.brlan",  "rimoconEntry_00_activeButton_00.brlan",
        "rimoconEntry_00_activeButton_00.brlan",  "rimoconEntry_00_activeButton_00.brlan",
        "rimoconEntry_00_pushButton_00.brlan",    "rimoconEntry_00_pushButton_00.brlan",
        "rimoconEntry_00_pushButton_00.brlan",    "rimoconEntry_00_pushButton_00.brlan",
        "rimoconEntry_00_pushButton_00.brlan",    "rimoconEntry_00_pushButton_00.brlan",
        "rimoconEntry_00_offRimocon_00.brlan",    "rimoconEntry_00_offRimocon_00.brlan",
        "rimoconEntry_00_offRimocon_00.brlan",    "rimoconEntry_00_offRimocon_00.brlan",
        "rimoconEntry_00_onRimocon_00.brlan",     "rimoconEntry_00_onRimocon_00.brlan",
        "rimoconEntry_00_onRimocon_00.brlan",     "rimoconEntry_00_onRimocon_00.brlan",
        "rimoconEntry_00_inoutScene2_00.brlan",   "rimoconEntry_00_inoutRimoOn_00.brlan",
        "rimoconEntry_00_loopRimoOff_00.brlan",   "rimoconEntry_00_loopRimoOff_00.brlan",
        "rimoconEntry_00_loopRimoOff_00.brlan",   "rimoconEntry_00_loopRimoOff_00.brlan",
        "rimoconEntry_00_off_onRimo_00.brlan",    "rimoconEntry_00_off_onRimo_00.brlan",
        "rimoconEntry_00_off_onRimo_00.brlan",    "rimoconEntry_00_off_onRimo_00.brlan",
        "rimoconEntry_00_onOffButton_00.brlan",   "rimoconEntry_00_off_onLamp_00.brlan",
        "rimoconEntry_00_off_onLamp_00.brlan",    "rimoconEntry_00_off_onLamp_00.brlan",
        "rimoconEntry_00_off_onLamp_00.brlan",    "rimoconEntry_00_off_onLamp_00.brlan",
        "rimoconEntry_00_off_onLamp_00.brlan",    "rimoconEntry_00_off_onLamp_00.brlan",
        "rimoconEntry_00_off_onLamp_00.brlan",    "rimoconEntry_00_off_onLamp_00.brlan",
        "rimoconEntry_00_off_onLamp_00.brlan",    "rimoconEntry_00_off_onLamp_00.brlan",
        "rimoconEntry_00_off_onLamp_00.brlan",    "rimoconEntry_00_off_onLamp_00.brlan",
        "rimoconEntry_00_off_onLamp_00.brlan",    "rimoconEntry_00_off_onLamp_00.brlan",
        "rimoconEntry_00_off_onLamp_00.brlan",
    };
    for (s32 i = 0; i < ANIM_NUM; i++) {
        mpAnmObjs[i] = mpLayout->createAnmObj(NULL, pAccessor, anmNames[i]);
    }
}

void RPRimoconEntryLyt::Reset() {
    for (s32 i = 0; i < BUTTON_NUM; i++) {
        mButtonStates[i] = 3;
    }

    for (s32 i = 0; i < 4; i++) {
        mEntryAnims[i] = -1;
        unk34[i] = 0;
    }

    mpLayout->unbindAllAnmObj();

    mpLayout->findPane("N_play4_00")->SetVisible(CountSelect(this));
    mpLayout->findPane("N_rimo4_00")->SetVisible(CountSelect(this));
    mpLayout->findPane("N_rimocon4")->SetVisible(CountSelect(this));
    mpLayout->findPane("N_play2_00")->SetVisible(!CountSelect(this));
    mpLayout->findPane("N_rimo2_00")->SetVisible(!CountSelect(this));
    mpLayout->findPane("N_rimocon2")->SetVisible(!CountSelect(this));

    nw4r::lyt::Pane* pPane = mpLayout->findPane("N_rimoEntry_00");
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[0], pPane, true);
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[1], pPane, true);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[0], pPane, true);
    mpAnmObjs[0]->restart();

    for (s32 i = 0; i < BUTTON_NUM; i++) {
        SetButtonState(i, 0);
    }

    BindSceneAnims();
}

// In an inline, the counter is a temporary numbered above the optimizer's walker, so it is
// coloured first (r31), as in the original.
inline void RPRimoconEntryLyt::UpdateEntryAnims() {
    for (s32 i = 0; i < 4; i++) {
        if (mEntryAnims[i] != unk34[i]) {
            mEntryAnims[i] = unk34[i];
            SetRemoteIconAnim(i, unk34[i]);
            SetRemoteLampAnim(i, unk34[i]);
        }
    }
}

void RPRimoconEntryLyt::Calculate() {
    if (!IsSceneAnimPlaying()) {
        UpdateEntryAnims();
    }

    mpLayout->calc();
}

void RPRimoconEntryLyt::Draw() {
    mpLayout->draw();
}

void RPRimoconEntryLyt::SetTexts(RPSysMessage* pMessage) {
    mpLayout->findTextBox("T_playMenber_00")->setMessage(pMessage, 500, 0, 0);
    mpLayout->findTextBox("T_text_04")->setMessage(pMessage, 510, 0, 0);
    mpLayout->findTextBox("T_text_05")->setMessage(pMessage, 511, 0, 0);
    mpLayout->findTextBox("T_text_00")->setMessage(pMessage, 520, 0, 0);
    mpLayout->findTextBox("T_text_01")->setMessage(pMessage, 521, 0, 0);
    mpLayout->findTextBox("T_text_02")->setMessage(pMessage, 522, 0, 0);
    mpLayout->findTextBox("T_text_03")->setMessage(pMessage, 523, 0, 0);
    mpLayout->findTextBox("T_text_06")->setMessage(pMessage, 530, 0, 0);
    mpLayout->findTextBox("T_text_07")->setMessage(pMessage, 531, 0, 0);
    mpLayout->findTextBox("T_rimocon_00")->setMessage(pMessage, 540, 0, 0);
    mpLayout->findTextBox("T_rimocon_01")->setMessage(pMessage, 541, 0, 0);
}

void RPRimoconEntryLyt::SetRemoteState(s32 player, s32 state) {
    unk34[player] = state;
}

void RPRimoconEntryLyt::PlayEntryOutAnim() {
    nw4r::lyt::Pane* pPane = mpLayout->findPane("N_rimoEntry_00");
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[0], pPane, true);
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[1], pPane, true);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[1], pPane, false);
    mpAnmObjs[1]->restart();
}

bool RPRimoconEntryLyt::IsEntryAnimPlaying() const {
    if (mpAnmObjs[0]->mFrameCtrl.mIsEnabled == 1 || mpAnmObjs[1]->mFrameCtrl.mIsEnabled == 1) {
        return true;
    }
    return false;
}

void RPRimoconEntryLyt::SetButtonState(s32 button, s32 state) {
    if (button >= 0 && button < BUTTON_NUM) {
        if (state != mButtonStates[button]) {
            mButtonStates[button] = state;

            nw4r::lyt::Pane* pPane = NULL;
            if (mCountSelect) {
                const char* names[BUTTON_NUM] = {"P_button_00", "P_button_01", "P_button_02",
                                                 "P_button_03", "P_button_06", "P_button_07"};
                pPane = mpLayout->findPane(names[button]);
            } else {
                const char* names[BUTTON_NUM] = {"P_button_04", "P_button_05", NULL,
                                                 NULL,          "P_button_06", "P_button_07"};
                if (names[button] != NULL) {
                    pPane = mpLayout->findPane(names[button]);
                }
            }

            if (pPane != NULL) {
                s32 anms[3] = {button + 2, button + 2, button + 8};
                RPSysLayout::unbindAnmObjToPane(mpAnmObjs[anms[0]], pPane, true);
                RPSysLayout::unbindAnmObjToPane(mpAnmObjs[anms[1]], pPane, true);
                RPSysLayout::unbindAnmObjToPane(mpAnmObjs[anms[2]], pPane, true);
                s32 anm = anms[state];
                RPSysLayout::forceBindAnmObjToPane(mpAnmObjs[anm], pPane, true);

                if (state < 2) {
                    mpAnmObjs[anm]->play(-1);
                    if (state == 0) {
                        mpAnmObjs[anm]->setRate(-1.0f);
                    } else if (state == 1) {
                        mpAnmObjs[anm]->setRate(1.0f);
                    }
                } else {
                    mpAnmObjs[anm]->restart();
                }
            }
        }
    }
}

bool RPRimoconEntryLyt::IsPointOnButton(s32 button, const nw4r::math::VEC2& rPos) const {
    if (button >= 0 && button < BUTTON_NUM) {
        nw4r::lyt::Pane* pPane = NULL;
        if (mCountSelect) {
            const char* names[BUTTON_NUM] = {"B_button_00", "B_button_01", "B_button_02",
                                             "B_button_03", "B_button_06", "B_button_07"};
            if (names[button] != NULL) {
                pPane = mpLayout->findPane(names[button]);
            }
        } else {
            const char* names[BUTTON_NUM] = {"B_button_04", "B_button_05", NULL,
                                             NULL,          "B_button_06", "B_button_07"};
            if (names[button] != NULL) {
                pPane = mpLayout->findPane(names[button]);
            }
        }

        if (pPane != NULL) {
            return mpLayout->isInsideBounding(RPSysLayout::castToBounding(pPane),
                                              static_cast<const EGG::Vector2f&>(rPos));
        }
    }

    return false;
}

nw4r::lyt::Pane* RPRimoconEntryLyt::GetButtonPane(s32 button) const {
    nw4r::lyt::Pane* pPane = NULL;
    if (button >= 0 && button < BUTTON_NUM) {
        if (mCountSelect) {
            const char* names[BUTTON_NUM] = {"B_button_00", "B_button_01", "B_button_02",
                                             "B_button_03", "B_button_06", "B_button_07"};
            if (names[button] != NULL) {
                pPane = mpLayout->findPane(names[button]);
            }
        } else {
            const char* names[BUTTON_NUM] = {"B_button_04", "B_button_05", NULL,
                                             NULL,          "B_button_06", "B_button_07"};
            if (names[button] != NULL) {
                pPane = mpLayout->findPane(names[button]);
            }
        }
    }

    return pPane;
}

bool RPRimoconEntryLyt::AreButtonAnimsDone() const {
    for (s32 i = 8; i < 8 + BUTTON_NUM; i++) {
        if (IsAnmPlaying(mpAnmObjs[i])) {
            return false;
        }
    }
    return true;
}

void RPRimoconEntryLyt::ResetButton6Anim() {
    mButtonStates[4] = 0;

    nw4r::lyt::Pane* pPane = mpLayout->findPane("P_button_06");
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[6], pPane, true);
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[12], pPane, true);
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[32], pPane, true);
    RPSysLayout::forceBindAnmObjToPane(mpAnmObjs[32], pPane, true);

    RPSysLytAnmObj* pAnmObj = mpAnmObjs[32];
    if (pAnmObj->mFrameCtrl.getFrame() > 0.0f) {
        pAnmObj->play(-1);
    } else {
        pAnmObj->restart();
    }
}

void RPRimoconEntryLyt::PlaySceneAnim(s32 anim) {
    BindSceneAnims();

    if (anim != 0) {
        mpAnmObjs[22]->restart();
        mpAnmObjs[22]->setRate(1.0f);
        mpAnmObjs[23]->restart();
        mpAnmObjs[23]->setRate(1.0f);
    } else {
        RPSysLytAnmObj* pAnmObj = mpAnmObjs[22];
        pAnmObj->play(pAnmObj->getFrameMax() - 1.0f);
        mpAnmObjs[22]->setRate(-1.0f);
        pAnmObj = mpAnmObjs[23];
        pAnmObj->play(pAnmObj->getFrameMax() - 1.0f);
        mpAnmObjs[23]->setRate(-1.0f);
    }
}

bool RPRimoconEntryLyt::IsSceneAnimPlaying() const {
    if (IsAnmPlaying(mpAnmObjs[22])) {
        return true;
    }
    return false;
}

#pragma push
#pragma auto_inline off
void RPRimoconEntryLyt::BindSceneAnims() {
    nw4r::lyt::Pane* pPane = mpLayout->findPane("N_playerBg_00");
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[22], pPane, true);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[22], pPane, true);

    pPane = mpLayout->findPane("N_rimoconBg_00");
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[22], pPane, true);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[22], pPane, true);

    pPane = mpLayout->findPane("N_scene2_00");
    RPSysLayout::unbindAnmObjToPane(mpAnmObjs[23], pPane, true);
    RPSysLayout::bindAnmObjToPane(mpAnmObjs[23], pPane, true);

    mpAnmObjs[22]->restart();
    mpAnmObjs[22]->stop();
    mpAnmObjs[23]->restart();
    mpAnmObjs[23]->stop();
}
#pragma pop

void RPRimoconEntryLyt::SetRemoteIconAnim(s32 player, s32 state) {
    nw4r::lyt::Pane* pPane = NULL;
    if (mCountSelect) {
        const char* names[4] = {"P_rimo_00", "P_rimo_01", "P_rimo_02", "P_rimo_03"};
        pPane = mpLayout->findPane(names[player]);
    } else {
        const char* names[4] = {"P_rimo_04", "P_rimo_05", NULL, NULL};
        if (names[player] != NULL) {
            pPane = mpLayout->findPane(names[player]);
        }
    }

    if (pPane != NULL) {
        s32 anms[2] = {player + 14, player + 18};
        RPSysLayout::unbindAnmObjToPane(mpAnmObjs[anms[0]], pPane, true);
        RPSysLayout::unbindAnmObjToPane(mpAnmObjs[anms[1]], pPane, true);
        s32 anm = anms[state];
        RPSysLayout::forceBindAnmObjToPane(mpAnmObjs[anm], pPane, true);
        mpAnmObjs[anm]->restart();
    }
}

void RPRimoconEntryLyt::SetRemoteLampAnim(s32 player, s32 state) {
    s32 chan = EGG::CoreControllerMgr::instance()->getNthController(player)->getChannelID();
    if (chan < 0) {
        chan = 0;
    }

    nw4r::lyt::Pane* pRimocon = NULL;
    nw4r::lyt::Pane* pLamps[4] = {NULL, NULL, NULL, NULL};

    if (mCountSelect) {
        const char* rimoconNames[4] = {"P_rimocon_00", "P_rimocon_01", "P_rimocon_02",
                                       "P_rimocon_03"};
        pRimocon = mpLayout->findPane(rimoconNames[player]);

        const char* lampNames[4][4] = {
            {"P_lamp_00", "P_lamp_01", "P_lamp_02", "P_lamp_03"},
            {"P_lamp_04", "P_lamp_05", "P_lamp_06", "P_lamp_07"},
            {"P_lamp_08", "P_lamp_09", "P_lamp_10", "P_lamp_11"},
            {"P_lamp_12", "P_lamp_13", "P_lamp_14", "P_lamp_15"},
        };
        for (s32 i = 0; i < 4; i++) {
            pLamps[i] = mpLayout->findPane(lampNames[player][i]);
        }
    } else {
        const char* rimoconNames[4] = {"P_rimocon_04", "P_rimocon_05", NULL, NULL};
        if (rimoconNames[player] != NULL) {
            pRimocon = mpLayout->findPane(rimoconNames[player]);
        }

        const char* lampNames[4][4] = {
            {"P_lamp_16", "P_lamp_17", "P_lamp_18", "P_lamp_19"},
            {"P_lamp_20", "P_lamp_21", "P_lamp_22", "P_lamp_23"},
            {NULL, NULL, NULL, NULL},
            {NULL, NULL, NULL, NULL},
        };
        if (lampNames[player][0] != NULL) {
            for (s32 i = 0; i < 4; i++) {
                pLamps[i] = mpLayout->findPane(lampNames[player][i]);
            }
        }
    }

    if (pRimocon != NULL) {
        s32 anms[2] = {player + 24, player + 28};
        RPSysLayout::unbindAnmObjToPane(mpAnmObjs[anms[0]], pRimocon, true);
        RPSysLayout::unbindAnmObjToPane(mpAnmObjs[anms[1]], pRimocon, true);
        s32 anm = anms[state];
        RPSysLayout::forceBindAnmObjToPane(mpAnmObjs[anm], pRimocon, true);
        mpAnmObjs[anm]->restart();
    }

    // The lamp loop is written here (not in an inline) so i and base are named locals, coloured
    // after the pane and anim walkers; j is unsigned, so (j + base) * 4 is not merged with the
    // walker's base * 4 (the original recomputes it).
    if (pLamps[0] != NULL) {
        s32 base = (player << 2) + 33;
        for (s32 i = 0; i < 4; i++) {
            RPSysLayout::unbindAnmObjToPane(mpAnmObjs[base + i], pLamps[i], true);
            RPSysLayout::forceBindAnmObjToPane(mpAnmObjs[base + i], pLamps[i], true);
        }

        if (state == 1) {
            mpAnmObjs[base + chan]->restart();
        } else {
            for (u32 j = 0; j < 4; j++) {
                mpAnmObjs[j + base]->restart();
                mpAnmObjs[j + base]->stop();
            }
        }
    }
}

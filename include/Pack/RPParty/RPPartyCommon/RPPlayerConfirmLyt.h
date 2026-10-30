#ifndef RP_PARTY_COMMON_PLAYER_CONFIRM_LYT_H
#define RP_PARTY_COMMON_PLAYER_CONFIRM_LYT_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPLytObjBase.h>
#include <Pack/RPParty/RPPartyCommon/RPStateMachine.h>

#include <nw4r/lyt.h>
#include <nw4r/math.h>

//! @addtogroup rp_party
//! @{

// Forward declarations
class RPPlayerNameTag;
class RPSysLayout;
class RPSysLytAnmObj;

/**
 * @brief Player list with two buttons and four name tags (playerSelect.arc,
 * playerSelect_02.brlyt)
 * @details vtable 80357FD8. Size 0x70, verified (Menu_PlayerConfirm_ctor). One six-state main
 * machine (hidden, in, idle, decide, out, ...) and one six-state machine per button (inactive,
 * activate, active, deactivate, push, ...).
 * @note Wii Play-only framework class (ledger Menu_PlayerConfirmLyt); the name is a project
 * convention.
 */
class RPPlayerConfirmLyt : public RPLytObjBase {
    friend class RPPlayerConfirm;

public:
    static const int BUTTON_NUM = 2;
    static const int PLAYER_NUM = 4;

    enum FrameAnm { FRAME_ANM_IN, FRAME_ANM_OUT, FRAME_ANM_NUM };
    enum ButtonAnm { BUTTON_ANM_ACTIVE, BUTTON_ANM_PUSH, BUTTON_ANM_NUM };

public:
    RPPlayerConfirmLyt();

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14

    void Show();
    void Hide();
    bool PushButton(s32 button, u32 remote);
    bool ActivateButton(s32 button);
    void DeactivateButtons();
    s32 HitTestButtons(const nw4r::math::VEC2& rPos) const;
    nw4r::lyt::Pane* GetNameTagModelPane(s32 player) const;
    void PlayCursorSe(s32 button);

    bool IsPushDone() const {
        bool done = false;
        if (mPushedButton != -1 && mpStateMachine->IsStateSettled(2)) {
            done = true;
        }
        return done;
    }

private:
    bool IsButtonStateOrNext(s32 button, s32 state) const {
        return mpButtonStateMachines[button]->IsStateOrNext(state);
    }
    bool DeactivateButton(s32 button) {
        if (!mpStateMachine->IsStateSettled(2)) {
            return false;
        }
        if (!IsButtonStateOrNext(button, 2) && !IsButtonStateOrNext(button, 3)) {
            mpButtonStateMachines[button]->ChangeState(3);
        }
        return true;
    }

    void DeactivateOthers(s32 button) {
        for (s32 i = 0; i < BUTTON_NUM; i++) {
            if (i != button) {
                DeactivateButton(i);
            }
        }
    }

    void MainInEnter();
    void MainInCalc();
    void MainIdleEnter();
    void MainIdleCalc();
    void MainDecideEnter();
    void MainDecideCalc();
    void MainOutEnter();
    void MainOutCalc();
    void ButtonInactiveEnter();
    void ButtonInactiveCalc();
    void ButtonActivateEnter();
    void ButtonActivateCalc();
    void ButtonActiveEnter();
    void ButtonActiveCalc();
    void ButtonDeactivateEnter();
    void ButtonDeactivateCalc();
    void ButtonPushEnter();
    void ButtonPushCalc();

private:
    RPStateMachine<RPPlayerConfirmLyt>* mpStateMachine;                    // at 0x8
    RPStateMachine<RPPlayerConfirmLyt>* mpButtonStateMachines[BUTTON_NUM]; // at 0xC
    s32 mPlayerNum;                                 // at 0x14
    s32 mActiveButton;                              // at 0x18
    s32 mPushedButton;                              // at 0x1C -1 = none
    RPSysLayout* mpLayout;                          // at 0x20
    RPSysLytAnmObj* mpFrameAnmObjs[FRAME_ANM_NUM];               // at 0x24 inFrame, outFrame
    RPSysLytAnmObj* mpButtonAnmObjs[BUTTON_ANM_NUM][BUTTON_NUM]; // at 0x2C activeButton, pushButton
    nw4r::lyt::Pane* mpButtonPanes[BUTTON_NUM];     // at 0x3C P_button_NN
    nw4r::lyt::Pane* mpButtonBoundings[BUTTON_NUM]; // at 0x44 B_button_NN
    nw4r::lyt::Pane* mpPlayerPanes[PLAYER_NUM];     // at 0x4C [M]
    RPPlayerNameTag* mpNameTags[PLAYER_NUM];        // at 0x5C
    u32 mPushRemote;                                // at 0x6C speaker flags for the push SE
};

//! @}

#endif

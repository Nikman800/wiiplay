#ifndef RP_PARTY_COMMON_PLAYER_CONFIRM_H
#define RP_PARTY_COMMON_PLAYER_CONFIRM_H
#include <Pack/types_pack.h>

#include <Pack/RPKernel/IRPSysHostIOSocket.h>
#include <Pack/RPParty/RPPartyCommon/RPStateMachine.h>

#include <nw4r/lyt.h>
#include <nw4r/math.h>
#include <egg/math/eggVector.h>

//! @addtogroup rp_party
//! @{

// Forward declarations
class RPABPress;
class RPKokeshiHead;
class RPCPadCursor;
class RPMenuFrame;
class RPPlayAdviceLyt;
class RPPlayerConfirmLyt;
class RPSysMessage;

/**
 * @brief Player confirmation before a game, with the play advice and the hold-A-and-B check
 * @details vtable 80357FF8 (IRPSysHostIOSocket). Size 0x78, verified (Menu_createPlayerConfirm;
 * singleton 8043C6C0). Nine states: in, select, decide, back, advice, message (RPABPress),
 * hide, done, ... [M]. The RPABPress is only created when mNoABPress is 0.
 * @note Wii Play-only framework class (ledger Menu_PlayerConfirm); the name is a project
 * convention.
 */
class RPPlayerConfirm : public IRPSysHostIOSocket {
public:
    /**
     * @brief Players and options of one confirmation (built by RPMenuScene)
     */
    struct Params {
        Params();

        s32 playerNum;      // at 0x0
        RPKokeshiHead* pHeads[4]; // at 0x4 per player, copied to the RPABPress players
        s32 controllers[4]; // at 0x14 [M]
        s32 adviceId;       // at 0x24 -1 = no play advice
        bool checkABPress;  // at 0x28 show the hold-A-and-B check after the advice
    };

public:
    static void CreateInstance(u32 noABPress);
    static void DestroyInstance();

    explicit RPPlayerConfirm(u32 noABPress);
    virtual ~RPPlayerConfirm(); // at 0x8

    void Load();
    void Reset(RPMenuFrame* pMenuFrame);
    void Calculate();
    void Start(const Params& rParams);
    void DrawLayout();
    void DrawCursor();
    void DrawOverlay();
    nw4r::lyt::Pane* GetPlayerModelPane(s32 player) const;

private:
    void InEnter();
    void InCalc();
    void SelectEnter();
    void SelectCalc();
    void DecideEnter();
    void DecideCalc();
    void BackEnter();
    void BackCalc();
    void AdviceEnter();
    void AdviceCalc();
    void MessageEnter();
    void MessageCalc();
    void HideEnter();
    void HideCalc();
    void DoneEnter();

public:
    u32 mNoABPress;                                  // at 0x4
    Params mParams;                                  // at 0x8
    RPStateMachine<RPPlayerConfirm>* mpStateMachine; // at 0x34
    s32 mResult;                                     // at 0x38 -1 = none, 0 = OK, 1 = button 0, 2 = back [M]
private:
    s32 mSelect;                                     // at 0x3C selected button, -1 = none [M]
    s32 mLastSelect;                                 // at 0x40 restored when the pad takes over [M]
    u32 unk44;                                       // at 0x44
    s32 mHitButton;                                  // at 0x48 button under the pointer [M]
    s32 mPrevHitButton;                              // at 0x4C [M]
    bool mPadMode;                                   // at 0x50 selected with the +Control Pad [M]
    EGG::Vector2f mPointerPos;                       // at 0x54 [M]
    bool mOnBackButton;                              // at 0x5C [M]
    RPPlayerConfirmLyt* mpLayout;                    // at 0x60
    RPPlayAdviceLyt* mpAdviceLyt;                    // at 0x64
    RPMenuFrame* mpMenuFrame;                        // at 0x68
    RPCPadCursor* mpCursor;                          // at 0x6C
    RPABPress* mpABPress;                            // at 0x70

public:
    // singleton 8043C6C0; defined in an .sbss gap after RPPlayerSettingPanel_2 (no source unit yet)
    static RPPlayerConfirm* sInstance;
public:
    RPSysMessage* mpMessage;                         // at 0x74
};

//! @}

#endif

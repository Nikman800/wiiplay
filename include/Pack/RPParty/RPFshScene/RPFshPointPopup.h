#ifndef RP_PARTY_FSH_POINT_POPUP_H
#define RP_PARTY_FSH_POINT_POPUP_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPFshScene/RPFshObject.h>

//! @addtogroup rp_fsh
//! @{

// Forward declarations
class RPSysLayout;
class RPSysLytAnmObj;
class RPSysLytResAccessor;

/**
 * @brief Points shown when a fish is caught, with the bonus line (fishingPoint_00.brlyt;
 * singleton Fsh_pointPopup)
 * @details vtable 80358F20. Size 0x30, verified (Fsh_PointPopup_new). The state runs inline
 * from sStateFuncs like RPFshResultLayout.
 */
// Wii Play: direct bases avoid an extra intermediate-base destructor null check.
class RPFshPointPopup : public RP_DEBUG_STUB_1,
                       public IRPSysHostIOSocket,
                       public IRPGrpDrawObject {
public:
    enum EState {
        STATE_NONE,
        STATE_IN,
        STATE_SHOW,
        STATE_OUT,

        STATE_NUM
    };

public:
    RPFshPointPopup();
    virtual ~RPFshPointPopup(); // at 0x8

    virtual void Create();    // at 0x34
    virtual void Reset();     // at 0x38
    virtual void Calculate(); // at 0x3C
    virtual void Exit();      // at 0x40 empty, out of line

    virtual void UserDraw(); // at 0x44

    /**
     * @brief Fills in the points (and N_bonusOya_00 when a bonus applies) and requests
     * STATE_IN [M]
     */
    void Show(s32 player, s32 fishIdx); // player is unused
    void Close();
    s32 GetOutAnmLength() const;

    bool IsShowStart() const {
        return mState == STATE_SHOW && !mIsChanging && mStateTimer == 1;
    }
    bool IsShowing() const {
        return mState == STATE_SHOW && !mIsChanging;
    }
    bool IsClosed() const {
        return mState == STATE_NONE && !mIsChanging;
    }

    static void CreateInstance();
    static void DestroyInstance();
    static RPFshPointPopup* GetInstance() {
        return sInstance;
    }

private:
    void InEnter();
    void InCalc();
    void ShowEnter();
    void ShowCalc();
    void OutEnter();
    void OutCalc();

private:
    s32 mState;                         // at 0x10 EState
    s32 mNextState;                     // at 0x14
    bool mIsChanging;                   // at 0x18
    s32 mStateTimer;                    // at 0x1C
    RPSysLayout* mpLayout;              // at 0x20
    RPSysLytAnmObj* mpAnmObjs[2];       // at 0x24 inAll_00, outAll_00
    RPSysLytResAccessor* mpResAccessor; // at 0x2C

    // The state functions are a local table in Calculate (8031F9C0, .rodata)

    static RPFshPointPopup* sInstance; // 8043C770
};

//! @}

#endif

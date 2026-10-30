#ifndef RP_PARTY_FSH_MSG_H
#define RP_PARTY_FSH_MSG_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPFshScene/RPFshObject.h>
#include <Pack/RPParty/RPPartyCommon/RPStateMachine.h>

//! @addtogroup rp_fsh
//! @{

// Forward declarations
class RPSysLayout;
class RPSysLytAnmObj;
class RPSysLytResAccessor;

/**
 * @brief Message balloon fishingMsg_00.brlyt (message 0x7D3; singleton Fsh_msg00)
 * @details vtable 803590F0. Size 0x20, verified (Fsh_Msg00_new). Open/Close request STATE_IN /
 * STATE_OUT. RPFshMsg01 is the same code for fishingMsg_01.
 */
class RPFshMsg00 : public RPFshDrawObj {
public:
    enum EState {
        STATE_NONE,
        STATE_IN,
        STATE_WAIT,
        STATE_OUT,

        STATE_NUM
    };

public:
    RPFshMsg00();
    virtual ~RPFshMsg00(); // at 0x8

    virtual void UserDraw(); // at 0xC

    virtual void Create();    // at 0x14
    virtual void Reset();     // at 0x18
    virtual void Calculate(); // at 0x1C

    void Open();
    void Close();

    bool IsClosed() const {
        return mpStateMachine->IsStateSettled(STATE_NONE);
    }

    static void CreateInstance();
    static void DestroyInstance();
    static RPFshMsg00* GetInstance() {
        return sInstance;
    }

private:
    void InEnter();
    void InCalc();
    void WaitEnter();
    void WaitCalc();
    void OutEnter();
    void OutCalc();

private:
    RPStateMachine<RPFshMsg00>* mpStateMachine; // at 0x8
    RPSysLayout* mpLayout;              // at 0xC
    RPSysLytAnmObj* mpAnmObjs[3];       // at 0x10 inAll_00, waitAll_00, outAll_00
    RPSysLytResAccessor* mpResAccessor; // at 0x1C

    // The state functions are a local table in the constructor (80320498, .rodata)

    static RPFshMsg00* sInstance; // 8043C7C0
};

/**
 * @brief Message balloon fishingMsg_01.brlyt (singleton Fsh_msg01)
 * @details Opened by RPFshPlayerMgr while the start layout is playing and a player in the WAIT
 * state holds the remote farther than 2.7 (closed again below 2.5) [M].
 * vtable 80359110. Size 0x20 [H] (Fsh_Msg01_new allocates 0x20). Same layout and
 * code as RPFshMsg00.
 */
class RPFshMsg01 : public RPFshDrawObj {
public:
    enum EState {
        STATE_NONE,
        STATE_IN,
        STATE_WAIT,
        STATE_OUT,

        STATE_NUM
    };

public:
    RPFshMsg01();
    virtual ~RPFshMsg01(); // at 0x8

    virtual void UserDraw(); // at 0xC

    virtual void Create();    // at 0x14
    virtual void Reset();     // at 0x18
    virtual void Calculate(); // at 0x1C

    void Open();
    void Close();

    bool IsClosed() const {
        return mpStateMachine->IsStateSettled(STATE_NONE);
    }

    static void CreateInstance();
    static void DestroyInstance();
    static RPFshMsg01* GetInstance() {
        return sInstance;
    }

private:
    void InEnter();
    void InCalc();
    void WaitEnter();
    void WaitCalc();
    void OutEnter();
    void OutCalc();

private:
    RPStateMachine<RPFshMsg01>* mpStateMachine; // at 0x8
    RPSysLayout* mpLayout;              // at 0xC
    RPSysLytAnmObj* mpAnmObjs[3];       // at 0x10 inAll_00, waitAll_00, outAll_00
    RPSysLytResAccessor* mpResAccessor; // at 0x1C

    // The state functions are a local table in the constructor (80320598, .rodata)

    static RPFshMsg01* sInstance; // 8043C7C8
};

//! @}

#endif

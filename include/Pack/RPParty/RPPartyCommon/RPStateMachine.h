#ifndef RP_PARTY_COMMON_STATE_MACHINE_H
#define RP_PARTY_COMMON_STATE_MACHINE_H
#include <Pack/types_pack.h>

//! @addtogroup rp_party
//! @{

/**
 * @brief State machine calling owner member functions (enter / calc / exit per state)
 * @details No vtable. Size 0x28 for every owner (verified for the instantiations allocated by
 * Menu_PlayerNameTag, Menu_PlayerConfirmLyt, Menu_PlayerConfirm, Menu_PlayAdviceLyt and
 * Menu_TPtGame00; not listed in fw_rp_sizes.tsv because the probe needs a plain class name).
 * Each owner type gets its own copy of the code, which the ledger lists as RP_StateMachine2-5
 * and RP_StateMgr (plus RP_StateMachine for the shared changeState/isStateOrNext/
 * isStateSettled copies).
 * calc: if a change is pending, exit the current state (unless suppressed), move
 * current -> previous and next -> current, reset the timer and enter; then calc and
 * timer++ (saturating).
 * @note Wii Play-only framework class; the name is a project convention.
 */
template <typename T> class RPStateMachine {
public:
    typedef void (T::*StateFunc)();

public:
    // Wii Play: the members are not inline (Fishing calls each instantiation's copy)
    RPStateMachine(T* pOwner, s32 stateNum);

    /**
     * @brief Sets the enter and calc functions of a state (exit functions stay NULL) [H]
     */
    void SetStateFuncs(const StateFunc& rEnter, const StateFunc& rCalc, s32 state);

    void Reset(s32 state);
    void ChangeState(s32 state);
    void Calculate();

    s32 GetState() const {
        return mState;
    }
    s32 GetPrevState() const {
        return mPrevState;
    }
    s32 GetTimer() const {
        return mTimer;
    }
    bool IsChanging() const {
        return mIsChanging;
    }
    // Wii Play: RPFshStartLayout restarts the wait timer while nobody has cast
    void ResetTimer() {
        mTimer = 0;
    }

    /**
     * @brief In the state, or changing to it [H]
     */
    bool IsStateOrNext(s32 state) const;

    /**
     * @brief In the state and not changing away from it [H]
     */
    bool IsStateSettled(s32 state) const;

private:
    void CallExit();
    void CallCalc();
    void CallEnter();

public:
    s32 mState;              // at 0x0
    s32 mPrevState;          // at 0x4
    s32 mNextState;          // at 0x8
private:
    s32 mTimer;              // at 0xC frames in the current state
public:
    bool mIsChanging;        // at 0x10
private:
    bool mCallExit;          // at 0x11
    T* mpOwner;              // at 0x14
    StateFunc* mpEnterFuncs; // at 0x18
    StateFunc* mpCalcFuncs;  // at 0x1C
    StateFunc* mpExitFuncs;  // at 0x20
    s32 mStateNum;           // at 0x24
};

template <typename T> RPStateMachine<T>::RPStateMachine(T* pOwner, s32 stateNum) {
    mStateNum = stateNum;
    mpOwner = pOwner;
    mpEnterFuncs = new StateFunc[mStateNum];
    mpCalcFuncs = new StateFunc[mStateNum];
    mpExitFuncs = new StateFunc[mStateNum];

    for (s32 i = 0; i < mStateNum; i++) {
        mpEnterFuncs[i] = NULL;
        mpCalcFuncs[i] = NULL;
        mpExitFuncs[i] = NULL;
    }
}

template <typename T>
void RPStateMachine<T>::SetStateFuncs(const StateFunc& rEnter, const StateFunc& rCalc,
                                      s32 state) {
    mpEnterFuncs[state] = rEnter;
    mpCalcFuncs[state] = rCalc;
}

template <typename T> void RPStateMachine<T>::Reset(s32 state) {
    mPrevState = state;
    mState = state;
    mNextState = state;
    mIsChanging = true;
    mCallExit = false;
    mTimer = 0;
}

template <typename T> void RPStateMachine<T>::ChangeState(s32 state) {
    mNextState = state;
    mIsChanging = true;
    mCallExit = true;
}

template <typename T> void RPStateMachine<T>::Calculate() {
    if (mIsChanging) {
        if (mCallExit) {
            CallExit();
        }

        mPrevState = mState;
        mState = mNextState;
        mIsChanging = false;
        mTimer = 0;
        CallEnter();
    }

    CallCalc();

    if (mTimer < 0x7FFFFFFF) {
        mTimer++;
    }
}

template <typename T> bool RPStateMachine<T>::IsStateOrNext(s32 state) const {
    return state == mState || (state == mNextState && mIsChanging);
}

template <typename T> bool RPStateMachine<T>::IsStateSettled(s32 state) const {
    return state == mState && (state == mNextState || !mIsChanging);
}

template <typename T> void RPStateMachine<T>::CallExit() {
    if (mpExitFuncs[mState] != NULL) {
        (mpOwner->*mpExitFuncs[mState])();
    }
}

template <typename T> void RPStateMachine<T>::CallCalc() {
    if (mpCalcFuncs[mState] != NULL) {
        (mpOwner->*mpCalcFuncs[mState])();
    }
}

template <typename T> void RPStateMachine<T>::CallEnter() {
    if (mpEnterFuncs[mState] != NULL) {
        (mpOwner->*mpEnterFuncs[mState])();
    }
}

//! @}

#endif

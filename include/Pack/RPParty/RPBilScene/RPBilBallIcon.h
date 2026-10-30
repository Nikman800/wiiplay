#ifndef RP_PARTY_BIL_BALL_ICON_H
#define RP_PARTY_BIL_BALL_ICON_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPBilScene/RPBilObject.h>
#include <Pack/RPParty/RPPartyCommon/RPStateMachine.h>

#include <egg/math.h>

#include <nw4r/math.h>

//! @addtogroup rp_bil
//! @{

/**
 * @brief Icon of a pocketed ball that flies in on the 2D screen and out again (ten made by
 * RPBilBallMgr)
 * @details vtable 8035E538. Size 0x88, verified (Bil_BallMgr_ctor). Does not override
 * UserDraw: the bad_ball model (with a 2D helper) is drawn by its model scene. Show sets the
 * scale from a per-row table (1, 1, 1, 1, 0.8, 0.7, 0.6, 0.52, 0.44 times sBaseScale) and
 * starts STATE_ENTER (sEnterFrames); STATE_LEAVE takes sLeaveFrames. Each frame the icon
 * slides in from / out to the screen edge, offset by its slot in the row, and Calculate
 * projects the screen position to the world and faces the model to the 2D camera.
 */
// Wii Play: derives from RPActorBase directly (an RPBilActor level adds a null check to the
// destructor); its own virtuals take RPBilActor's slots.
class RPBilBallIcon : public RPActorBase {
public:
    enum EState {
        STATE_NONE,
        STATE_ENTER,
        STATE_WAIT,
        STATE_LEAVE,

        STATE_NUM
    };

public:
    explicit RPBilBallIcon(s32 index);
    virtual ~RPBilBallIcon(); // at 0x8

    virtual void Load();      // at 0x78
    virtual void Reset();     // at 0x7C
    virtual void Calculate(); // at 0x80
    virtual void Exit();      // at 0x84

    /**
     * @brief Screen offset of the icon's slot in its row (the base position minus half a row,
     * plus the slot index, in icon sizes) [M]
     */
    EGG::Vector2f GetRowOffset() const;
    void Hide();
    void Show(f32 y, u32 player, s32 row, s32 slot);

    /**
     * @brief Shown (STATE_WAIT) or hidden (STATE_NONE), not moving (RPBilBallMgr::AreIconsDone)
     * [M]
     */
    bool IsSettled() const {
        return mpStateMachine->IsStateSettled(STATE_WAIT) ||
               mpStateMachine->IsStateSettled(STATE_NONE);
    }

private:
    void EnterEnter();
    void EnterCalc();
    void WaitEnter();
    void WaitCalc();
    void LeaveEnter();
    void LeaveCalc();

private:
    s32 mIndex;                                    // at 0x60
    u32 mPlayer;                                   // at 0x64 [L]
    s32 mRow;                                      // at 0x68 1-based number of icons in the row [M]
    s32 mSlot;                                     // at 0x6C index in the row [M]
    nw4r::math::VEC2 mScreenPos;                   // at 0x70 projected by Calculate
    nw4r::math::VEC2 mBasePos;                     // at 0x78 (0, y) from Show
    f32 mSize;                                     // at 0x80 scale * 2.85 * 2 * 1.2
public:
    RPStateMachine<RPBilBallIcon>* mpStateMachine; // at 0x84
private:

    /**
     * @brief State functions, 80327DC8 (copied to the stack by the constructor)
     */
    static const RPBilStateFuncs<RPBilBallIcon> sStateFuncs[STATE_NUM];

    static f32 sBaseScale;   // 8043B908 20.0
    static s32 sEnterFrames; // 8043B90C 20
    static s32 sLeaveFrames; // 8043B910 15
};

//! Ledger Bil_IconSM (size 0x28)
typedef RPStateMachine<RPBilBallIcon> RPBilIconSeq;

//! @}

#endif

#ifndef RP_PARTY_FSH_BONUS_FISH_H
#define RP_PARTY_FSH_BONUS_FISH_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPFshScene/RPFshObject.h>
#include <Pack/RPParty/RPPartyCommon/RPStateMachine.h>

#include <nw4r/lyt.h>

//! @addtogroup rp_fsh
//! @{

// Forward declarations
class RPGrpModel;
class RPSysLayout;
class RPSysLytAnmObj;
class RPSysLytResAccessor;

/**
 * @brief Spinning "bonus fish" indicator (fishingBonus_00.brlyt plus one model per fish type;
 * singleton Fsh_bonusFish)
 * @details vtable 80359060. Size 0xC8, verified (Fsh_BonusFish_new). Catching a fish of mType
 * multiplies its points by mBonus (2 after Reset). Every mInterval frames (900 after Reset) the
 * indicator spins down and back up to the next type (never sukaA). Create, Reset and Calculate
 * are not virtual here (RPFshGame calls them directly).
 */
class RPFshBonusFish : public RPActorBase {
public:
    enum EState {
        STATE_NONE,
        STATE_FADE_IN,
        STATE_SHOW,
        STATE_SPIN_DOWN,
        STATE_SPIN_UP,
        STATE_FADE_OUT,

        STATE_NUM
    };

    enum {
        FISH_TYPE_NUM = 6
    };

public:
    RPFshBonusFish();
    virtual ~RPFshBonusFish(); // at 0x8

    /**
     * @brief IRPGrpDrawObject::UserDraw override [M]
     */
    virtual void UserDraw(); // at 0x78

    void Create();
    void Reset();
    void Calculate();

    void Show(s32 arg); // [L] argument
    void Hide();
    void SetNextType(s32 type);

    s32 GetType() const {
        return mType;
    }
    f32 GetBonus() const {
        return mBonus;
    }

    static void CreateInstance();
    static void DestroyInstance();
    static RPFshBonusFish* GetInstance() {
        return sInstance;
    }

private:
    void FadeInEnter();
    void FadeInCalc();
    void ShowEnter();
    void ShowCalc();
    void SpinDownEnter();
    void SpinDownCalc();
    void SpinUpEnter();
    void SpinUpCalc();
    void FadeOutEnter();
    void FadeOutCalc();

private:
    RPStateMachine<RPFshBonusFish>* mpStateMachine; // at 0x60
    nw4r::math::VEC2 mScreenPos;        // at 0x64 [M] N_fishPosi_00 position
    s32 mType;                          // at 0x6C RPFshFish::EType
    s32 mNextType;                      // at 0x70 -1 when none
    bool mChangeEnabled;                // at 0x74 [M] true after Reset
    s32 mFrame;                         // at 0x78 [M]
    s32 mInterval;                      // at 0x7C [M] 900 after Reset
    f32 mSpinAngle;                     // at 0x80 [M]
    f32 mSpinSpeed;                     // at 0x84 [M]
    s32 mSpinDir;                       // at 0x88 [M] 1 speeding up, -1 slowing down
    f32 mBonus;                         // at 0x8C points multiplier
    RPGrpModel* mpModels[FISH_TYPE_NUM]; // at 0x90 fsh_fish_* with the swim animation
    RPSysLayout* mpLayout;              // at 0xA8
    RPSysLytAnmObj* mpAnmObjs[3];       // at 0xAC inAll_00, changeAll_00, outAll_00
    nw4r::lyt::Pane* mpFishPosPane;     // at 0xB8 N_fishPosi_00
    nw4r::lyt::Pane* mpUnderPane;       // at 0xBC N_under_00
    nw4r::lyt::Pane* mpAbovePane;       // at 0xC0 N_above_00
    RPSysLytResAccessor* mpResAccessor; // at 0xC4

    // The state functions are a local table in the constructor (80320140, .rodata)

    static const f32 sSpinStart; // 8043C7A0 10 degrees
    static const f32 sSpinMin;   // 8043C7A4 2 degrees
    static f32 sSpinStep;       // 8043C7A8 5 degrees
    static const f32 sSpinMax;   // 8043C7AC 100 degrees

    static RPFshBonusFish* sInstance; // 8043C7B0
};

//! @}

#endif

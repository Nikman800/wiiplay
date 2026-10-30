#ifndef RP_PARTY_PNP_AUDIENCE_H
#define RP_PARTY_PNP_AUDIENCE_H
#include <Pack/types_pack.h>

#include <Pack/RPKernel/IRPSysHostIOSocket.h>
#include <Pack/RPKernel/RPSysKokeshi.h>
#include <Pack/RPParty/RPPartyCommon/RPSimpleModel.h>

#include <nw4r/math.h>

//! @addtogroup rp_pnp
//! @{

// Forward declarations
class RPSysMiddleDB;

/**
 * @brief One Mii spectator standing around the table
 * @details vtable 80359980 (RPSysKokeshi slots, IRPSysHostIOSocket part at 0xB4 with a
 * destructor thunk). Size 0xF8, verified (RPPnpAudience constructor). Same bases as
 * RPKokeshiHead but its own constructor and members. The start position comes from a 17-entry
 * table in the constructor, indexed by the seat.
 */
class RPPnpSpectator : public RPSysKokeshi, public IRPSysHostIOSocket {
public:
    RPPnpSpectator(const RPSysKokeshiGenInfo* pGenInfo, s32 seat, s32 slot);
    virtual ~RPPnpSpectator(); // at 0x8

    void Reset();
    void Calculate();
    void Draw();

    void SetTargetRotY(f32 rotY) {
        mTargetRotY = rotY;
    }
    void SetRotY(f32 rotY) {
        mRotY = rotY;
    }
    nw4r::math::VEC3 GetPos() const {
        return mPos;
    }

private:
    void UpdateWalkIn();

private:
    /**
     * @brief Per-seat entry: walks in once the single-player rally reaches mMinRally
     */
    struct SeatParam {
        s32 mMinRally;   // at 0x0
        s32 mWalkFrames; // at 0x4
        s32 mAnimCycle;  // at 0x8
        s32 mTimer;      // at 0xC
    };

    BOOL mVisible;               // at 0xB8 1 after Reset (models entered)
    f32 mScale;                  // at 0xBC 10.0
    nw4r::math::VEC3 mPos;       // at 0xC0
    nw4r::math::VEC3 mTargetPos; // at 0xCC seat position
    f32 mRotY;                   // at 0xD8
    f32 mTargetRotY;             // at 0xDC turned towards the ball by RPPnpAudience
    s32 mSeat;                   // at 0xE0
    bool mIsSecond;              // at 0xE4 slot == 1: drawn mirrored
    SeatParam mParam;            // at 0xE8 from the seat table
};

/**
 * @brief The crowd: 17 seats, each with two spectators, built from the Mii databases
 * @details vtable 803599A8 (RPSimpleModel slots, Calculate overridden). Size 0x804, verified
 * (Pnp_createAudience; singleton Pnp_audienceInstance). Spectators come from up to
 * Pnp_maxOfficialMii official Miis, then default Miis; the seats are shuffled with RP_lcgSeed.
 */
class RPPnpAudience : public RPSimpleModel {
public:
    static const int SEAT_NUM = 17;

public:
    RPPnpAudience();
    virtual ~RPPnpAudience(); // at 0x8
    virtual void Calculate(); // at 0x14

    void Create();
    void Load();
    void Reset();
    void Draw();
    void Exit();

    /**
     * @brief Loop animations on one seat, or every seat with -1
     */
    void PlayLoopAnimA(s32 seat);
    void PlayLoopAnimB(s32 seat);
    /**
     * @brief Cheers on one seat, or every seat with -1
     */
    void CheerTimed(s32 seat);

    static void CreateInstance();  // Pnp_createAudience
    static void DestroyInstance(); // Pnp_destroyAudience

    static RPPnpAudience* GetInstance() {
        return sInstance;
    }

private:
    struct Seat {
        RPPnpSpectator* mpSpectators[2]; // at 0x0
        s32 mCheerTimer;                 // at 0x8 counts to 120, then mAnim = 1
        s32 mAnim;                       // at 0xC
    };

    RPSysMiddleDB* mpOfficialDB;                       // at 0x44 [M] up to 6 official Miis
    RPSysMiddleDB* mpDefaultDB;                        // at 0x48 [M] source 0, 7 Miis
    RPSysMiddleDB* mpExtraDB;                          // at 0x4C [L] source 3, 17 Miis
    RPSysKokeshiGenInfo mGenInfos[SEAT_NUM];           // at 0x50
    RPSysKokeshiOverloadInfo mOverloadInfos[SEAT_NUM]; // at 0x66C
    Seat mSeats[SEAT_NUM];                             // at 0x6F4

    static RPPnpAudience* sInstance; // 8043C820 (Pnp_audienceInstance)

public:
    static s32 sMaxOfficialMii; // 8043B738 (Pnp_maxOfficialMii) 6
};

//! @}

#endif

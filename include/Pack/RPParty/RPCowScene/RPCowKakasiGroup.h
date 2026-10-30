#ifndef RP_PARTY_COW_KAKASI_GROUP_H
#define RP_PARTY_COW_KAKASI_GROUP_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPCowScene/RPCowFieldObj.h>
#include <Pack/RPParty/RPPartyCommon/RPActor.h>

#include <egg/math.h>

//! @addtogroup rp_cow
//! @{

// Forward declarations
class RPCollisionShape;
class RPCowGame;
class RPCowPlayer;
class RPPathObject;

/**
 * @brief Group of scarecrows placed together (resource type 0x70005); knocking the whole
 * group gives the group bonus
 * @details vtable 8035D250. Size 0x50, verified (Cow_Game_load). SpawnAll places the group's
 * scarecrows (RPCowKakasi / RPCowKakasiKing) when the order message ends; in versus games a
 * group appears when a player comes within 700 units. OnKakasiHit counts the hits per player
 * and scores the group once all are knocked.
 * States (mState): 0 waiting, 1 spawned, 2 finished (sunk or knocked) [H].
 * @note Name is a project convention (ledger Cow_KakasiGroup).
 */
class RPCowKakasiGroup : public RPActor {
public:
    RPCowKakasiGroup();
    virtual ~RPCowKakasiGroup(); // at 0x38

    virtual void LoadResource();    // at 0x44 ledger init
    virtual void OnStart();         // at 0x48 ledger reset
    virtual void OnStop();          // at 0x4C ledger exit
    virtual void OnActivate();      // at 0x50 ledger calc
    virtual void Calculate();       // at 0x54 ledger entryDraw (empty)
    virtual void DrawScreen(u8 pass); // at 0x58 ledger drawScreen (empty)
    /**
     * @brief IRPActorEnable::IsEnabled override (ledger onCollide): returns true [H]
     */
    virtual bool IsEnabled(IRPActorPriority* pOther); // at 0x60

    /**
     * @brief Ledger Cow_KakasiGroup_spawnAll: spawns three scarecrows for every group with
     * no group ahead of it (mGroupsAhead 0) [M]
     */
    static void SpawnAll();
    /**
     * @brief Ledger Cow_KakasiGroup_spawnKakasi: spawns scarecrows start..end-1 (kings when
     * the resource's parameter 1 is 2 or more) in a row along rRot.y, spacing apart [M]
     */
    void SpawnKakasi(const EGG::Vector3f& rPos, const EGG::Vector3f& rRot, s32 start,
                     s32 end, s32 drop, f32 spacing);
    void OnKakasiHit(s32 player);
    EGG::Vector3f GetPos() const;
    // Scarecrows left to knock (RPCowArrow::Calc points at the group while true) [M]
    bool IsRemaining() const {
        return mNum != 0 && mHits < mNum;
    }

private:
    // Inline helpers of OnActivate/OnKakasiHit (defined in RPCowKakasiGroup.cpp)
    bool IsKingNear(RPCowPlayer* pPlayer);
    bool IsKakasiNear(RPCowPlayer* pPlayer);
    bool IsPlayerNear(RPCowPlayer* pPlayer);
    bool IsPlayerAhead(RPCowPlayer* pPlayer);
    void SinkAll();
    void SinkActors(RPCowGame* pGame);

    RPPathObject* mpPathObj;       // at 0x24 placement (resource type 0x70005)
    RPCollisionShape* mpCollision; // at 0x28
    s32 mState;                    // at 0x2C
    s32 mTimer;                    // at 0x30 frames between spawns in state 1
    s32 mGroupsAhead;              // at 0x34 groups further along the course (z)
    s32 mNum;                      // at 0x38 scarecrow count
    s32 mHits;                     // at 0x3C hits by mPlayer
    s32 mScoreTimer;               // at 0x40 frames until the group score (10)
    s32 mBonusLevel;               // at 0x44 [M] consecutive groups (1-3)
    s32 unk48;                     // at 0x48 [L] 15 after a third consecutive group
    s32 mPlayer;                   // at 0x4C player hitting the group (2 = none yet)

    static s32 sSpawnCounter;      // versus: kings spawn every 3-4 approaches
    static const f32 sRadiusScale; // 5.0
};

//! @}

#endif

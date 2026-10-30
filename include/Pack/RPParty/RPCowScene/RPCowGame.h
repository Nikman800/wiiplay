#ifndef RP_PARTY_COW_GAME_H
#define RP_PARTY_COW_GAME_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPActorList.h>
#include <Pack/RPParty/RPPartyCommon/RPActorScene.h>

//! @addtogroup rp_cow
//! @{

// Forward declarations
class RPColliderList;
class RPCollisionModel;
class RPCowSceneSound;
class RPResTable;
class RPSysLytResAccessor;


/**
 * @brief Charge! game manager: the RPActorList of every Cow actor
 * @details vtable 8035CE18 (pointer at 0x18). Size 0x7C, verified (Cow_Scene_createCowMgr).
 * The constructor allocates 0x1D actor slots (RPActorList::AllocTables) and creates the
 * per-frame actors, in table slot order: 0 cameras (2), 1 field, 2 scarecrows (32),
 * 3 scarecrow batch, 4 scarecrow kings (8), 6 hurdle batch, 7 players (2), 10 tree batch,
 * 16 generic model batch, 17 point popups (32), 18 main HUD, 19 perfect message, 20 extra
 * message, 21 order message, 22 target markers (2), 23 balloons (2), 24 start message,
 * 25 goal message, 26 limit count, 27 versus result, 28 angle display. Load adds the
 * resource-placed objects (scarecrow groups, hurdles, trees, goals, knit cows, clouds) in the
 * remaining slots. mpTableNum (RPActorList) holds the actor count per slot.
 * States (mState): 0 intro, 1 ready, 2 pull back, 4 order shown, 5 start wait, 6 count down,
 * 7 play ... 10-13 result sequence [M].
 * @note Name is a project convention (ledger Cow_Game).
 */
class RPCowGame : public RPActorList {
public:
    RPCowGame();
    virtual ~RPCowGame(); // at 0x8

    /**
     * @brief Loads the field (KCL, model, light), the object placement table and every
     * Cow resource, then the party common resources and the sound archive [H]
     */
    virtual void LoadResource(); // at 0xC
    /**
     * @brief Starts every actor, picks the intro or the ready camera and the BGM [H]
     */
    virtual void Reset();     // at 0x10
    virtual void Calculate(); // at 0x14
    // CalculatePost (0x18) is not overridden: the vtable holds RPActorList's empty inline
    // (801C7E68, ledger Lyt_Cow_8035CE18_vf04)
    virtual void DrawScreen(u8 pass);   // at 0x1C
    /**
     * @brief Draws the collider list RPCowGame::sColliderListB on the debug pass [M]
     */
    virtual void DrawPass(u8 pass);     // at 0x20
    virtual void Draw();                // at 0x24

    /**
     * @brief Time ran out for a player: result sequence (ledger Cow_Game_onTimeUp; the
     * player index is passed by RPCowPlayer::CalcProgress) [M]
     */
    void OnTimeUp(s32 player);
    /**
     * @brief A player reached the goal (ledger Cow_Game_onCourseComplete; the player index
     * is passed by RPCowPlayer::HandleCollisions) [M]
     */
    void OnCourseComplete(s32 player);

    // The game manager is RPActorScene's actor list (8043C680) [H]
    static RPCowGame* GetInstance() {
        return static_cast<RPCowGame*>(RPActorScene::GetActorList());
    }
    RPSysLytResAccessor* GetLytAccessor() const {
        return mpLytAccessor;
    }
    s32 GetPlayerNum() const {
        return mPlayerNum;
    }
    s32 GetState() const {
        return mState;
    }
    // Groups spawned / knocked whole / consecutive knocked groups (RPCowKakasiGroup) [M]
    s32& GetSpawnedGroups() {
        return mSpawnedGroups;
    }
    s32& GetKnockedGroups() {
        return mKnockedGroups;
    }
    s32& GetGroupChain() {
        return mGroupChain;
    }
    // Object placement table of the course (cow_*.brres placements, resource types 0x7000N) [M]
    RPResTable* GetResTable() const {
        return mpResTable;
    }
    // First actor of a table slot (slot 0 cameras, 7 players: see above) [H]
    RPActor* GetSlotActor(int slot) const {
        return mpTableA[slot];
    }
    // Player that reached the goal first (-1 on reset) [M]
    s32 GetWinner() const {
        return unk48;
    }
    // In the goal/result states (9-24) and player reached the goal first [M]
    bool IsWinner(s32 player) const {
        return (u32)(mState - 9) <= 15 && unk48 == player;
    }
    bool IsPlayIntro() const {
        return mPlayIntro;
    }
    // Field collision model, RP_CollisionModel_checkSphere [M]
    RPCollisionModel* GetCollisionModel() const {
        return mpCollisionModel;
    }
    // Number of actors in a table slot [M]
    s32 GetSlotNum(int slot) const {
        return mpTableNum[slot];
    }

private:
    // Actor creation for LoadResource, defined in RPCowGame.cpp (inlined there)
    template <typename T>
    void RegistArray(T* pActors, int num, u16 group, u16 key);
    template <typename T>
    void CreateActors(int num, int slot, u16 key);
    template <typename T>
    void CreateActor(int slot, u16 key);
    template <typename T>
    void CreatePlaced(RPResTable* pTable, u32 type, int slot, u16 key);

    s32 mState;                        // at 0x1C
    s32 mTimer;                        // at 0x20
    s32 mCountdown;                    // at 0x24 seconds shown by the count down [M]
    s32 unk28;                         // at 0x28 150 on reset [L]
    s32 mCourse;                       // at 0x2C from the party player data (+0x1C) [M]
    s32 mTimeBonus;                    // at 0x30 remaining time in seconds at the goal [M]
    s32 unk34;                         // at 0x34 [L]
    s32 unk38;                         // at 0x38 [L]
    f32 mTime;                         // at 0x3C [M] elapsed time
    s32 mPlayerNum;                    // at 0x40 1 or 2
    s32 unk44;                         // at 0x44 -1 on reset [L]
    s32 unk48;                         // at 0x48 -1 on reset [L]
    char unk4C[0x54 - 0x4C];
    s32 mTimeUp[2];                    // at 0x54 [M] set per player by OnTimeUp
    s32 mSpawnedGroups;                // at 0x5C [M] scarecrow groups spawned
    s32 mKnockedGroups;                // at 0x60 [M] groups knocked whole
    s32 mGroupChain;                   // at 0x64 [M] consecutive knocked groups
    bool mPlayIntro;                   // at 0x68 [M] intro camera on the first play of a course
    RPSysLytResAccessor* mpLytAccessor; // at 0x6C cow.arc layouts
    RPCowSceneSound* mpSceneSound;     // at 0x70
    /**
     * @brief Object placement table (resource types 0x70000-0x7000A: players, scarecrow
     * groups, goals, trees, hurdles, ...) read with RP_ResTable_* [M]
     */
    RPResTable* mpResTable;            // at 0x74
    RPCollisionModel* mpCollisionModel; // at 0x78 field collision model (LoadResource)

public:
    static u16 sDrawPriorityBase; // base resource count of the draw resource list
    static RPColliderList sColliderListA;
    static RPColliderList sColliderListB; // drawn on the debug pass
};

//! @}

#endif

#ifndef RP_PARTY_WLY_AVATAR_MGR_H
#define RP_PARTY_WLY_AVATAR_MGR_H
#include <Pack/types_pack.h>

#include <Pack/RPKernel/RPSysKokeshiGenInfo.h>
#include <Pack/RPKernel/RPSysKokeshiOverloadInfo.h>
#include <Pack/RPParty/RPPartyCommon/RPSimpleModel.h>

#include <nw4r/math.h>

//! @addtogroup rp_wly
//! @{

// Forward declarations
class RPSysMiddleDB;
class RPWlyAvatar;

/**
 * @brief Avatar placement, motion patterns and target bookkeeping of the current stage
 * @details No vtable. Size 0xC78, verified (Wly_AvatarMgr_ctor). mTargetNum comes from the
 * level's question type (1-5 targets). Positions are placed by one of the place* patterns and
 * moved each frame by the matching calcMotion* function; category 4 walks the avatars along
 * numbered nodes of the field model (mNodes).
 */
class RPWlyStage {
    friend class RPWlyAvatarMgr;

public:
    /**
     * @brief Per-avatar hover state (a cursor resting on the avatar)
     */
    struct HoverEntry {
        u32 unk0;      // at 0x0 [L] never accessed
        bool mHovered; // at 0x4
        s32 mTimer;    // at 0x8 frames on the avatar (pick at 30)
        s32 mPlayer;   // at 0xC
    };

    /**
     * @brief Avatar slot on a field node (category 4 stages)
     */
    struct Node {
        s32 mNode;    // at 0x0 node number (14 per row, 3 rows)
        f32 mOffset;  // at 0x4 x offset from the node
    };

public:
    RPWlyStage();

    void Calculate();
    void Reset();
    void UpdateAvatarHoverReactions();
    void UpdateHoverTimers();
    bool IsAvatarHovered(s32 idx) const;
    void CheckMultiHoverPick();
    void RecordFoundMii(s32 idx);
    s32 GetPointValue() const;
    bool IsAllFound() const;
    void MarkTargetFound(s32 idx);
    bool IsTargetFound(s32 idx) const;
    bool IsTargetNotFound(s32 idx) const;
    bool IsTargetFoundInPlay(s32 idx) const;
    void ClampPosition(nw4r::math::VEC3* pPos) const;
    void GetNodePosition(nw4r::math::VEC3* pPos, s32 idx) const;

    // Defined in RPWlyAvatarMgr_2.cpp (the original takes these parameters)
    void UpdateAvatarHoverReactions(s32 idx, s32 player);
    void CheckMultiHoverPick(s32 idx);
    void ClampPosition(nw4r::math::VEC3* pPos); // non-const (the const declaration above is unused)
    s32 GetPointValue(); // non-const (the const declaration above is unused)
    void PlaceAvatarsFormation();
    void PlaceAvatarsRandom();
    void PlaceAvatarsSpaced();
    void PlacePattern0();
    void CalcMotionPattern0();
    void PlacePattern1();
    void CalcMotionPattern1();
    void PlacePattern2();
    void CalcMotionPattern2();
    void PlacePattern5();
    void CalcMotionPattern5();
    nw4r::math::VEC3 GetEscalatorSlotPos(s32 idx);
    void PlaceEscalator();
    void CalcMotionEscalator();

    // Wii Play: RPWlyField
    s32 GetTargetNum() const {
        return mTargetNum;
    }
    s32 GetFoundNum() const {
        return mFoundNum;
    }

private:
    bool mActive;                          // at 0x0 Calculate runs only when set
    s32 mTargetIdx;                        // at 0x4 -1 after construction
    s32 mTargetNum;                        // at 0x8 1-5
    s32 unkC;                              // at 0xC
    s32 mFoundNum;                         // at 0x10
    bool mFound[5];                        // at 0x14
    HoverEntry mHover[100];                // at 0x1C
    s32 mMotionTimer;                      // at 0x65C [M]
    f32 mMotionValue;                      // at 0x660 [M] 1.0
    f32 mClampX;                           // at 0x664 [M]
    f32 mClampZ;                           // at 0x668 [M]
    nw4r::math::VEC3 mPositions[100];      // at 0x66C
    Node mNodes[42];                       // at 0xB1C
    s32 mPatternTimer;                     // at 0xC6C pattern 1: flips every 90 frames
    BOOL mPatternFlip;                     // at 0xC70
    s32 mPatternStep;                      // at 0xC74 0..4

public:
    static f32 sAvatarSpacing[2]; // [0] minimum spacing between placed avatars
};

/**
 * @brief Owner of the stage's avatars (singleton RPWlyAvatarMgr::sInstance)
 * @details vtable 8035C850 (RPSimpleModel slots). Size 0x291C, verified
 * (Wly_AvatarMgr_createGlobal). Builds up to 100 avatars from the official Mii database or a
 * middle database (question types 6 and 7: CreateAvatarsSpecial) plus one extra avatar for
 * the question layout. Per player: avatar under / near the cursor (mHoverIdx) and the last
 * one that played the hover sound.
 */
class RPWlyAvatarMgr : public RPSimpleModel {
public:
    RPWlyAvatarMgr();
    virtual ~RPWlyAvatarMgr(); // at 0x8
    virtual void Calculate();  // at 0x14

    void Create();
    void Init();
    void Reset();
    void Draw();
    void Destroy();
    void SetPaused(bool paused);
    void StartSwimEffects();
    s32 FindAvatarUnderCursor(s32 player);
    s32 FindAvatarNearCursor(s32 player);
    bool IsAnyWrongReacting() const;
    bool IsWrongReacting(s32 idx) const;
    void ShowAll();
    void HideAll();
    void HideNonTargets();
    void MarkFound(s32 idx);
    void SetAvatarFound(s32 idx);
    bool TryWrongReaction(s32 idx);
    void CreateAvatars();
    void CreateAvatarsSpecial();
    RPSysKokeshiGenInfo GetGenInfo(s32 idx);           // RPWlyAvatarMgr_2.cpp
    RPSysKokeshiOverloadInfo GetOverloadInfo(s32 idx); // RPWlyAvatarMgr_2.cpp

    static void CreateInstance();  // Wly_AvatarMgr_createGlobal
    static void DestroyInstance(); // Wly_AvatarMgr_destroyGlobal

public:
    RPSysMiddleDB* mpOfficialDB;                   // at 0x44 random official Miis (CreateAvatars)
    RPSysMiddleDB* mpHiddenDB;                     // at 0x48 hidden Miis (CreateAvatars)
    RPSysMiddleDB* mpMiddleDB;                     // at 0x4C random Miis, or the scene's (special)
    RPWlyStage* mpStage;                           // at 0x50
    RPSysKokeshiGenInfo mGenInfos[100];            // at 0x54
    RPSysKokeshiOverloadInfo mOverloadInfos[100];  // at 0x2444
    RPWlyAvatar* mpAvatars[100];                   // at 0x2764
    RPWlyAvatar* mpQuestionAvatar;                 // at 0x28F4 [M] Mii shown by the question
    s32 unk28F8;                                   // at 0x28F8
    s32 mHoverIdx[2];                              // at 0x28FC per player, -1 for none
    s32 mLastHoverIdx[2];                          // at 0x2904
    s32 mFoundIdx;                                 // at 0x290C
    BOOL mMotionFlags[3];                          // at 0x2910 [L] set by Reset and the wipe

public:
    static RPWlyAvatarMgr* sInstance; // 8043C968
    static f32 sHitOffsetY; // hover pick radius: offset
    static f32 sHitScale;   // hover pick radius: scale / depth
};

//! @}

#endif

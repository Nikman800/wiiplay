#ifndef RP_PARTY_WLY_AVATAR_H
#define RP_PARTY_WLY_AVATAR_H
#include <Pack/types_pack.h>

#include <Pack/RPKernel/IRPSysHostIOSocket.h>
#include <Pack/RPKernel/RPSysKokeshi.h>
#include <Pack/RPParty/RPWlyScene/RPWlyField.h>
#include <Pack/RPParty/RPWlyScene/RPWlySound.h>

#include <nw4r/math.h>

//! @addtogroup rp_wly
//! @{

// Forward declarations
class RPSysEffect;

/**
 * @brief Avatar shadow (wtd_ava_shadow), only on the ground fields
 * @details vtable 8035CBB0. Size 0x44, verified (Wly_Avatar_ctor). Calculate is skipped on
 * field types 1 and 2. Derives directly from RPSimpleModel: the destructor has no
 * intermediate RPWlyObject destructor/null check.
 */
class RPWlyAvatarShadow : public RPSimpleModel {
public:
    RPWlyAvatarShadow();
    virtual ~RPWlyAvatarShadow(); // at 0x8
    virtual void Calculate();     // at 0x14
    virtual void Load();          // at 0x1C
    /**
     * @brief Zero position, unit scale [H]
     */
    virtual void Reset();         // at 0x20
};

/**
 * @brief One Mii in the crowd
 * @details vtable 8035C7E0 (IRPSysHostIOSocket part at 0xB4, vtable 8035C7F4). Size 0x128,
 * verified (Wly_AvatarMgr_ctor/createAvatars/createAvatarsSpecial). Same bases as
 * RPKokeshiHead but built directly on RPSysKokeshi (its constructor is not called).
 * Reaction states (mReaction): 0 none, 1 wrong (70 frames), 2 found, 3 found done [M].
 * The effects are the found sparkle (0xC4), found effect (0xC8, field types other than 2),
 * field-2 found effect (0xCC) and Wtd_ava_swim01 (0xD0, field type 1).
 */
class RPWlyAvatar : public RPSysKokeshi, public IRPSysHostIOSocket {
public:
    RPWlyAvatar(const RPSysKokeshiGenInfo* pGenInfo, s32 index, BOOL hasShadow);
    virtual ~RPWlyAvatar(); // at 0x8

    /**
     * @brief RPSysKokeshi::LoadResource, then the shadow's Load [H]
     */
    virtual void LoadResource(const RPSysKokeshiOverloadInfo* pOverloadInfo =
                                  NULL); // at 0xC

    void StartSwimEffect();
    /**
     * @brief Hides the models while paused and restores the saved visibility [H]
     */
    void SetPaused(bool paused);
    void SetDrawScene(u8 value);
    void PlayHoverSound();
    void OnFound();
    void SetFound();
    bool StartWrongReaction();
    void Draw();
    /**
     * @brief Sway, spin and scale animation, model matrices, screen position and the sound
     * actor's 3D position (called by RPWlyStage::Calculate) [H]
     */
    void CalcModelPose();
    void ResetState();

public:
    bool mWasVisible;                 // at 0xB8 saved by SetPaused
    RPWlySoundActor* mpSound;         // at 0xBC
    RPWlyAvatarShadow* mpShadow;      // at 0xC0
    RPSysEffect* mpEffects[4];        // at 0xC4
    s32 mReaction;                    // at 0xD4
    s32 mReactionTimer;               // at 0xD8
    u8 unkDC;                         // at 0xDC [L] 1 from the constructor
    bool mHovered;                    // at 0xDD [M]
    bool mVisible;                    // at 0xDE
    bool unkDF;                       // at 0xDF [L] debug draw flag
    f32 mScale;                       // at 0xE0
    f32 mCurScale;                    // at 0xE4 eases toward mScale (or the level's 0x28)
    nw4r::math::VEC3 mPos;            // at 0xE8
    f32 mSwayX;                       // at 0xF4 [M]
    f32 mSwaySpeedX;                  // at 0xF8 [M]
    f32 mSwayZ;                       // at 0xFC [M]
    f32 mSwaySpeedZ;                  // at 0x100 [M]
    f32 mRotY;                        // at 0x104 random on field type 2
    f32 mRotSpeed;                    // at 0x108 +-(0.008..0.024) on field type 2
    s32 mIndex;                       // at 0x10C
    nw4r::math::VEC3 mScreenPos;      // at 0x110 head position on screen
    nw4r::math::VEC3 mPopupPos;       // at 0x11C [M] point popup position on screen

public:
    static f32 sDebugHitScale; // size of the debug hit box drawn around the head
};

//! @}

#endif

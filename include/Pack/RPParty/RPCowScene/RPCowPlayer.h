#ifndef RP_PARTY_COW_PLAYER_H
#define RP_PARTY_COW_PLAYER_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPCowScene/RPCowSound.h>
#include <Pack/RPParty/RPPartyCommon/RPActor.h>

#include <egg/math.h>
#include <nw4r/lyt.h>

//! @addtogroup rp_cow
//! @{

// Forward declarations
class RPCollisionShape;
class RPCowCowModel;
class RPCowPlayerSound;
class RPGrpModel;
class RPGrpShadowTexture;
class RPPathObject;
class RPSysEffect;
class RPSysLayout;
class RPSysLytAnmObj;


/**
 * @brief Ring buffer of 600 positions (ten seconds)
 * @details No vtable. Size 0x1C2C [H] (constructor of RPCowPosHistory).
 * @note Name is a project convention (Push is the ledger's Cow_PosHistory_push).
 */
class RPCowPosRing {
public:
    RPCowPosRing() : mStart(0), mNext(0), mFull(false) {}

    /**
     * @brief Stores rPos at mNext, advances it (wrapping at 600) and sets mFull once it
     * reaches mStart [H]
     */
    void Push(const EGG::Vector3f& rPos);
    /**
     * @brief The position pushed last (RPCowPlayer::CalcInput) [M]
     */
    const EGG::Vector3f& GetLast() const {
        return mPos[mNext - 1 >= 0 ? mNext - 1 : mNext + 599];
    }

private:
    EGG::Vector3f mPos[600]; // at 0x0
    s32 mStart;              // at 0x1C20
    s32 mNext;               // at 0x1C24
    bool mFull;              // at 0x1C28
};

/**
 * @brief Two position rings of a player [L purpose]
 * @details No vtable. Size 0x385C, verified (array stride in the RPCowPlayer constructor).
 * @note Name is a project convention (ledger Cow_PosHistory).
 */
class RPCowPosHistory {
    friend class RPCowPlayer;

public:
    RPCowPosHistory();
    ~RPCowPosHistory();

private:
    // Two members, not an array: the constructor and destructor handle each ring inline
    // (RPCowPlayer_4, RPCowPlayer_2).
    RPCowPosRing mRing0;    // at 0x0
    RPCowPosRing mRing1;    // at 0x1C2C
    u32 unk3858;            // at 0x3858 [L] never accessed
};

/**
 * @brief Arrow above the cow pointing at the next scarecrow group (cow_arrow1P/2P)
 * @details No vtable; embedded in RPCowPlayer at 0x7238. Size 0x50 [M] (RPCowPlayerCursor
 * follows at 0x7288).
 * @note Name is a project convention (ledger Cow_Arrow).
 */
class RPCowArrow {
    friend class RPCowPlayer;

public:
    void Init(s32 player);
    void Reset(s32 player);
    void Calc();
    /**
     * @brief Keeps the arrow up for 5 frames, restarting its pop-up animation when hidden
     * (defined in RPCowPlayer_6.cpp, used by RPCowPlayer::HandleCollisions) [M]
     */
    inline void Show();

private:
    // RP_approachStep of mTargetPos towards rTarget, per axis (defined in RPCowPlayer_5) [M]
    inline void ApproachTarget(const EGG::Vector3f& rTarget);

    static const f32 sRate; // 1: approach rate of the target position
    static const f32 sOfsY; // 20: arrow height above the cow

    RPGrpModel* mpModel;           // at 0x0
    RPCollisionShape* mpCollision; // at 0x4
    s32 mTimer;                    // at 0x8 [M] frames the arrow stays up
    s32 mPlayer;                   // at 0xC
    s32 mTargetGroup;              // at 0x10 [M] set by RPCowPlayer::SetTargetGroup
    EGG::Vector3f unk14;           // at 0x14 [L] player position on reset
    EGG::Vector3f mTargetPos;      // at 0x20 [M] eased towards the target
    EGG::Vector3f mTargetVel;      // at 0x2C [M]
    f32 unk38;                     // at 0x38 [L]
    f32 unk3C;                     // at 0x3C [L]
    f32 mScale;                    // at 0x40 [M] read by the player
    f32 mScaleVel;                 // at 0x44 [M]
    f32 mAngle;                    // at 0x48 [M] relative to the cow's yaw
    f32 unk4C;                     // at 0x4C [L]
};

/**
 * @brief Two-player cursor showing where the other player is (cowCursor_00.brlyt)
 * @details No vtable; embedded in RPCowPlayer at 0x7288. Size 0x40 [M] (RPCowPlayer members
 * resume at 0x72C8).
 * @note Name is a project convention (ledger Cow_PlayerCursor).
 */
class RPCowPlayerCursor {
public:
    /**
     * @brief Empty, but out of line (RPCowPlayer's constructor calls it) [H]
     */
    RPCowPlayerCursor();
    ~RPCowPlayerCursor();

    void Init(s32 player, s32 target);
    void Reset();
    void Calc();
    void Draw();

    /**
     * @brief Named constants at the start of RPCowPlayer_7's .sdata2 (defined before the code,
     * read through these getters so they are loaded, not folded) [M]
     */
    static const f32& GetOfsY() {
        return sOfsY;
    }
    static const f32& GetMarginX() {
        return sMarginX;
    }
    static const f32& GetNearDepth() {
        return sNearDepth;
    }

private:
    static const f32 sOfsY;      // 12: cursor height above the cow
    static const f32 sMarginX;   // 34: kept this far inside the screen edge
    static const f32 sNearDepth; // 40: closer than this along the view, the cursor shows

    RPSysLayout* mpLayout;          // at 0x0
    RPSysLytAnmObj* mpAnmObjs[2];   // at 0x4 cursor1p, cursor2p
    nw4r::lyt::Pane* mpPanes[2];    // at 0xC N_cursor_00, N_cursor_01
    s32 mTimer;                     // at 0x14 [M] frames the cursor stays up
    s32 mPlayer;                    // at 0x18 viewing player (camera)
    s32 mTarget;                    // at 0x1C [M] player shown (and pane index)
    EGG::Vector3f mPos;             // at 0x20 [M] screen position
    EGG::Vector3f mVel;             // at 0x2C [M]
    f32 mScale;                     // at 0x38 [M]
    f32 mScaleVel;                  // at 0x3C [M]
};

/**
 * @brief The cow's particle effects (speed smoke, jump, damage, landing, dash smoke, crash,
 * weeds and wool)
 * @details No vtable. Size 0x118, verified (Cow_Player_init allocates 0x118; the ledger's
 * Cow_PlayerEffects_init is the constructor).
 * @note Name is a project convention (ledger Cow_PlayerEffects).
 */
class RPCowPlayerEffects {
    friend class RPCowPlayer;

public:
    explicit RPCowPlayerEffects(RPCowCowModel* pCowModel);

    /**
     * @brief Ledger Cow_spawnEffectAtMtx: one-shot effect at rPos (height 6) with the cow's
     * orientation [M]
     */
    void SpawnEffectAtMtx(const EGG::Vector3f& rPos, const char* pName);
    /**
     * @brief Ledger Cow_spawnLandSmoke: one-shot landing smoke under the cow [M]
     */
    void SpawnLandSmoke(bool big, f32 power);
    void EmitWool(f32 rate, const EGG::Vector3f& rPos, const EGG::Vector3f& rDir);
    void ResetWool();
    /**
     * @brief Ledger Cow_CowModel_calcAnim: drives the looping effects from the cow model's
     * head, root and foot positions [M]
     */
    void Calc();

    // Set by RPCowPlayer::CalcMovement (the matrix is passed by value) [M]
    void SetMtx(EGG::Matrix34f mtx) {
        static_cast<EGG::Matrix34f&>(mMtx) = mtx;
    }
    // Set by RPCowPlayer::UpdateModel, clamped to 0..1 [M]
    void SetSpeed(f32 speed) {
        mSpeed = speed < 0.0f ? 0.0f : (speed > 1.0f ? 1.0f : speed);
    }
    void SetDashRate(f32 rate) {
        mDashRate = rate < 0.0f ? 0.0f : (rate > 1.0f ? 1.0f : rate);
    }

private:
    static const f32 sEffectY; // 6: height of one-shot effects and wool bursts [M]

    RPCowCowModel* mpCowModel;          // at 0x0
    nw4r::math::MTX34 mMtx;             // at 0x4 [M] cow orientation, set by the player
    /**
     * @brief Cow_cow_speedsmoke01l/r, Cow_cow_jump_ok, Cow_cow_damage01, Cow_landsmoke x2
     */
    RPSysEffect* mpEffects[6];          // at 0x34
    s32 mWoolIdx;                       // at 0x4C
    RPSysEffect* mpCrashEffects[4];     // at 0x50 Cow_vs_crash01
    EGG::Vector3f mCrashPos[4];         // at 0x60 [M]
    EGG::Vector3f mCrashVel[4];         // at 0x90 [M]
    u8 mCrashActive[4];                 // at 0xC0 [M] wool emit flags
    RPSysEffect* mpDashEffects[4];      // at 0xC4 Cow_dashsmk_front x2, Cow_dashsmk_back x2
    RPSysEffect* mpWeedEffects[4];      // at 0xD4 Cow_dashsmk_weed
    RPSysEffect* mpWoolEffects[4];      // at 0xE4 Cow_slip_wool
    RPSysEffect* mpGreenWoolEffects[4]; // at 0xF4 Cow_slip_wool_green
    bool mSpeedSmoke;                   // at 0x104 [M] speedsmoke on
    bool mJump;                         // at 0x105 [M] jump_ok on
    bool mDamage;                       // at 0x106 [M] damage on
    bool mOnWeed;                       // at 0x107 [M] weed / green wool instead of wool
    f32 mSpeed;                         // at 0x108 [M] 0..1, dash effects above 0.1
    f32 mDashRate;                      // at 0x10C [M] 0..1
    f32 mSlip;                          // at 0x110 [M] sign picks the wool direction
    f32 mSlip2;                         // at 0x114 [M]
};

/**
 * @brief A player: the cow and rider, its movement, collisions and score
 * @details vtable 8035D00C. Size 0x72E8, verified (array stride in the RPCowGame constructor).
 * Two position histories of 2 x 600 samples make up most of the size.
 * @note Name is a project convention (ledger Cow_Player).
 */
class RPCowPlayer : public RPActor {
public:
    RPCowPlayer();

    /**
     * @brief Ledger create: RPCowCowModel and the sound actor [H]
     * @details Declared before the destructor so that it is the key function (vtable and
     * thunks in RPCowPlayer_6); the destructor sits in a unit of its own (RPCowPlayer).
     */
    virtual void VF_0x3C(); // at 0x3C
    virtual ~RPCowPlayer(); // at 0x38
    /**
     * @brief Ledger destroy: deletes the sound actor [H]
     */
    virtual void VF_0x40();           // at 0x40
    virtual void LoadResource();      // at 0x44 ledger init
    virtual void OnStart();           // at 0x48 ledger reset
    virtual void OnStop();            // at 0x4C ledger exit
    virtual void OnActivate();        // at 0x50 ledger calc
    virtual void Calculate();         // at 0x54 ledger entryDraw
    virtual void DrawScreen(u8 pass); // at 0x58
    /**
     * @brief Draws the debug log on pass 4 [M]
     */
    virtual void DrawPass(u8 pass); // at 0x5C
    /**
     * @brief IRPActorEnable::IsEnabled override (ledger onCollide) [M]
     */
    virtual bool IsEnabled(IRPActorPriority* pOther); // at 0x60

    void UpdateModel();
    void DrawShadowTex();
    void CalcInput();
    void CalcMovement();
    void CalcProgress();
    void CalcFootEffects();
    void HandleCollisions();
    void AddBonusScore(s32 points);
    void AddGroupScore(s32 group);
    /**
     * @brief Ledger Cow_Player_addGroupScore: adds a knocked group's points (popup with the
     * perfect flag) [M]
     */
    void AddGroupScore(s32 points, s32 bonusLevel);
    /**
     * @brief Ledger Cow_Player_playKakasiHitSound (the argument is unused) [M]
     */
    void PlayKakasiHitSound(s32 bonusLevel);
    void AddScore(s32 points);
    /**
     * @brief Ledger Cow_Player_addScore: adds the points and pops them up at rPos (the course
     * objects pass their collision position) [M]
     */
    void AddScore(s32 points, const EGG::Vector3f& rPos);
    f32 GetSpeed() const {
        return mSpeed;
    }
    // 1 while the cow charges (field objects are knocked away), else it jumps hurdles [L]
    s32 GetUnkA8() const {
        return unkA8;
    }
    void PlayBonusSound();
    // unkA8 0 or 1 [L]
    bool IsCharging() const {
        return unkA8 == 0 || unkA8 == 1;
    }
    s32 GetTotalScore() const;
    void SetTargetGroup(s32 group);
    EGG::Vector3f GetTargetPos();
    // Not const: the original reloads the member after storing into the result (a const
    // method lets the load move above the store) [H]
    EGG::Vector3f GetCameraYawVec();
    EGG::Vector3f GetRightDir();
    EGG::Vector3f GetForwardDir();
    EGG::Vector3f GetRotation();
    EGG::Vector3f GetVelocity() const;
    EGG::Vector3f GetForwardMove();
    EGG::Vector3f GetMoveDelta();
    EGG::Vector3f GetPrevPos();
    EGG::Vector3f GetPos() const;
    void SetArrowVisible(bool visible);

private:
    // RPCowGame drives the cow models, the timer and the score of its players
    friend class RPCowGame;

    // Named constants defined after the code (loaded, not folded), .sdata2 80440AD0.. and
    // .sbss2 804419A8.. in definition order [L names]
    static const f32 sModelScale;
    static const f32 sC_AD4;
    static const f32 sC_AD8;
    static const f32 sC_ADC;
    static const f32 sC_AE0;
    static const f32 sC_AE4;
    static const f32 sC_AE8;
    static const f32 sC_AEC;
    static const f32 sC_AF0;
    static const f32 sC_AF4;
    static const f32 sC_AF8;
    static const f32 sC_AFC;
    static const f32 sC_B00;
    static const f32 sC_B04;
    static const f32 sC_B08;
    static const f32 sC_B0C;
    static const f32 sC_B10;
    static const f32 sC_B14;
    static const f32 sMoveScale;
    static const f32 sMaxRateSolo;
    static const f32 sMaxRateMax;
    static const f32 sSpeedRateSolo;
    static const f32 sSpeedRateMin;
    static const f32 sLeadDistMax;
    static const f32 sLeadAhead;
    static const f32 sChaseDist;
    static const f32 sChaseAngle;
    static const f32 sZ_19A8;
    static const f32 sZ_19AC;
    static const f32 sZ_19B0;
    static const f32 sLeadDistMin;
    static const f32 sZ_19B8;
    static const f32 sZ_19BC;
    static const f32 sShadowScale;     // 200 (80440B44), defined after the arrow's constants
    static const GXColor sShadowColor; // 0, 0, 20, 255 (80440B48)

    RPPathObject* mpPathObj;          // at 0x24 start position (resource type 0x70001)
    EGG::Matrix34f mMtx;              // at 0x28
    EGG::Matrix34f mShadowMtx;        // at 0x58 [M]
    RPGrpShadowTexture* mpShadowTex;  // at 0x88 0x200 bytes, vtable replaced by 80358E18 [M]
    RPCowPlayerSound* mpSound;        // at 0x8C
    RPCollisionShape* mpCollision;    // at 0x90
    RPCowCowModel* mpCowModel;        // at 0x94
    RPCowPlayerEffects* mpEffects;    // at 0x98
    s32 unk9C;                        // at 0x9C [L]
    s32 unkA0;                        // at 0xA0 [L]
    bool unkA4;                       // at 0xA4 [L]
    bool unkA5;                       // at 0xA5 [L]
    bool unkA6;                       // at 0xA6 [L]
    s32 unkA8;                        // at 0xA8 [L]
    u16 unkAC;                        // at 0xAC [L] hit flags from the collider list
    RPCowPosHistory mPosHistory[2];   // at 0xB0
    f32 unk7168;                      // at 0x7168 [L]
    f32 unk716C;                      // at 0x716C [L]
    f32 unk7170;                      // at 0x7170 [L]
    f32 unk7174;                      // at 0x7174 [L]
    f32 unk7178;                      // at 0x7178 [L]
    f32 unk717C;                      // at 0x717C [L]
    f32 unk7180;                      // at 0x7180 [L]
    f32 unk7184;                      // at 0x7184 [L]
    f32 unk7188;                      // at 0x7188 [L]
    f32 mProgress;                    // at 0x718C [L]
    s32 unk7190;                      // at 0x7190 [L]
    f32 unk7194;                      // at 0x7194 [L]
    s32 unk7198;                      // at 0x7198 [L]
    s32 unk719C;                      // at 0x719C [L]
    s32 unk71A0;                      // at 0x71A0 [L]
    f32 unk71A4;                      // at 0x71A4 [L]
    f32 unk71A8;                      // at 0x71A8 [L]
    f32 mSpeed;                       // at 0x71AC [M]
    f32 unk71B0;                      // at 0x71B0 [L]
    f32 unk71B4;                      // at 0x71B4 [L]
    f32 unk71B8;                      // at 0x71B8 [L]
    f32 unk71BC;                      // at 0x71BC [L]
    f32 unk71C0;                      // at 0x71C0 [L]
    f32 unk71C4;                      // at 0x71C4 [L]
    f32 unk71C8;                      // at 0x71C8 [L]
    f32 unk71CC;                      // at 0x71CC [L]
    f32 unk71D0;                      // at 0x71D0 [L]
    f32 unk71D4;                      // at 0x71D4 [L]
    f32 unk71D8;                      // at 0x71D8 [L]
    f32 unk71DC;                      // at 0x71DC [L]
    s32 unk71E0;                      // at 0x71E0 [L]
    f32 unk71E4;                      // at 0x71E4 [L]
    f32 unk71E8;                      // at 0x71E8 [L]
    f32 unk71EC;                      // at 0x71EC [L]
    s32 unk71F0;                      // at 0x71F0 [L]
    f32 unk71F4;                      // at 0x71F4 [L]
    f32 unk71F8;                      // at 0x71F8 [L]
    f32 unk71FC;                      // at 0x71FC [L]
    f32 unk7200;                      // at 0x7200 [L]
    f32 unk7204;                      // at 0x7204 [L]
    f32 unk7208;                      // at 0x7208 [L]
    f32 unk720C;                      // at 0x720C [L]
    s32 unk7210;                      // at 0x7210 [L]
    s32 unk7214;                      // at 0x7214 [L]
    s32 unk7218[4];                   // at 0x7218 [L] hit counters
    f32 unk7228;                      // at 0x7228 [L]
    f32 unk722C[3];                   // at 0x722C [L]
    RPCowArrow mArrow;                // at 0x7238
    RPCowPlayerCursor mCursor;        // at 0x7288
    s32 mScore;                       // at 0x72C8
    s32 unk72CC;                      // at 0x72CC [L]
    s32 mBonusScore;                  // at 0x72D0 [M]
    f32 mTime;                        // at 0x72D4 [M] frames, shown divided by 60
    f32 unk72D8;                      // at 0x72D8 [L]
    s32 unk72DC;                      // at 0x72DC [L]
    s32 unk72E0;                      // at 0x72E0 [L]
    s32 unk72E4;                      // at 0x72E4 [L]
};

//! @}

#endif

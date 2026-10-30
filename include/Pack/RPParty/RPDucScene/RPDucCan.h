#ifndef RP_PARTY_DUC_CAN_H
#define RP_PARTY_DUC_CAN_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPDucScene/RPDucTarget.h>

#include <nw4r/math.h>

//! @addtogroup rp_duc
//! @{

// Forward declarations
class RPGrpModel;
namespace EGG { class Archive; }

/**
 * @brief One pattern of RPDucCanParam (RPDucCan::Launch, pattern & 7)
 * @details Size 0x58 [H]. Load triples mBounceFrames and, in 4:3, scales mSpinRange by
 * 0.85 [M].
 */
struct RPDucCanPattern {
    f32 mAimRangeX;       // at 0x0
    f32 mAimRangeY;       // at 0x4
    f32 mAimY;            // at 0x8
    f32 mAimZ;            // at 0xC
    f32 mHitScale;        // at 0x10
    f32 mSpinMax;         // at 0x14
    f32 mSpinMin;         // at 0x18
    f32 unk1C;            // at 0x1C [L]
    f32 mBounceVelZ;      // at 0x20
    s32 mMaxHits;         // at 0x24 the can breaks on this hit
    f32 mGravityUp;       // at 0x28
    f32 mSpeed;           // at 0x2C
    f32 mGravityDown;     // at 0x30
    f32 mBounceHeight[4]; // at 0x34 per hit count 1, 2, 3, more
    s32 mBounceFrames[4]; // at 0x44 per hit count 1, 2, 3, more
    f32 mHitRadius;       // at 0x54
};

/**
 * @brief Contents of param/DucCanParam.bin, copied into RPDucCan by Load
 * @details Size 0x2E0 [H]: a 0x20-byte header and eight patterns; copied member by member.
 */
struct RPDucCanParam {
    f32 mDrag;                    // at 0x0
    f32 mHitPush;                 // at 0x4 [M] spin push of a hit
    s32 unk08;                    // at 0x8 [L]
    s32 unk0C;                    // at 0xC [L]
    s32 mCooldown;                // at 0x10 frames between hits, tripled on load
    f32 mBaseRadius;              // at 0x14 hit radius of scale 1
    f32 mEdgeScreenY;             // at 0x18 [M]
    f32 mSpinDecay;               // at 0x1C
    RPDucCanPattern mPatterns[8]; // at 0x20
};

/**
 * @brief Tin can target, knocked up by each hit (five damage models)
 * @details vtable 8035AC10. Size 0x470, verified. States: 0 check hit, 1 toss, 2 knocked.
 * @note Name is a project convention (ledger Duc_Can).
 */
class RPDucCan : public RPDucTarget {
public:
    typedef void (RPDucCan::*StateFunc)();

public:
    RPDucCan();
    virtual ~RPDucCan(); // at 0x8

    virtual void Load();      // at 0x10
    virtual void Calculate(); // at 0x18

    virtual void CalcModelMtx(); // at 0x24

    virtual void ResetParams();       // at 0x60
    virtual void Launch(u32 pattern); // at 0x64
    /**
     * @brief RPDucTarget::SpawnHitEffect override (break effect) [M]
     */
    virtual void SpawnHitEffect(); // at 0x68
    virtual void PlaySpawnSound(); // at 0x6C
    virtual void PlayHitSound(s32 player); // at 0x70
    virtual void ApplyVelocity();  // at 0x78

    void SetupLaunch();
    bool CheckShotHit();
    void CalcHitBounce(s32 player);
    void ApplyHitSpin(const nw4r::math::MTX34* pRotMtx);
    void StateCheckHit();
    void StateToss();
    void StateKnocked();

    static const s32 sPatternNum;

private:
    void AdjustPatterns();
    void LoadCanModels(EGG::Archive* pArchive, RPGrpModelResManager* pResMgr,
                        const char** names, const char* pFileName);

    RPDucCanParam mParam;          // at 0xFC
    nw4r::math::VEC3 mPrevPos;     // at 0x3DC position before the last step
    f32 mGravityUp;                // at 0x3E8
    f32 mGravityDown;              // at 0x3EC
    f32 mSpin[6];                  // at 0x3F0 axis mixing rates: x by y/z, y by x/z, z by x/y
    nw4r::math::VEC3 mBounceVel;   // at 0x408
    f32 mSpinAdd[6];               // at 0x414 added to mSpin by the next step
    s16 mHitNum;                   // at 0x42C [M] selects the damage model
    s16 mCooldown;                 // at 0x42E
    bool mBroken;                  // at 0x430
    s32 mState;                    // at 0x434
    RPGrpModel* mpModels[5];       // at 0x438 sht_can_00 .. sht_can_04
    StateFunc mStateFuncs[3];      // at 0x44C
};

//! @}

#endif

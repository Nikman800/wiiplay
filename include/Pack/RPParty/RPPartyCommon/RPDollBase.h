#ifndef RP_PARTY_COMMON_DOLL_BASE_H
#define RP_PARTY_COMMON_DOLL_BASE_H
#include <Pack/types_pack.h>

#include <Pack/RPGraphics/IRPGrpModelCallback.h>
#include <Pack/RPParty/RPPartyCommon/RPActorBase.h>

#include <egg/math.h>
#include <nw4r/math.h>

//! @addtogroup rp_party
//! @{

// Forward declarations
class RPFaceCtrl;
class RPGrpModel;
class RPJointMtxBuffer;
class RPSysKokeshi;

/**
 * @brief Mii body shown in the game-select scene (base of the player dolls and the crowd)
 * @details vtable 80358558. Size 0x1CC [M]: no allocation of the base itself; both derived
 * classes start their members at 0x1CC. mpKokeshi and mpFaceCtrl are created by the derived
 * constructors. The model callback (IRPGrpModelCallback part at 0x60, slots from 0x80) caches
 * the world positions of 14 joints (head, hip, arms, wrists, legs, ankles) in EditWorldMtx.
 * Up to four body animations are blended (mAnmIdx/mAnmWeight/mAnmWeightStep); the head turns
 * toward a look target, eased by mLookBlend.
 * vtable layout (CodeWarrior): RPActorBase slots to 0x74, IRPGrpModelCallback part at 0x78
 * (thunks -0x60), new virtuals from 0x94; EditWorldMtx gets a primary slot at 0xA8 because it
 * overrides a non-primary base.
 * @note Wii Play-only framework class (ledger Party_DollBase); the name is a project
 * convention.
 */
class RPDollBase : public RPActorBase, public IRPGrpModelCallback {
public:
    static const int JOINT_NUM = 14;
    static const int ANM_NUM = 4;

public:
    /**
     * @brief Pick ray for the hit tests (camera position along the pointer direction) [M]
     */
    struct Ray {
        EGG::Vector3f mDir;    // at 0x0 normalised
        EGG::Vector3f mOrigin; // at 0xC camera position
    };

public:
    explicit RPDollBase(s32 index);
    // Inline in the original: the vtable goes with the key function Load (RPDollBase.cpp);
    // RPDoll.cpp keeps the surviving weak copy
    virtual ~RPDollBase() {} // at 0x8

    /**
     * @brief Loads the Mii model, resolves the joint indices, builds the joint matrix buffer
     * and measures the upper/lower arm and leg lengths [H]
     */
    virtual void Load(); // at 0x94
    /**
     * @brief Clears the look state and the animation blend slots [H]
     */
    virtual void Reset(); // at 0x98
    /**
     * @brief Steps the look blend and the animation weights [H]
     */
    virtual void Calculate(); // at 0x9C
    /**
     * @brief Head direction toward a screen position: the camera ray meets the plane through
     * the doll facing the camera, pulled back by distance, then ClampLookDir [M]
     */
    virtual EGG::Vector3f ProjectLookTarget(const nw4r::math::VEC2& rScreenPos, f32 distance,
                                            f32 maxAngle); // at 0xA0
    /**
     * @brief Head direction toward a world position, clamped to maxAngle (radians) [M]
     */
    virtual EGG::Vector3f ClampLookDir(const nw4r::math::VEC3& rTarget,
                                       f32 maxAngle); // at 0xA4
    /**
     * @brief IRPGrpModelCallback override: caches the world positions of the 14 joints [H]
     */
    virtual void EditWorldMtx(RPGrpModel* pModel); // at 0xA8

    void LookAt(const nw4r::math::VEC2& rScreenPos, f32 distance, f32 maxAngle);
    void LookAtTarget(const nw4r::math::VEC3& rTarget, f32 maxAngle);
    void PlayAnm(u16 anm, f32 blendFrames);
    bool IsAnmDone(u16 anm);
    /**
     * @brief Part under a screen position: 0 head, 1 body, 2-5 arms and legs, -1 none [M]
     */
    s32 HitTestPart(const nw4r::math::VEC2& rScreenPos, s32 player) const;
    static f32 GetTurnSpeed();

    RPSysKokeshi* GetKokeshi() const {
        return mpKokeshi;
    }
    void ClearLook() {
        mIsLooking = false;
    }

protected:
    void UpdateModelMatrices();
    void StartBlendAnm(u16 anm, f32 step);
    /**
     * @brief Ray against a capsule, tested as spheres along a to b (closest hit in pDist) [M]
     */
    bool RayHitCapsule(const Ray& rRay, f32 radius, EGG::Vector3f a, EGG::Vector3f b,
                       f32* pDist) const;
    /**
     * @brief RayHitCapsule from joint a to joint b (HitTestPart's limb segments) [M]
     */
    bool RayHitJoints(const Ray& rRay, f32 radius, s32 a, s32 b, f32* pDist) const {
        return RayHitCapsule(rRay, radius, mJointPos[a], mJointPos[b], pDist);
    }
    /**
     * @brief Ray against a sphere; pDist gets the distance of the chord midpoint [M]
     */
    bool RayHitSphere(const Ray& rRay, const EGG::Vector3f& rCenter, f32 radius,
                      f32* pDist) const;

protected:
    s32 mIndex;                         // at 0x64 player / crowd slot
    nw4r::math::VEC3 mHeadPos;          // at 0x68 head attach matrix (right-hand joint) column 3
    nw4r::math::VEC3 mHeadAxisY;        // at 0x74 column 1
    nw4r::math::VEC3 mHeadAxisX;        // at 0x80 column 0
    bool mIsLooking;                    // at 0x8C
    f32 mLookWeight;                    // at 0x90 0..1, rises while looking
    f32 mLookBlend;                     // at 0x94 0..1, from mLookFrom to mLookTo
    f32 mLookSpeed;                     // at 0x98 mLookBlend step (GetTurnSpeed)
    nw4r::math::VEC3 mLookFrom;         // at 0x9C
    nw4r::math::VEC3 mLookTo;           // at 0xA8
    nw4r::math::VEC3 mLookDir;          // at 0xB4 current head direction
    RPFaceCtrl* mpFaceCtrl;             // at 0xC0 created by the derived constructor
    u16 mJointIds[JOINT_NUM];           // at 0xC4
    EGG::Vector3f mJointPos[JOINT_NUM]; // at 0xE0 world positions (EditWorldMtx)
    RPJointMtxBuffer* mpJointMtxBuffer; // at 0x188
    f32 mBoneLengths[3];                // at 0x18C offset lengths of arm_l1, arm_l2, wrist_l
    s32 mAnmIdx[ANM_NUM];               // at 0x198 -1 = slot unused
    f32 mAnmWeight[ANM_NUM];            // at 0x1A8
    f32 mAnmWeightStep[ANM_NUM];        // at 0x1B8
    RPSysKokeshi* mpKokeshi;            // at 0x1C8 created by the derived constructor

public:
    static f32 sLookInStep;
    static f32 sLookOutStep;
    static f32 sTurnSpeed;
    static f32 sHitRadiusA;
    static f32 sHitRadiusB;
    static f32 sHitRadiusC;
    static f32 sHitRadiusD;
    static f32 sHitRadiusE;
};

//! @}

#endif

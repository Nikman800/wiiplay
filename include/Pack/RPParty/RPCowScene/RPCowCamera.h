#ifndef RP_PARTY_COW_CAMERA_H
#define RP_PARTY_COW_CAMERA_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPActor.h>

#include <egg/math.h>

//! @addtogroup rp_cow
//! @{

// Forward declarations
class RPCollisionShape;
class RPCowPlayer;
class RPGrpCamera;
class RPGrpModel;
class RPPathObject;

/**
 * @brief Per-player camera (one per player, two allocated as an array)
 * @details vtable 8035CE40. Size 0xD0, verified (array stride in the RPCowGame constructor).
 * The camera is a state machine: mpState points at one of twelve 4-byte state member
 * objects (vtable pointer only, at 0xA0..0xCC: overview, follow ease, follow snap, follow
 * near, follow, finish, result, result orbit, intro, intro blend, ready, start); Calculate
 * runs mpState->Calc(this). The state vtables are .data 8035CEA0..8035CF30 ({0, 0, Calc}
 * each, in reverse declaration order), stored by the inlined state constructors. mViewA/mViewB are the renderer view indices
 * used for the 3D and the second view (1P: 0/2, 2P: 1/3 or 4/4).
 * @note Name is a project convention (ledger Cow_Camera).
 */
class RPCowCamera : public RPActor {
public:
    /**
     * @brief A state of the camera: a 4-byte member object whose only virtual runs the state on
     * the owner (mpState->Calc(this)) [H]
     */
    class State {
    public:
        virtual void Calc(RPCowCamera* pOwner) = 0; // at 0x8
    };
#define RP_COW_CAMERA_STATE(name)                                                            \
    class name : public State {                                                              \
    public:                                                                                  \
        virtual void Calc(RPCowCamera* pOwner);                                              \
    };
    RP_COW_CAMERA_STATE(StateOverview)
    RP_COW_CAMERA_STATE(StateFollowEase)
    RP_COW_CAMERA_STATE(StateFollowSnap)
    RP_COW_CAMERA_STATE(StateFollowNear)
    RP_COW_CAMERA_STATE(StateFollow)
    RP_COW_CAMERA_STATE(StateFinish)
    RP_COW_CAMERA_STATE(StateResult)
    RP_COW_CAMERA_STATE(StateResultOrbit)
    RP_COW_CAMERA_STATE(StateIntro)
    RP_COW_CAMERA_STATE(StateIntroBlend)
    RP_COW_CAMERA_STATE(StateReady)
    RP_COW_CAMERA_STATE(StateStart)
#undef RP_COW_CAMERA_STATE

public:
    RPCowCamera();

    /**
     * @brief Ledger create: sets up the screens/views of this player [H]
     * @details Declared before the destructor so that it is the key function (vtable and
     * thunks in RPCowCamera_2); the destructor sits in a unit of its own (RPCowCamera).
     */
    virtual void VF_0x3C(); // at 0x3C
    virtual ~RPCowCamera(); // at 0x38
    /**
     * @brief Ledger destroy (empty) [H]
     */
    virtual void VF_0x40(); // at 0x40
    /**
     * @brief Ledger init: lights, light map, collision shape, path object [H]
     */
    virtual void LoadResource(); // at 0x44
    /**
     * @brief Ledger reset [H]
     */
    virtual void OnStart(); // at 0x48
    /**
     * @brief Ledger exit [H]
     */
    virtual void OnStop(); // at 0x4C
    /**
     * @brief Ledger calc: runs the current state and updates the views [M]
     */
    virtual void OnActivate(); // at 0x50
    /**
     * @brief Ledger entryDraw [M]
     */
    virtual void Calculate(); // at 0x54
    /**
     * @brief Ledger drawPass (empty) [H]
     */
    virtual void DrawPass(u8 pass); // at 0x5C

    void StartIntro();
    void SetupIntro();
    bool IsIntroDone() const;
    // In one of the intro states (8, 9) [M]
    bool IsInIntro() const {
        return mpState == &mStateIntro || mpState == &mStateIntroBlend;
    }
    void SetupReady();
    void StartStart();
    bool IsPullbackDone();
    void SetFollowState();
    void SetFollowNearState();
    void StartFinish();
    void StartResult();
    void SetModelVisible(bool visible);
    /**
     * @brief Screen position (RPGrpCamera::GetWorldToScreen with the current screen), returned
     * by value (RPCowTargetMarker::OnActivate) [H]
     */
    EGG::Vector3f ProjectToScreen(const EGG::Vector3f& rPos) const;
    /**
     * @brief ProjectToScreen, but a point closer than 30 along the view direction (depth) is
     * first pushed back along it (RPCowPlayerCursor::Calc) [H]
     */
    EGG::Vector3f ProjectToScreenClamped(const EGG::Vector3f& rPos, f32 depth) const;
    EGG::Vector3f GetBackDir() const;
    EGG::Vector3f GetEyePos() const;
    /**
     * @brief Both side planes of the view (unk5C, set by OnActivate) face the point [M]
     */
    bool IsInView(const EGG::Vector3f& rPos) const;
    // Squared distance from the eye [H]
    f32 CalcSquareDistance(const EGG::Vector3f& rPos) const;

private:
    /**
     * @brief Named tuning constants (.sdata2 804409C0..80440A5C and .sbss2 804419A0, defined at
     * the end of the camera file so that they are loaded, not folded) [M]
     */
    static const f32 sModelScale;          // 1
    static const f32 sIntroStartHeight;    // 20
    static const f32 sIntroHeight;         // 15
    static const f32 sIntroStartDist;      // 200
    static const f32 sIntroEndDist1P;      // 140
    static const f32 sIntroEndDist2P;      // 120
    static const f32 sIntroDist1P;         // 35
    static const f32 sIntroDist2P;         // 20
    static const f32 sIntroRange1P;        // 4
    static const f32 sIntroRange2P;        // 4
    static const f32 sIntroYawDeg;         // 180
    static const f32 sIntroBlendYawDeg;    // 240
    static const f32 sIntroTurnSpeedDeg;   // 1
    static const f32 sReadyStartHeight;    // 14
    static const f32 sReadyStartDist;      // 20
    static const f32 sReadyDist2P;         // 30
    static const f32 sReadyDist1P;         // 40
    static const f32 sPullbackDist;        // 40
    static const f32 sStartDist;           // 80
    static const f32 sReadyHeight2P;       // 15
    static const f32 sReadyHeight1P;       // 16
    static const f32 sStartHeight;         // 20
    static const f32 sFollowHeight;        // 20
    static const f32 sFollowDist;          // 80
    static const f32 sSnapRate;            // 1
    static const f32 sTargetRate;          // 1
    static const f32 sFollowNearDist;      // 60
    static const f32 sResultDist;          // 100
    static const f32 sOrbitHeight;         // 80
    static const f32 sOrbitAccel;          // 1
    static const f32 sOrbitDist;           // 320
    static const f32 sOrbitDistAccel;      // 16
    static const f32 sFovy;                // 20 (16:9; 4:3 scales it by 4/3)
    static const f32 sOverviewHeight;      // 1000
    static const f32 sOverviewDist;        // -1500
    static const f32 sNearDist;            // 30
    static const f32 sFarDist;             // 2500
    static const f32 sViewMargin;          // 100
    static const f32 sReadyYawDeg;         // 0 (.sbss2)
    static const f32 sUnusedZero;          // 0 (.sbss2 804419A4, unreferenced)

    void SetView(const EGG::Vector3f& rTarget);
    void SetViewFromPlayer(RPCowPlayer* pPlayer);

    RPPathObject* mpPathObj;           // at 0x24 player start (resource type 0x70001)
    RPGrpModel* mpModel;               // at 0x28 [L]
    u8 mViewA;                         // at 0x2C
    u8 mViewB;                         // at 0x2D
    char unk2E[0x30 - 0x2E];
    f32 mNearDistSq;                   // at 0x30 [L] 30^2
    f32 mFarDistSq;                    // at 0x34 [L] 2500^2
    EGG::Vector3f mViewDir;            // at 0x38 target - eye
    EGG::Vector3f mEdgeL;              // at 0x44 view direction + side
    EGG::Vector3f mEdgeR;              // at 0x50 view direction - side
    f32 unk5C[6];                      // at 0x5C [L] side plane normals read by IsInView
    void* mpDemoLight;                 // at 0x74 demo_Light1P/2P.plight
    void* mpSceneLight;                // at 0x78 scene_Light1P/2P.plight
    /**
     * @brief Draw camera from Gfx_createDrawModel; its matrix holds the view axes (back
     * direction negated from +0x100) [L]
     */
    RPGrpCamera* mpDrawCamera;         // at 0x7C
    RPCollisionShape* mpCollision;     // at 0x80
    f32 mBlend;                        // at 0x84 intro: eases from the player index (or 0.5) toward this player
    f32 mBlendVel;                     // at 0x88
    f32 mTargetY;                      // at 0x8C look-at height (11.3 above the player)
    f32 mTargetYVel;                   // at 0x90
    s32 mTimer;                        // at 0x94 result state frames
    State* mpState;                    // at 0x98 one of the state members below
    s32 unk9C;                         // at 0x9C [L]
    StateOverview mStateOverview;      // at 0xA0
    StateFollowEase mStateFollowEase;  // at 0xA4
    StateFollowSnap mStateFollowSnap;  // at 0xA8
    StateFollowNear mStateFollowNear;  // at 0xAC
    StateFollow mStateFollow;          // at 0xB0
    StateFinish mStateFinish;          // at 0xB4
    StateResult mStateResult;          // at 0xB8
    StateResultOrbit mStateResultOrbit; // at 0xBC
    StateIntro mStateIntro;            // at 0xC0
    StateIntroBlend mStateIntroBlend;  // at 0xC4
    StateReady mStateReady;            // at 0xC8
    StateStart mStateStart;            // at 0xCC
};

//! @}

#endif

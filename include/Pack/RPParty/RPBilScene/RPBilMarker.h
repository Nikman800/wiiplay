#ifndef RP_PARTY_BIL_MARKER_H
#define RP_PARTY_BIL_MARKER_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPBilScene/RPBilObject.h>

#include <nw4r/math.h>

//! @addtogroup rp_bil
//! @{

// Forward declarations
class RPGrpModel;

/**
 * @brief Marker shown at a predicted ball position while aiming (ten made by RPBilBallMgr)
 * @details vtable 8035E1E0. Size 0x68, verified (Bil_BallMgr_ctor). Load builds the
 * bad_ball_2 model (anim frame 150); Calculate updates the model while visible.
 */
class RPBilAimMarker : public RPBilActor {
public:
    explicit RPBilAimMarker(s32 index);
    virtual ~RPBilAimMarker(); // at 0x8

    virtual void Load();      // at 0x78
    virtual void Reset();     // at 0x7C
    virtual void Calculate(); // at 0x80
    virtual void Exit();      // at 0x84
    virtual void UserDraw(); // at 0x88 empty

    void SetVisible(bool visible) {
        mVisible = visible;
    }

private:
    s32 mIndex;    // at 0x60
    bool mVisible; // at 0x64
};

/**
 * @brief Ghost cue ball showing where the ball in hand would be placed (RPBilBallMgr + 0xCC)
 * @details vtable 8035E2B8. Size 0x64, verified (Bil_BallMgr_ctor allocates 100). The
 * bad_ball_2 animation frame shows whether the position is valid (frame 255) or not (128).
 */
class RPBilCueGhost : public RPActorBase {
public:
    RPBilCueGhost();
    virtual ~RPBilCueGhost(); // at 0x8

    virtual void Load();      // at 0x78
    virtual void Reset();     // at 0x7C
    virtual void Exit();      // at 0x80
    virtual void Calculate(); // at 0x84

    virtual void UserDraw(); // at 0x88 empty

public:
    bool mVisible; // at 0x60
    bool mIsValid; // at 0x61 [M] placement allowed (RPBilBallMgr::CanPlaceBall)
};

/**
 * @brief Two bad_next_ball markers over the lowest ball still on the table (singleton
 * RPBilNextBallMarker::sInstance)
 * @details vtable 8035E3C0. Size 0x7C, verified (Bil_NextBallMarker_create). Overrides
 * CalcModelMtx to place both models at mPos, scaled by the ball radius.
 */
class RPBilNextBallMarker : public RPActorBase {
public:
    static RPBilNextBallMarker* sInstance; // 8043CA28

    static const int MODEL_NUM = 2;

public:
    RPBilNextBallMarker();
    virtual ~RPBilNextBallMarker(); // at 0x8

    /**
     * @brief Places both models (ledger Bil_NextBallMarker_updateModels) [M]
     */
    virtual void CalcModelMtx(); // at 0x74

    virtual void Load();      // at 0x78
    virtual void Reset();     // at 0x7C
    virtual void Calculate(); // at 0x80
    virtual void Exit();      // at 0x84

    virtual void UserDraw(); // at 0x88

    /**
     * @brief Moves to the lowest remaining ball and shows the markers; false when none is
     * left [H]
     */
    bool Show();

    void SetVisible(bool visible) {
        mVisible = visible;
    }

    static void CreateInstance();
    static void DestroyInstance();

private:
    bool mVisible;                        // at 0x60
    nw4r::math::VEC3 mBallPos;            // at 0x64
    f32 mBallRadius;                      // at 0x70 the ball's phys sphere radius
    RPGrpModel* mpModels[MODEL_NUM];      // at 0x74
};

//! @}

#endif

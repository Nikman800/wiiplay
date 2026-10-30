#ifndef RP_PARTY_BOM_PANEL_2_H
#define RP_PARTY_BOM_PANEL_2_H

#include <Pack/types_pack.h>

#include <Pack/RPGraphics/IRPGrpModelCallback.h>
#include <Pack/RPParty/RPBomScene/RPBomObjPool.h>

#include <nw4r/math.h>

//! @addtogroup rp_bom
//! @{

// Forward declarations
class RPBomPlayerSound;
class RPGrpModel;

/**
 * @brief One falling bubble (panel with a pose keyhole, bom_panelA/B/C_model by pose)
 * @details vtable 8035F100 (IRPGrpModelCallback part at 0xA4; EditWorldMtx also gets the
 * primary slot 0xBC). Size 0x11C, verified (RPBomPanelPool::CreateItem). Setup flags: bits
 * 0-1 pose, bits 4-5 colour type (0 normal, 1 two-player target, 2 special), bit 31 start
 * bubble (level intro). States (mState, PTMFs copied from the file's table): 0 none, 1 fall
 * (sways sideways by mOrbitRadius, missed past RPBomQuestionParamFile::mMissZ), 2 hit.
 * CalcHit finds the nearest posing player within 6.3 * scale; after 4 overlapping frames with
 * the Mii facing the same way (cos 20 degrees) and the same pose the bubble is matched and
 * PostCalc scores it.
 */
class RPBomPanel : public RPBomPoolObj, public IRPGrpModelCallback {
public:
    typedef void (RPBomPanel::*StateFunc)();

public:
    RPBomPanel();
    virtual ~RPBomPanel(); // at 0x8

    virtual void Create();       // at 0xC creates the sound actor (remote flag 0xF)
    virtual void Load();         // at 0x10 LoadModels
    virtual void Reset();        // at 0x14
    virtual void Calculate();    // at 0x18 runs mStateFuncs[mState], moves the sound actor
    virtual void CalcModelMtx(); // at 0x24

    virtual void ResetState();     // at 0x58 RPBomPoolObj's
    virtual void InitFields();     // at 0x64
    virtual void Setup(u32 flags); // at 0x68
    virtual void Move();           // at 0x6C moves by mVel and turns by mRotSpeed
    virtual void PreCalc();        // at 0x70 RPBomPoolObj's
    virtual void CalcHit();        // at 0x74
    virtual void PostCalc();       // at 0x78
    virtual void Deactivate();     // at 0x80
    virtual void Spawn(const nw4r::math::VEC3& rPos, u32 flags,
                       RPBomSpawnInfo info); // at 0x88
    virtual s32 GetColorType() {                     // at 0x8C
        return mColorType;
    }
    virtual s32 GetPoseIndex() { // at 0x90
        return mPose;
    }
    virtual bool IsFalling() { // at 0x98 not hit yet
        return mState != 2;
    }
    virtual void Miss(); // at 0x9C Bom_bubble_miss effect and sound, then Deactivate

    /**
     * @brief Places panel_node at the bubble's transform [H]
     */
    virtual void EditWorldMtx(RPGrpModel* pModel); // at 0xBC

    void CheckPlayerMatch();
    void PlayOkEffects(s32 player, s32 combo);
    void LoadModels();

    void StateNone();
    void StateFall();
    void StateHit();

private:
    void GetEffectMtx(nw4r::math::MTX34& rMtx) const;
    void CancelColorAnms();

public:
    f32 mAngle;                     // at 0xA8 y rotation (degrees)
    f32 mRotSpeed;                  // at 0xAC
    f32 mBaseScale;                 // at 0xB0
    f32 mOrbitRadius;               // at 0xB4
    f32 mOrbitStep;                 // at 0xB8 (1.5 for start bubbles)
    f32 mOrbitAngle;                // at 0xBC 0 or 180 (random for start bubbles)
    s32 mHitPlayer;                 // at 0xC0 -1
    bool mIsStartBubble;            // at 0xC4 setup flag bit 31
    s32 mMatchPlayer;               // at 0xC8 nearest posing player, -1
    s32 mOverlapFrames;             // at 0xCC
    s32 mAlignedFrames;             // at 0xD0
    f32 mMatchDist;                 // at 0xD4
    BOOL mMatched;                  // at 0xD8
    s32 mColorType;                 // at 0xDC
    s32 mState;                     // at 0xE0
    s32 mPose;                      // at 0xE4
    RPGrpModel* mpModels[3];        // at 0xE8 by pose
    RPBomPlayerSound* mpPanelSound; // at 0xF4
    StateFunc mStateFuncs[3];       // at 0xF8
};

//! @}

#endif

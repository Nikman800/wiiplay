#ifndef RP_PARTY_BOM_BORDER_H
#define RP_PARTY_BOM_BORDER_H
#include <Pack/types_pack.h>

#include <Pack/RPGraphics/IRPGrpModelCallback.h>
#include <Pack/RPParty/RPPartyCommon/RPModelObj.h>

#include <nw4r/math.h>

//! @addtogroup rp_bom
//! @{

// Forward declarations
class RPGrpModel;

class RPBomBorder;

/**
 * @brief The three border (miss counter) models at the top of the screen (singleton
 * RPBomBorder::sInstance)
 * @details vtable 8035F708 (IRPGrpModelCallback part at 0x48, vtable 8035F760). Size 0x158,
 * verified (Bom_Border_createInstance). One border_model per allowed miss; CalcModelMtx sets
 * bit 0 of the model flags for borders below the misses left (RPBomGameManager mMissMax -
 * mMissCount) and clears it for the others [M]. States (mState, table
 * Bom_borderStateFuncs copied by CreateInstance): 0 intro drop, 1 none, 2 flash (level start
 * / level up, 80 frames), 3 idle (breaks a border with Bom_borderbreak01 on a miss), 4 miss
 * wait, 5 rise (all clear / game over).
 * @note Name is a project convention (ledger Bom_Border).
 */
class RPBomBorder : public RPModelObj, public IRPGrpModelCallback {
public:
    static RPBomBorder* sInstance; // 8043CAD8

    typedef void (RPBomBorder::*StateFunc)();

public:
    RPBomBorder();
    virtual ~RPBomBorder(); // at 0x8

    virtual void Create();    // at 0xC empty
    /**
     * @brief Loads border_model.brres: three models with border_model / border_landing and
     * border_normal / border_miss animations [H]
     */
    virtual void Load();         // at 0x10
    virtual void Reset();        // at 0x14
    virtual void Calculate();    // at 0x18 runs mStateFuncs[mState]
    virtual void CalcModelMtx(); // at 0x24 ledger updateModels

    /**
     * @brief IRPGrpModelCallback::EditWorldMtx override: copies the border_model node of the
     * matching model into mNodeMtx (ledger attachBorderNode) [H]
     */
    virtual void EditWorldMtx(RPGrpModel* pModel); // at 0x74

    /**
     * @brief Starts the intro drop (state 0): the borders start below their place, delayed by
     * RPBomBorder::sIntroDelays [H]
     */
    void StartIntro();
    /**
     * @brief Starts the flash (state 2, 80 frames) at a level start / level up [H]
     */
    void StartFlash();

    void StateIntroDrop(); // 0
    void StateNone();      // 1
    void StateFlash();     // 2
    void StateIdle();      // 3
    void StateMissWait();  // 4
    void StateRise();      // 5

    static void CreateInstance();
    static void DestroyInstance();

private:
    void PlayFlashAnm();
    void PlayNormalAnm();
    void ResetOffsets();

public:
    bool mAnimating;                  // at 0x4C [M] UpdateFrame also while set (flash)
private:
    s32 unk50;                        // at 0x50 [L] 3 after Reset, never read
    s32 mTimer;                       // at 0x54 flash frames (80)
public:
    s32 mState;                       // at 0x58
private:
    bool mFallSePlayed[3];            // at 0x5C intro drop: fall sound started
    bool mLanded[3];                  // at 0x5F intro drop: in place, landing animation
    bool mSettled[3];                 // at 0x62 intro drop: landing animation done
    f32 mOffsetZ[3];                  // at 0x68 per border z offset (-16 * index)
    nw4r::math::MTX34 mNodeMtx[3];    // at 0x74 border_model nodes (break effect position)
    RPGrpModel* mpModels[3];          // at 0x104
    StateFunc mStateFuncs[6];         // at 0x110

public:
    static const f32 sIntroDelays[3];
};

//! @}

#endif

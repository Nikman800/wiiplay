#ifndef RP_PARTY_BOM_START_BG_H
#define RP_PARTY_BOM_START_BG_H
#include <Pack/types_pack.h>

#include <Pack/RPGraphics/IRPGrpModelCallback.h>
#include <Pack/RPParty/RPPartyCommon/RPModelObj.h>

#include <nw4r/math.h>

//! @addtogroup rp_bom
//! @{

// Forward declarations
class RPBomStartBg;
class RPGrpModel;
class RPSysEffect;

// STAND-IN (dead-stripped), see RPBomStartBg.cpp
extern "C" void Bom_StartBg_poolOrder(RPBomStartBg* pBg);

/**
 * @brief Background (bg_model with its colour animation) and the bg_acc_model accessory, plus
 * the start wipe effect (singleton RPBomStartBg::sInstance)
 * @details vtable 8035E728 (IRPGrpModelCallback part at 0x48, vtable 8035E780). Size 0x9C,
 * verified (Bom_StartBg_createInstance). The background's frame (colour) is the level's entry
 * in the parameter file (RPBomLevelData::mBgFrame); SetLevel with a level-up plays
 * Bom_levelup01 and switches the frame after 17 frames. The material colour of the two
 * background materials fades toward mFadeTarget (255 normal, 120 while bubbles are on screen).
 * The model and the effect are not owned through RPModelObj::mpModel.
 * @note Name is a project convention (ledger Bom_StartBg).
 */
class RPBomStartBg : public RPModelObj, public IRPGrpModelCallback {
public:
    static RPBomStartBg* sInstance; // 8043CA70

    virtual ~RPBomStartBg(); // at 0x8

    /**
     * @brief Creates the start effect (Bom_startbg on 16:9, Bom_startbg_st on 4:3) [H]
     */
    virtual void Create(); // at 0xC
    /**
     * @brief Loads bg_model.brres and bom_bg_acc_model.brres [H]
     */
    virtual void Load();      // at 0x10
    virtual void Reset();     // at 0x14
    virtual void Calculate(); // at 0x18
    /**
     * @brief Sets both models' matrices (mRot rotated by Bom_rotXNeg90) and scale, then
     * UpdateFrame/Calc/Entry [H]
     */
    virtual void CalcModelMtx(); // at 0x24

    /**
     * @brief IRPGrpModelCallback::EditWorldMtx override: copies the bg_model node matrix into
     * mNodeMtx (ledger attachBgModelNode) [H]
     */
    virtual void EditWorldMtx(RPGrpModel* pModel); // at 0x74

    void SetFadeTargetFull();
    void SetFadeTargetDim();
    /**
     * @brief Sets the level; with levelUp the Bom_levelup01 effect and jingle play first [H]
     */
    void SetLevel(s32 level, BOOL levelUp);
    void PlayWipe();
    void UpdateFade();
    void LoadBgModel();
    void LoadBgAccModel();
    /**
     * @brief Shows the level's background frame (RPBomLevelData::mBgFrame) and clears the
     * level-up wait [H]
     */
    void StartLevelFrame();

    static void CreateInstance();
    static void DestroyInstance();

    friend void Bom_StartBg_poolOrder(RPBomStartBg* pBg);

public:
    s32 mLevel;                     // at 0x4C
private:
    s32 mLevelUpTimer;              // at 0x50 17 after a level-up
    f32 mFade;                      // at 0x54 material colour 0-255 (+3 / -1.5 per frame)
    f32 mFadeTarget;                // at 0x58
    bool mLevelUpPending;           // at 0x5C
    RPGrpModel* mpBgModel;          // at 0x60 bg_model
    RPGrpModel* mpBgAccModel;       // at 0x64 bg_acc_model (bg_acc_16_9 or 4:3 animation)
    nw4r::math::MTX34 mNodeMtx;     // at 0x68 bg_model node, places Bom_levelup01
    RPSysEffect* mpEffect;          // at 0x98 Bom_startbg / Bom_startbg_st
};

//! @}

#endif

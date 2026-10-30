#ifndef RP_PARTY_FSH_STAGE_H
#define RP_PARTY_FSH_STAGE_H
#include <Pack/types_pack.h>

#include <Pack/RPKernel/IRPSysHostIOSocket.h>
#include <Pack/RPKernel/RP_DEBUG_STUB_1.h>

#include <nw4r/math.h>

//! @addtogroup rp_fsh
//! @{

// Forward declarations
class RPGrpModel;
class RPGrpProjectionObject;
class RPKclHolder;
class RPGrpShadowTexture;

/**
 * @brief Fishing stage: lake, trees, clouds, water surface and the lake shadow texture
 * (singleton Fsh_stage)
 * @details vtable 80358DE8. Size 0x24, verified (Fsh_Stage_new). Create builds the four models
 * (the tree, cloud and water models get one looping animation each), a 256x128 shadow texture
 * (RPGrpShadowTexture, ledger Fsh_SceneTexture) attached to the lake's shadow_dummy material,
 * and loads the scene camera into the projection object. The pond is a circle of radius 210
 * around Fsh_pondCenter, water surface y -15, bottom y -104.
 */
class RPFshStage : public RP_DEBUG_STUB_1, public IRPSysHostIOSocket {
public:
    enum EModel {
        MODEL_LAKE,
        MODEL_TREE,
        MODEL_CLOUD,
        MODEL_WATERFACE,

        MODEL_NUM
    };

public:
    RPFshStage();
    virtual ~RPFshStage(); // at 0x8

    virtual void Create();    // at 0x20
    virtual void Reset();     // at 0x24 empty, out of line (among the normal functions)
    virtual void Calculate(); // at 0x28
    virtual void Exit() {}    // at 0x2C

    RPKclHolder* GetKcl() const {
        return mpKcl;
    }
    RPGrpShadowTexture* GetShadowTexture() const {
        return mpShadowTexture;
    }

    static f32 GetPondRadius();     // 210
    static f32 GetPondBottomY();    // -104
    static f32 GetWaterSurfaceY();  // -15
    // Wii Play: a member that does not use this (callers pass sInstance)
    bool IsInPond(const nw4r::math::VEC3& rPos);

    static void CreateInstance();
    static void DestroyInstance();
    static RPFshStage* GetInstance() {
        return sInstance;
    }

private:
    /**
     * @brief Pond collision; Create gives it lake.kcl from the game common archive
     */
    RPKclHolder* mpKcl;                     // at 0x8
    RPGrpModel* mpModels[MODEL_NUM];        // at 0xC
    RPGrpShadowTexture* mpShadowTexture;    // at 0x1C
    RPGrpProjectionObject* mpProjection;    // at 0x20

    static RPFshStage* sInstance; // 8043C758
};

//! @}

#endif

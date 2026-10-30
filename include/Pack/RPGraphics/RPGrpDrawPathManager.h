#ifndef RP_GRAPHICS_DRAW_PATH_MANAGER_H
#define RP_GRAPHICS_DRAW_PATH_MANAGER_H
#include <Pack/types_pack.h>

#include <egg/gfxe.h>

//! @addtogroup rp_graphics
//! @{

// Forward declarations
class RPGrpDrawPathBloom;
class RPGrpDrawPathDOF;
class RPGrpDrawPathHDR;
class RPGrpDrawPathLightMap;
class RPGrpDrawPathShadowTexture;
class RPGrpDrawPathSV;
class RPGrpDrawPathXluSnap;
class RPGrpModelScene;
class RPScnProcDrawHook;

/**
 * @brief Draw path manager
 */
class RPGrpDrawPathManager : public EGG::IScnProc {
public:
    /**
     * @brief Flags to enable draw path types with Construct
     */
    enum {
        EFlag_DrawPathLightMap = 1 << 0,
        EFlag_DrawPathShadowTexture = 1 << 1,
        EFlag_DrawPathXluSnap = 1 << 2,
        EFlag_DrawPathSV = 1 << 3,
        EFlag_DrawPathHDR = 1 << 4,
        EFlag_DrawPathBloom = 1 << 5,
        EFlag_DrawPathDOF = 1 << 6,
    };

public:
    static RPGrpDrawPathManager* Construct(u32 pathFlags,
                                           RPGrpModelScene* pModelScene);

    virtual ~RPGrpDrawPathManager() override {} // at 0x8

    virtual void scnProcDraw(u16 idx) override; // at 0xC

    // Wii Play: used by RPHkyGame
    // Wii Play: RPPnpGame::Load
    RPGrpDrawPathShadowTexture* GetDrawPathShadowTexture() const {
        return mpDrawPathShadowTexture;
    }
    RPGrpDrawPathSV* GetDrawPathSV() const {
        return mpDrawPathSV;
    }
    RPGrpDrawPathBloom* GetDrawPathBloom() const {
        return mpDrawPathBloom;
    }
    // Wii Play: RPFshBonusFish
    RPGrpDrawPathXluSnap* GetDrawPathXluSnap() const {
        return mpDrawPathXluSnap;
    }
    RPGrpDrawPathLightMap* GetDrawPathLightMap() const {
        return mpDrawPathLightMap;
    }

private:
    RPGrpDrawPathManager(u32 pathFlags, RPGrpModelScene* pModelScene);

private:
    //! Wii Play: scene renderer timing of each of the manager's six procs
    static const u32 PROC_TIMINGS[];

public:
    // Wii Play: RPGrpRenderer::LinkViews
    void SetDrawHook(RPScnProcDrawHook* pHook) {
        mpDrawHook = pHook;
    }

private:
    u16 unkA;
    u16 unkC;
    RPGrpModelScene* mpModelScene; // at 0x10
    //! Wii Play: per-step draw callback, set by RPGrpRenderer::LinkViews (ledger
    //! Gfx_ScnProc_drawStage calls its slot 0xC with the step)
    RPScnProcDrawHook* mpDrawHook; // at 0x14
    RPGrpDrawPathLightMap* mpDrawPathLightMap;           // at 0x18
    RPGrpDrawPathShadowTexture* mpDrawPathShadowTexture; // at 0x1C
    RPGrpDrawPathXluSnap* mpDrawPathXluSnap;             // at 0x20
    RPGrpDrawPathSV* mpDrawPathSV;                       // at 0x24
    RPGrpDrawPathHDR* mpDrawPathHDR;                     // at 0x28
    RPGrpDrawPathBloom* mpDrawPathBloom;                 // at 0x2C
    RPGrpDrawPathDOF* mpDrawPathDOF;                     // at 0x30
};

//! @}

#endif

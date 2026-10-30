#ifndef RP_GRAPHICS_MODEL_MANAGER_H
#define RP_GRAPHICS_MODEL_MANAGER_H
#include <Pack/types_pack.h>

#include <egg/core.h>

//! @addtogroup rp_graphics
//! @{

// Forward declarations
class RPGrpDrawPathManager;
class RPGrpModelMisc;
class RPGrpModelResManager;
class RPGrpModelScene;
namespace EGG {
class FogManager;
class LightManager;
} // namespace EGG

/**
 * @brief Model manager
 * @details Wii Play: size 0x38, verified (RPGrpRenderer::Construct allocates 0x38); the vendored
 * header only had the scene table (and placed it at 0x4). vtable 8034FEB0 (destructor only).
 * Ledger name Gfx_ModelSceneMgr.
 */
class RPGrpModelManager {
public:
    //! Maximum number of model scenes
    static const int MAX_SCENE = 8;

public:
    /**
     * @brief Gets the currently active manager instance
     */
    static RPGrpModelManager* GetCurrent() {
        return spCurrent;
    }

    /**
     * @brief Constructor (ledger Gfx_ModelSceneMgr_ctor)
     * @details Creates and configures the resource manager and an RPGrpModelMisc [H]
     */
    RPGrpModelManager(EGG::Allocator* pAllocator, EGG::Allocator* pScnAllocator);

    /**
     * @brief Creates model scene drawScene (new RPGrpModelScene, 0xA8) [H]
     */
    void CreateModelScene(u8 drawScene, u32 pathFlags, u8 lightNum,
                          EGG::LightManager* pLightManager,
                          EGG::FogManager* pFogManager);

    /**
     * @brief Makes this manager current: sets the static current pointers [H]
     */
    void BecomeCurrent();
    static void ClearCurrent();

    EGG::LightManager* GetLightManager(u8 drawScene) const; // [H]
    RPGrpDrawPathManager* GetDrawPathManager(u8 drawScene) const;

    void TerminateResources();    // [M] ledger clearResources
    void CalcModelScenes();       // [M] ledger calcAll
    void CalcWorldModelScenes();  // [L] ledger calcWorldAll
    void ClearDrawEntries();      // [M] ledger clearEntriesAll
    void ResetViews();            // [L] ledger resetViewsAll
    void DrawPost();              // [L] ledger drawPostAll

    // Wii Play: RPPnpGame::Load
    RPGrpModelMisc* GetModelMisc() const {
        return mpModelMisc;
    }
    RPGrpModelScene* GetModelScene(int i) {
        return mpModelScenes[i];
    }
    // Wii Play: RPGrpRenderer::PostCalculate
    void SetCalcDisabled(bool disabled) {
        mCalcDisabled = disabled;
    }

private:
    //! Model scenes
    RPGrpModelScene* mpModelScenes[MAX_SCENE]; // at 0x0
    RPGrpModelResManager* mpResManager;        // at 0x20
    //! Becomes RPGrpModel::spAllocator when current
    EGG::Allocator* mpAllocator; // at 0x24
    //! Allocator for the g3d scene roots (static 8043C5C0 when current) [M]
    EGG::Allocator* mpScnAllocator; // at 0x28
    //! Skips CalcModelScenes while set [M]
    bool mCalcDisabled; // at 0x2C

public:
    virtual ~RPGrpModelManager(); // at 0x8

private:
    RPGrpModelMisc* mpModelMisc; // at 0x34

    //! Allocator used for model-related allocations
    static RPGrpModelManager* spCurrent;
};

//! @}

#endif

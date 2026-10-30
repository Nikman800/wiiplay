#ifndef RP_GRAPHICS_MODEL_SCENE_H
#define RP_GRAPHICS_MODEL_SCENE_H
#include <Pack/types_pack.h>

//! @addtogroup rp_graphics
//! @{

// Forward declarations
class RPGrpCamera;
class RPGrpDrawPathManager;
class RPGrpModel;
class RPGrpProjectionObject;
class RPGrpScreen;
namespace EGG {
class Allocator;
class FogManager;
class LightManager;
class ScnRenderer;
} // namespace EGG
namespace nw4r {
namespace g3d {
class ScnRoot;
} // namespace g3d
} // namespace nw4r

/**
 * @brief Model rendering scene
 * @details Wii Play: size 0xA8, verified (RPGrpModelManager allocates 0xA8). vtable 80350138 at
 * 0xA4 has seven virtuals; the vendored header listed six. Members filled in from Wii Play's
 * constructor (ledger Gfx_ModelScene_ctor) and calc/draw code.
 */
class RPGrpModelScene {
public:
    /**
     * @brief Head and tail of one list of models queued for drawing
     */
    struct DrawList {
        RPGrpModel* pHead; // at 0x0
        RPGrpModel* pTail; // at 0x4 (models link through RPGrpModel +0x14)
    };

private:
    u8 mDrawListNum; // at 0x0 (1)
    u8 mLightNum;    // at 0x1 [M] (constructor argument, passed to the light manager)
    u8 unk2;         // at 0x2 [L] (cleared by calc/ResetViews)
    u8 mFlags;       // at 0x3 [M] (0x1C after construction; bit 2 selects the ScnRootEx draw mode)
    //! Opaque draw priorities, three bytes from the scene renderer's path table [M]
    u8* mpOpaPriorities; // at 0x4
    //! Translucent draw priorities [M]
    u8* mpXluPriorities;                     // at 0x8
    EGG::LightManager* mpLightManager;       // at 0xC (created when not passed in)
    EGG::FogManager* mpFogManager;           // at 0x10 (created when not passed in)
    RPGrpDrawPathManager* mpDrawPathManager; // at 0x14
    //! Projection objects by camera index (ledger setLightHook) [M]
    RPGrpProjectionObject* mpProjections[32]; // at 0x18
    nw4r::g3d::ScnRoot* mpScnRoot;            // at 0x98
    EGG::ScnRenderer* mpScnRenderer;          // at 0x9C
    DrawList* mpDrawLists;                    // at 0xA0

public:
    RPGrpModelScene(u32 pathFlags, u8 lightNum, EGG::LightManager* pLightManager,
                    EGG::FogManager* pFogManager);

    virtual ~RPGrpModelScene();                      // at 0x8
    /**
     * @brief UpdateFrame + CalcWorld of the scene renderer (Wii Play)
     */
    virtual void CalcView();                         // at 0xC
    /**
     * @brief Queues the draw lists, calculates projections, pushes the draw paths (Wii Play)
     */
    virtual void CalcDrawList();                     // at 0x10
    virtual void Draw();                             // at 0x14
    virtual void DrawFinish();                       // at 0x18
    /**
     * @brief Loads the scene's lights and fog into GX (Wii Play)
     */
    virtual void PreConditionForRenderingGX() const; // at 0x1C
    /**
     * @brief Sets a model's opaque/translucent draw priority from the priority tables
     * @note Wii Play: not in the vendored header; name is a project convention [M]
     */
    virtual void SetModelDrawPriority(RPGrpModel* pModel); // at 0x20

    void DrawPrepare(const RPGrpCamera* pCamera, const RPGrpScreen* pScreen);

    // Wii Play: used by RPGrpDrawPathManager's constructor
    EGG::ScnRenderer* GetScnRenderer() const {
        return mpScnRenderer;
    }
    // Wii Play: RPPnpGame::Load
    // Wii Play: RPFshScene::Configure
    EGG::LightManager* GetLightManager() const {
        return mpLightManager;
    }
    EGG::FogManager* GetFogManager() const {
        return mpFogManager;
    }

    RPGrpDrawPathManager* GetDrawPathManager() const {
        return mpDrawPathManager;
    }

    void AppendDrawModel(RPGrpModel* pModel); // [H] ledger appendDrawEntry
    void SetProjection(RPGrpProjectionObject* pProjection); // [M] ledger setLightHook
    void ResetViews();                        // [L]
    void ClearDrawLists();                    // [M] ledger clearEntries

public:
    static EGG::Allocator* spAllocator;
    static const u32 sOpaPriorityPaths[3];
    static const u32 sXluPriorityPaths[3];
};

//! @}

#endif

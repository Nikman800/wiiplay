#ifndef RP_GRAPHICS_RENDERER_H
#define RP_GRAPHICS_RENDERER_H
#include <Pack/types_pack.h>

#include <egg/core.h>

//! @addtogroup rp_graphics
//! @{

// Forward declarations
class IRPGrpDrawObject;
class RPGrpModelManager;
class RPGrpScreen;
class RPGrpView;
class RPScnProcDrawHook;

/**
 * @brief Rendering engine
 */
class RPGrpRenderer {
public:
    //! Maximum number of renderer views
    static const int MAX_VIEW = 32;

public:
    /**
     * @brief Gets the currently active manager instance
     */
    static RPGrpRenderer* GetCurrent() {
        return spCurrent;
    }

    /**
     * @brief Gets the view currently being rendered to
     */
    static RPGrpView* GetCurrentView() {
        return spCurrentView;
    }

    RPGrpView* GetView(int idx) const {
        return mpDrawViews[idx];
    }

    /**
     * @brief Gets the screen currently being rendered to
     */
    static RPGrpScreen* GetCurrentScreen() {
        return spCurrentScreen;
    }

    /**
     * @brief Draw step in progress (Wii Play: 5 is the layout step) [H]
     */
    static s32 GetDrawStep() {
        return sDrawStep;
    }
    /**
     * @brief Number of the view being drawn [H]
     */
    static u8 GetCurrentViewNo() {
        return sCurrentViewNo;
    }

    static void Begin();
    static void End();

    static void StartDebugDraw();

    static RPGrpRenderer* Construct(EGG::Allocator* pAllocator, u16 id);

    void Rendering();
    void Destruct();
    void BecomeCurrent();

    void AppendDrawObject(IRPGrpDrawObject* pObject);
    void AppendDrawObject(IRPGrpDrawObject& rObject) {
        AppendDrawObject(&rObject);
    }

    void PreCalculate();
    void PostCalculate();
    void CalculateInPause();

private:
    //! Allocator used for model-related allocations
    static RPGrpRenderer* spCurrent;

    //! View currently being rendered to
    static RPGrpView* spCurrentView;
    //! Screen currently being rendered to
    static RPGrpScreen* spCurrentScreen;

    // Wii Play: names are a project convention
    static s32 sDrawStep;     // 8043B354 (Gfx_drawStep)
    static u8 sCurrentViewNo; // 8043C5A5 (Gfx_currentViewNo)

    u16 mID;    // at 0x0
    u16 mFlags; // at 0x2 (bit 0 set once the views are linked) [M]
    u16 unk4;   // at 0x4 (1 after Construct) [L]
    GXColor unk6; // at 0x6 ({0, 0, 0, 0x80} after Construct) [L]
    IRPGrpDrawObject* mpDrawList;     // at 0xC
    IRPGrpDrawObject* mpDrawListTail; // at 0x10
    RPGrpView* mpDrawViews[MAX_VIEW]; // at 0x14

    // Wii Play: the members below and the destructor were missing from the vendored header.
    // Size 0xA0 [H] (Construct allocates 0xA0); vtable 8034FF84 (destructor only).

    //! Head of the linked list of non-null views (ledger Gfx_Renderer_linkViews) [H]
    RPGrpView* mpViewList; // at 0x94
    //! Model manager created by Construct
    RPGrpModelManager* mpModelManager; // at 0x98

public:
    virtual ~RPGrpRenderer(); // at 0x8

    /**
     * @brief Creates a 2D view on viewNo for the screen (ledger Gfx_Renderer_createView2D) [H]
     */
    void CreateView2D(u8 viewNo, RPGrpScreen* pScreen);
    /**
     * @brief Creates a 3D view (ledger Gfx_Renderer_createView3D) [M]
     */
    void CreateView3D(u8 viewNo, RPGrpCamera* pCamera, RPGrpScreen* pScreen); // Wii Play: checked by RPHkyGame
    /**
     * @brief Links the model scenes' draw hooks and chains the non-null views [H]
     */
    void LinkViews();

    // Wii Play: not in the vendored header (ledger names in brackets)
    static void Initialize(u16 width, u16 height); // [Gfx_initEngine]
    static void SetClearColor(u8 r, u8 g, u8 b);    // [Gfx_setClearColor]
    void PreDraw();                                // [Gfx_Renderer_beginFrame]
    void PostDraw();                               // [Gfx_Renderer_endFrame]

private:
    RPGrpRenderer(EGG::Allocator* pAllocator, u16 id);
    void UserDrawObjects();

    // Wii Play: RPScnProcDrawHook::Draw's two parts (inline, RPGrpRenderer.cpp)
    friend class RPScnProcDrawHook;
    static void DrawObjectsForStep(s32 step);
    static void DrawEffects();

public:
    static RPScnProcDrawHook sScnProcDrawHook; // the draw hook every model scene calls
    static GXColor sClearColor;
    static u8 sRenderFrameCount; // frames rendered since Begin
    static RPGrpCamera* spCurrentCamera; // camera of the view being rendered
    static nw4r::math::_MTX34 sEffectDrawMtx; // view matrix the effects are drawn with
};

//! @}

#endif

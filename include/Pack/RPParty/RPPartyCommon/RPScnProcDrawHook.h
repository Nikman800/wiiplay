#ifndef RP_PARTY_COMMON_SCN_PROC_DRAW_HOOK_H
#define RP_PARTY_COMMON_SCN_PROC_DRAW_HOOK_H
#include <Pack/types_pack.h>

//! @addtogroup rp_party
//! @{

/**
 * @brief Callback that RPGrpDrawPathManager::scnProcDraw runs for every draw step
 * @details vtable 8034FF70. One global instance (8043C598, 8 bytes, constructed by
 * Gfx_ScnProcDrawHook_initGlobal with mEnabled = true); RPGrpRenderer::LinkViews stores it in
 * every model scene's draw path manager (+0x14). The draw path manager skips step 4 unless its
 * DOF path is enabled. Draw sets the effect draw step for the step, runs the renderer's draw
 * list, and on step 5 draws the RPSysEffectMgr effects (with an EFB capture when the current
 * view asks for one). Size 0x8 [M] (the global's symbol size; no allocation).
 * @note Wii Sports has the same global (lbl_804BF608), class name unknown; the name is a
 * project convention. Not in the vendored headers.
 */
class RPScnProcDrawHook {
public:
    RPScnProcDrawHook() : mEnabled(true) {}

    virtual ~RPScnProcDrawHook() {} // at 0x8
    /**
     * @brief Draws for one scene-renderer step [H]
     */
    virtual void Draw(u16 step); // at 0xC
    virtual void VF_0x10();      // at 0x10 (Wii Play: out of line, RPGrpRenderer.cpp)

    // Wii Play: RPGrpRenderer::Rendering
    void SetEnabled(bool enable) {
        mEnabled = enable;
    }

private:
    //! Effects are drawn on step 5 only while set [M]
    bool mEnabled; // at 0x4

public:
    static const s32 sDrawSteps[6];
};

//! @}

#endif

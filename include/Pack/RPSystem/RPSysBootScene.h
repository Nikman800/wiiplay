#ifndef RP_SYSTEM_BOOT_SCENE_H
#define RP_SYSTEM_BOOT_SCENE_H
#include <Pack/types_pack.h>

#include <Pack/RPSystem/RPSysScene.h>

//! @addtogroup rp_system
//! @{

// Forward declarations
class RPSysLayout;
class RPSysLytAnmObj;
class RPSysProjectLocal;

/**
 * @brief Pack Project bootup scene
 * @details Wii Play: scene 0, the wrist strap screen (ledger Health_StrapScene). vtable
 * 80357948; size 0x58, verified (RPSysSceneCreator::create allocates 0x58). Configure mounts
 * strap.arc from the embedded archive (802E9E60) and builds the strap_00_<region>.brlyt layout;
 * the scene leaves after any button or 1200 frames. Members and virtuals filled in from Wii Play
 * (the vendored header only had the size).
 */
class RPSysBootScene : public RPSysScene {
public:
    /**
     * @brief Sequence state (mState)
     */
    enum EState {
        EState_WaitDisplay, //!< Until the scene is displayed
        EState_FadeIn,      //!< 60 frames
        EState_WaitInput,   //!< Until a button press or timeout
        EState_Exit,        //!< Scene change requested
    };

public:
    virtual ~RPSysBootScene(); // at 0x8

    /**
     * @brief Creates the 2D view and screen, mounts strap.arc and the region's layout [H]
     */
    virtual void Configure(); // at 0x44
    /**
     * @brief Attaches rp_Health_sound.brsar when the project is the Health Pack [H]
     */
    virtual void LoadResource(); // at 0x48
    virtual void Reset();        // at 0x4C
    virtual void Calculate();    // at 0x50
    virtual void Exit();         // at 0x54
    /**
     * @brief Loads the system archives and resources (fonts, windows, kokeshi, effects,
     * pause menu, HOME menu, save banner) asynchronously [H]
     */
    virtual void taskAsync(); // at 0x58
    /**
     * @brief Draws the layout (or a fallback message for unsupported languages) [H]
     */
    virtual void UserDraw(); // at 0x5C

    /**
     * @brief State 2: waits for the async task and a button press (ledger
     * Health_StrapScene_calcWaitInput) [H]
     */
    void CalcWaitInput();

private:
    RPSysProjectLocal* mpProjectLocal; // at 0x44
    RPSysLayout* mpLayout;             // at 0x48
    RPSysLytAnmObj* mpAnmObj;          // at 0x4C
    s32 mState;                        // at 0x50
    s32 mTimer;                        // at 0x54
};

//! @}

#endif

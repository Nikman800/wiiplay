#ifndef RP_PARTY_FSH_PROMPT_H
#define RP_PARTY_FSH_PROMPT_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPFshScene/RPFshObject.h>

#include <nw4r/lyt.h>

//! @addtogroup rp_fsh
//! @{

// Forward declarations
class RPSysLayout;
class RPSysLytAnmObj;
class RPSysLytResAccessor;

/**
 * @brief Per-player "yank!" prompt (fishingYank_00.brlyt; singleton Fsh_yankPrompt)
 * @details vtable 80358FF8. Size 0x28, verified (Fsh_YankPrompt_new). Show/Hide toggle the
 * player's N_L_00 / N_R_00 pane and its waitAll_00 animation.
 */
class RPFshYankPrompt : public RPFshDrawObj {
public:
    enum {
        PLAYER_MAX = 2
    };

public:
    RPFshYankPrompt();
    virtual ~RPFshYankPrompt(); // at 0x8

    virtual void UserDraw(); // at 0xC

    virtual void Create();    // at 0x14
    virtual void Reset();     // at 0x18
    virtual void Calculate(); // at 0x1C
    virtual void Exit() {}    // at 0x20

    void Show(s32 player);
    void Hide(s32 player);

    static void CreateInstance();
    static void DestroyInstance();
    static RPFshYankPrompt* GetInstance() {
        return sInstance;
    }

private:
    s32 mState[PLAYER_MAX];                  // at 0x8 0 hidden, 1 shown [M]
    RPSysLayout* mpLayout;                   // at 0x10
    RPSysLytAnmObj* mpAnmObjs[PLAYER_MAX];   // at 0x14 waitAll_00 bound per pane
    nw4r::lyt::Pane* mpPanes[PLAYER_MAX];    // at 0x1C N_L_00 / N_R_00
    RPSysLytResAccessor* mpResAccessor;      // at 0x24

    static RPFshYankPrompt* sInstance; // 8043C790
};

/**
 * @brief Per-player aiming arrow drawn at the rod grip (fishingArrow_00.brlyt; singleton
 * Fsh_aimArrow)
 * @details vtable 80359020. Size 0x34 [H] (Fsh_AimArrow_new allocates 0x34). One layout per
 * player.
 */
class RPFshAimArrow : public RPFshDrawObj {
public:
    enum {
        PLAYER_MAX = 2
    };

public:
    RPFshAimArrow();
    virtual ~RPFshAimArrow(); // at 0x8

    virtual void UserDraw(); // at 0xC

    virtual void Create();    // at 0x14
    virtual void Reset();     // at 0x18
    virtual void Calculate(); // at 0x1C
    virtual void Exit() {}    // at 0x20

    static void CreateInstance();
    static void DestroyInstance();
    static RPFshAimArrow* GetInstance() {
        return sInstance;
    }

private:
    RPSysLayout* mpLayouts[PLAYER_MAX];          // at 0x8
    /**
     * @brief Per player: arrowUpDown_00, arrowBlue_00, arrowRed_00
     */
    RPSysLytAnmObj* mpAnmObjs[PLAYER_MAX][3];    // at 0x10
    nw4r::lyt::Pane* mpRootPanes[PLAYER_MAX];    // at 0x28 RootPane
    RPSysLytResAccessor* mpResAccessor;          // at 0x30

    static RPFshAimArrow* sInstance; // 8043C798
};

//! @}

#endif

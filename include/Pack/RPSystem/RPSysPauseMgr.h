#ifndef RP_SYSTEM_PAUSE_MGR_H
#define RP_SYSTEM_PAUSE_MGR_H
#include <Pack/types_pack.h>

#include <Pack/RPKernel.h>
#include <Pack/RPSingleton.h>

//! @addtogroup rp_system
//! @{

/**
 * @brief Pause menu manager
 * @details Wii Play: size 0x1F4, verified (CreateInstance allocates 0x1F4).
 */
class RPSysPauseMgr {
    RP_SINGLETON_DECL_EX(RPSysPauseMgr);

public:
    // Wii Play: [M] names after the Sys_Pause ledger
    void loadMenu();
    void init();
    void calc();
    void draw();
    bool isPause() const {
        return mPause & 1;
    }

private:
    void checkStart();

private:
    //! Wii Play: cleared as a block by init [L]
    struct Work {
        f32 mValues[60];
    };

    BOOL mPause;                // at 0x8
    u32 mUNK_0x0C;              // at 0xC
    Work mWork[2];              // at 0x10
    RPSysPauseMenu* mPauseMenu; // at 0x1F0
};

//! @}

#endif

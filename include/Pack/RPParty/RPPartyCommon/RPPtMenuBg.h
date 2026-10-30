#ifndef RP_PARTY_COMMON_PT_MENU_BG_H
#define RP_PARTY_COMMON_PT_MENU_BG_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPActorBase.h>

//! @addtogroup rp_party
//! @{

// Forward declarations
namespace EGG {
class Archive;
}
class RPGrpCamera;
class RPPtMenuBg;

/**
 * @brief Party (game select) menu background: PT_menu_BG_model plus optionally the TV model
 * showing the selected game
 * @details vtable 80357DE8 (no new virtuals). Size 0x7C, verified (Menu_createPtMenuBg;
 * singleton 8043C6A8). The BG animation uses PT_menu_BG_4_3 or PT_menu_BG_16_9 by aspect ratio.
 * The TV model's frame is 40 * game index; its pattern frame is the first locked game - 1.
 * @note Wii Play-only framework class (ledger Menu_PtMenuBg); the name is a project convention.
 */
class RPPtMenuBg : public RPActorBase {
public:
    RPPtMenuBg(u8 viewNo, u8 sceneNo, bool withTv);
    virtual ~RPPtMenuBg(); // at 0x8

    static void CreateInstance(u8 viewNo, u8 sceneNo, bool withTv);
    static void DestroyInstance();

    void Load(EGG::Archive* pArchive);
    void Reset(s32 mode);
    void Calculate();
    void SetMode(s32 mode);

    static RPPtMenuBg* sInstance; // singleton 8043C6A8

private:
    void CalcTvModel();

private:
    u8 mViewNo;               // at 0x60
    u8 mSceneNo;              // at 0x61
    bool mWithTv;             // at 0x62
    s32 mMode;                // at 0x64 selected game slot, 9 = none
    f32 mTvFrame;             // at 0x68
    f32 unk6C;                // at 0x6C
    RPGrpModel* mpBgModel;    // at 0x70
    RPGrpModel* mpTvModel;    // at 0x74
    RPGrpCamera* mpCamera;    // at 0x78

public:
    static f32 sTvRateIdle;
    static f32 sTvRateMove;
};

//! @}

#endif

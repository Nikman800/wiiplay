#ifndef RP_PARTY_COMMON_PLAYER_NAME_TAG_H
#define RP_PARTY_COMMON_PLAYER_NAME_TAG_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPLytObjBase.h>
#include <Pack/RPParty/RPPartyCommon/RPStateMachine.h>

#include <nw4r/lyt.h>

//! @addtogroup rp_party
//! @{

// Forward declarations
class RPSysLayout;
class RPSysLytAnmObj;

/**
 * @brief Per-player lamp and Mii name tag (playerSelect.arc, playerSelect_03.brlyt)
 * @details vtable 80357FB8. Size 0x34, verified (Menu_PlayerConfirmLyt_ctor, four of them).
 * Two states (hidden, shown); the frame and line are tinted with the player colour.
 * @note Wii Play-only framework class (ledger Menu_PlayerNameTag); the name is a project
 * convention.
 */
class RPPlayerNameTag : public RPLytObjBase {
public:
    explicit RPPlayerNameTag(s32 player);

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14

    void Show(bool show);

private:
    void ShownEnter();
    void ShownCalc();

private:
    RPStateMachine<RPPlayerNameTag>* mpStateMachine; // at 0x8
public:
    f32 mNamePosX;                                   // at 0xC name pane position (RPPlayerConfirmLyt
    f32 mNamePosY;                                   // at 0x10 sets it from the player pane)
private:
    s32 mPlayer;                                     // at 0x14
    RPSysLayout* mpLayout;                           // at 0x18
    RPSysLytAnmObj* mpLampAnmObjs[4];                // at 0x1C onOffLamp per lamp
    nw4r::lyt::Pane* mpNamePane;                     // at 0x2C N_name_00
public:
    nw4r::lyt::Pane* unk30;                          // at 0x30
};

//! @}

#endif

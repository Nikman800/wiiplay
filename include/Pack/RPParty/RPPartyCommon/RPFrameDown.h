#ifndef RP_PARTY_COMMON_FRAME_DOWN_H
#define RP_PARTY_COMMON_FRAME_DOWN_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPLytObjBase.h>

#include <nw4r/math.h>

//! @addtogroup rp_party
//! @{

// Forward declarations
class RPSysLayout;
class RPSysLytAnmObj;
class RPSysMessage;

/**
 * @brief Bottom frame with a caption and a back button (backCommon.arc, backCommon_00.brlyt,
 * pane N_frameDown_00, caption T_caption_01)
 * @details vtable 80357718. Size 0x30, verified (Menu_SportsMessageScene_configure).
 * States 0 hidden, then in / idle / hover / unhover / press / out driven by mRequest.
 * The caption is a common_message.bmg entry chosen by mType.
 * @note Wii Play-only framework class (ledger Lyt_FrameDown); the name is a project convention.
 */
class RPFrameDown : public RPLytObjBase {
public:
    explicit RPFrameDown(s32 type);

    virtual void Load(EGG::Archive* pArchive); // at 0x8
    virtual void Reset();                    // at 0xC
    virtual void Calculate();                // at 0x10
    virtual void Draw();                     // at 0x14
    virtual void VF_0x18();                  // at 0x18
    virtual ~RPFrameDown();                  // at 0x1C

    void RequestIn();
    void RequestHover(s32 controller);
    void RequestUnhover();
    void RequestPressed();
    void PlayHover();
    void PlayUnhover();
    void PlayPressed();
    bool IsIdle() const;
    bool IsIdleOrHovered() const;
    bool HitTestBackButton(const nw4r::math::VEC2& rPos) const;

private:
    s32 mType;                    // at 0x8 caption message index (table lbl_8043ED38)
    s32 mState;                   // at 0xC
    s32 mRequest;                 // at 0x10 requested state change
    RPSysLayout* mpLayout;        // at 0x14
    RPSysLytAnmObj* mpAnmObjs[4]; // at 0x18 in / hover / press / out [M]
    s32 unk28;                    // at 0x28 [L] hover request flag
    RPSysMessage* mpMessage;      // at 0x2C common_message.bmg

public:
    static const char* sBackButtonNames[2];
    static const char* sArcName;
    static const char* sLayoutName;
    static const char* sAnimNames[4];
    static const char* sCaptionName;
};

//! @}

#endif

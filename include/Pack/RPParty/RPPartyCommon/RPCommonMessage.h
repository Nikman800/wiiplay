#ifndef RP_PARTY_COMMON_COMMON_MESSAGE_H
#define RP_PARTY_COMMON_COMMON_MESSAGE_H
#include <Pack/types_pack.h>

#include <Pack/RPUtility/RPUtlLoadSaveDataScene.h>

//! @addtogroup rp_party
//! @{

// Forward declarations
class RPButtonSeqHolder;
class RPTitleLyt;

/**
 * @brief Title scene: save-data check, then the title layout waiting for input
 * @details vtable 80358040. Size 0x68, verified (allocated by RPSysSceneCreator with the inlined
 * constructor). States 17/18 fade in, 19 waits for input (calcWaitInput); CalcState4 changes to
 * scene 0x1A. Configure creates a 2D screen view, the RPTitleLyt and the button-sequence
 * (secret code) holder.
 * @note Wii Play-only class (ledgers Lyt_CommonMessage for Configure/destructor and
 * Menu_CommonMessage for the rest); the name is a project convention.
 */
class RPCommonMessage : public RPUtlLoadSaveDataScene {
public:
    virtual ~RPCommonMessage(); // at 0x8

    virtual void Configure();    // at 0x44
    virtual void LoadResource(); // at 0x48 common_message.bmg
    virtual void Reset();        // at 0x4C
    virtual void Calculate();    // at 0x50
    virtual void Exit();         // at 0x54
    virtual void UserDraw();     // at 0x5C

    virtual void CalcState4(); // at 0xB4 changes to scene 0x1A after the fade

private:
    void CalcWaitInput();
    /**
     * @brief Sets the fade color (white when started first, else black), state 17 and opens the
     * title layout (inlined into Configure and Reset)
     */
    void StartTitle();

private:
    s32 mTimer;                     // at 0x54 18000 or 0 when input waiting starts
    u8 unk58[4];                    // at 0x58 cleared together
    RPTitleLyt* mpTitleLyt;         // at 0x5C
    RPButtonSeqHolder* mpButtonSeq; // at 0x60
    RPSysMessage* mpMessage;        // at 0x64
};

//! @}

#endif

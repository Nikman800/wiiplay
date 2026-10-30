#ifndef RP_PARTY_COMMON_MSG_BOARD_H
#define RP_PARTY_COMMON_MSG_BOARD_H
#include <Pack/types_pack.h>

//! @addtogroup rp_party
//! @{

// Forward declarations
class RPPartySystemData;
class RPSysKokeshi;
class RPSysMessage;
class RPSysNWC24Manager;

/**
 * @brief Posts medal and unlock letters to the Wii Message Board (singleton sInstance)
 * @details vtable 80358700 (destructor only). Size 0xC (Party_MsgBoard_create allocates 0xC and
 * constructs it inline). Letters are formatted from party_message.bmg and committed through
 * RPSysNWC24Manager; at most 15 per day (RPPartySystemData mRegistTodayCount /
 * mRegistLastDate). Guest Miis (avatar + 0xC != 0) never post.
 * @note Wii Play-only framework class (ledger Party_MsgBoard); the name is a project
 * convention.
 */
class RPMsgBoard {
public:
    static const int DAILY_POST_MAX = 15;

public:
    static void CreateInstance();
    static void DestroyInstance();
    static RPMsgBoard* GetInstance() {
        return sInstance;
    }

    virtual ~RPMsgBoard(); // at 0x8

    void LoadMessages();
    bool CanPost(RPSysKokeshi* pAvatar, const RPPartySystemData& rData) const;
    void RecordPost(RPSysKokeshi* pAvatar, RPPartySystemData& rData);
    void PostMedalMessage(RPSysKokeshi* pAvatar, s32 game, s32 medal);
    void PostAllMedalsMessage(RPSysKokeshi* pAvatar, s32 medal);
    void PostUnlockMessage(RPSysKokeshi* pAvatar);

private:
    RPMsgBoard();

private:
    RPSysNWC24Manager* mpNwc24Manager; // at 0x4
    RPSysMessage* mpMessage;           // at 0x8 party_message.bmg

    static RPMsgBoard* sInstance; // 8043C720
};

//! @}

#endif

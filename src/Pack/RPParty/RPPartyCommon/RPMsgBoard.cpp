#include <Pack/RPParty/RPPartyCommon/RPMsgBoard.h>

#include <Pack/RPKernel.h>
#include <Pack/RPSystem/RPSysResourceManager.h>

RPMsgBoard* RPMsgBoard::sInstance;

// Guest Miis never post: only avatars from the official database do.
inline bool IsOfficialAvatar(RPSysKokeshi* pAvatar) {
    return pAvatar->GetGenInfo().GetLocation().GetDataSource() == RFLDataSource_Official;
}

inline RPMsgBoard::RPMsgBoard() {
    mpNwc24Manager = new RPSysNWC24Manager(NULL);
    mpMessage = NULL;
}

void RPMsgBoard::CreateInstance() {
    sInstance = new RPMsgBoard;
}

void RPMsgBoard::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPMsgBoard::~RPMsgBoard() {}

void RPMsgBoard::LoadMessages() {
    void* pResource = RPSysResourceManager::GetMessageResource("party_message.bmg");
    mpMessage = new RPSysMessage(pResource, NULL);
}

bool RPMsgBoard::CanPost(RPSysKokeshi* pAvatar, const RPPartySystemData& rData) const {
    if (!IsOfficialAvatar(pAvatar)) {
        return false;
    }

    RPTime16 date = RPSysSystem::instance()->getCalendarTime16();
    if (date == rData.getRegistLastDate()) {
        return rData.getRegistTodayCount() < DAILY_POST_MAX;
    }
    return true;
}

void RPMsgBoard::RecordPost(RPSysKokeshi* pAvatar, RPPartySystemData& rData) {
    if (!IsOfficialAvatar(pAvatar)) {
        return;
    }

    RPTime16 date = RPSysSystem::instance()->getCalendarTime16();
    if (date == rData.getRegistLastDate()) {
        rData.setRegistTodayCount(rData.getRegistTodayCount() + 1);
    } else {
        rData.setRegistLastDate(date);
        rData.setRegistTodayCount(1);
    }
}

void RPMsgBoard::PostUnlockMessage(RPSysKokeshi* pAvatar) {
    if (!IsOfficialAvatar(pAvatar)) {
        return;
    }

    RPTime16 date = RPSysSystem::instance()->getCalendarTime16();
    mpNwc24Manager->commitMail(mpMessage->GetMsg(10000, 0), mpMessage->GetMsg(11000, 0), date,
                               pAvatar, 1, pAvatar->mAdditionalInfo.name);
}

void RPMsgBoard::PostMedalMessage(RPSysKokeshi* pAvatar, s32 game, s32 medal) {
    if (!IsOfficialAvatar(pAvatar)) {
        return;
    }

    RPTime16 date = RPSysSystem::instance()->getCalendarTime16();
    s32 gameIds[] = {0, 1, 2, 3, 4, 5, 7, 8, 9};
    s32 medalIdx = medal - 1;
    mpNwc24Manager->commitMail(mpMessage->GetMsg(medalIdx + 10002, 0),
                               mpMessage->GetMsg(gameIds[game] + 11100, medalIdx), date, pAvatar,
                               1, pAvatar->mAdditionalInfo.name);
}

void RPMsgBoard::PostAllMedalsMessage(RPSysKokeshi* pAvatar, s32 medal) {
    if (!IsOfficialAvatar(pAvatar)) {
        return;
    }

    RPTime16 date = RPSysSystem::instance()->getCalendarTime16();
    mpNwc24Manager->commitMail(mpMessage->GetMsg(medal + 10005, 0),
                               mpMessage->GetMsg(medal + 11037, 0), date, pAvatar, 1,
                               pAvatar->mAdditionalInfo.name);
}

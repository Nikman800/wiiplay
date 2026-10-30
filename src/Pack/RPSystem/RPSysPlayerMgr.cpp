#include <Pack/RPSystem.h>
// Ported from doldecomp/ogws (CC0) src/Pack/RPSystem/RPSysPlayerMgr.cpp @ f5a1bb5e; Wii Play changes marked "Wii Play:".

#include <RVLFaceLib.h>

#include <egg/core.h>

// The file compiles in reverse definition order (-inline deferred): the destructor and constructor
// (defined last) come first at 0x8018BE70, followed by the weak RPSysPlayerOldData constructor and
// RPSysPlayer destructor that `new T[n]` in the constructor emits. Wii Play: DestroyInstance is not
// in the binary.
RP_SINGLETON_IMPL_EX_NO_DESTROY(RPSysPlayerMgr);

static inline void ResetAllOldData(RPSysPlayerOldData* const& data) {
    for (int i = 0; i < RPSysPlayerMgr::OLD_DATA_LEN; i++) {
        data[i].dataSource = -1;
        data[i].index = -1;
        for (int j = 0; j < RFL_CREATEID_LEN; j++) {
            data[i].createID.data[j] = 0;
        }
        data[i].address.reset();
    }
}
void RPSysPlayerMgr::resetOldData() {
    ResetAllOldData(mpOldData);
}

const RPSysPlayer& RPSysPlayerMgr::getPlayer(s32 idx) const {
    return mpPlayers[idx];
}

u8 RPSysPlayerMgr::getChannel(s32 idx) const {
    return mpPlayers[idx].getChannel();
}

u32 RPSysPlayerMgr::getSpeakerFlag(s32 player) const {
    s32 chan;

    // Wii Play: a negative player selects the active controller's player
    if (player < 0) {
        chan = RPSysCoreController::getActiveController()->getPlayerNo();
    } else {
        chan = getChannel(player);
    }

    if (chan == WPAD_CHAN0) {
        return 1 << WPAD_CHAN0;
    }

    if (chan == WPAD_CHAN1) {
        return 1 << WPAD_CHAN1;
    }

    if (chan == WPAD_CHAN2) {
        return 1 << WPAD_CHAN2;
    }

    if (chan == WPAD_CHAN3) {
        return 1 << WPAD_CHAN3;
    }

    return 1 << WPAD_CHAN0 | //
           1 << WPAD_CHAN1 | //
           1 << WPAD_CHAN2 | //
           1 << WPAD_CHAN3;
}

void RPSysPlayerMgr::saveAllData() {
    for (int i = 0; i < mPlayerNum; i++) {
        RPSysPlayerArg arg;
        arg.pDefaultSportsPlayerData = mpDefaultSportsPlayerData;
        arg.ppCtrlSportsPlayerData = mppCtrlSportsPlayerData;
        arg.pDefaultPartyPlayerData = mpDefaultPartyPlayerData;
        arg.ppCtrlPartyPlayerData = mppCtrlPartyPlayerData;

        mpPlayers[i].saveData(arg);
    }
}

// Wii Play: A+B tracking is disabled
bool RPSysPlayerMgr::isABPress(s32 /* player */) const {
    return false;
}

// Wii Play: A+B tracking is disabled
void RPSysPlayerMgr::setABPress(s32 /* player */) {}

void RPSysPlayerMgr::reset() {
    mPlayerNum = 0;
    mRemoteNum = 0;

    for (int i = 0; i < RP_MAX_PLAYERS; i++) {
        mpPlayers[i].reset();
    }
}

void RPSysPlayerMgr::setPlayerNum(s32 num) {
    mPlayerNum = num;
}

void RPSysPlayerMgr::setRemoteNum(s32 num) {
    mRemoteNum = num < mPlayerNum ? num : mPlayerNum;

    s32 chan = WPAD_CHAN0;
    for (int i = 0; i < mPlayerNum; i++) {
        mpPlayers[i].setChannel(chan);

        if (++chan > num - 1) {
            chan = WPAD_CHAN0;
        }
    }
}

static inline bool IsOldAddressEqual(const RPSysCoreAddress& address, s32 player) {
    return RP_GET_CORE_CTRL(player)->getAddress().isEqual(address);
}

bool RPSysPlayerMgr::isValidOldData() {
    // Can't use old data when people are sharing remotes
    if (mPlayerNum == mRemoteNum) {
        s32 rel2Abs[RP_MAX_PLAYERS] = {
            0,     // 1 player
            0 + 1, // 2 players
#if defined(PACK_SPORTS)
            0 + 1 + 2,     // 3 players
            0 + 1 + 2 + 3, // 4 players
#endif
        };

        s32 relIdx = mPlayerNum;
        s32 absIdx = rel2Abs[--relIdx];

        for (s32 i = absIdx; i < absIdx + mPlayerNum; i++) {
            if (mpOldData[i].dataSource != RFLDataSource_Official) {
                continue;
            }

            // Mii database has changed since last play
            if (RPSysKokeshiManager::GetOfficialDBIndexFromCreateID(
                    &mpOldData[i].createID) != mpOldData[i].index) {
                return false;
            }
        }

        s32 player = 0;
        s32 found = 0;

        for (s32 i = absIdx; i < absIdx + mPlayerNum; i++, player++) {
            for (s32 j = absIdx; j < absIdx + mPlayerNum; j++) {
                if (!mpOldData[j].address.isValid()) {
                    continue;
                }

                if (IsOldAddressEqual(mpOldData[j].address, player)) {

                    found++;

                    // Wii Play: the two entries are swapped
                    RPSysPlayerOldData temp = mpOldData[i];
                    mpOldData[i] = mpOldData[j];
                    mpOldData[j] = temp;
                    break;
                }
            }
        }

        return mPlayerNum == found;
    }

    return false;
}

void RPSysPlayerMgr::getOldData(s8* pDataSrc, u16* pIndex, s32 idx) {
    s32 rel2Abs[RP_MAX_PLAYERS] = {
        0,     // 1 player
        0 + 1, // 2 players
#if defined(PACK_SPORTS)
        0 + 1 + 2,     // 3 players
        0 + 1 + 2 + 3, // 4 players
#endif
    };

    s32 relIdx = mPlayerNum;
    s32 absIdx = rel2Abs[--relIdx];

    *pDataSrc = mpOldData[idx + absIdx].dataSource;
    *pIndex = mpOldData[idx + absIdx].index;
}

bool RPSysPlayerMgr::isCtrlOldData(s32 idx) {
    s32 rel2Abs[RP_MAX_PLAYERS] = {
        0,     // 1 player
        0 + 1, // 2 players
#if defined(PACK_SPORTS)
        0 + 1 + 2,     // 3 players
        0 + 1 + 2 + 3, // 4 players
#endif
    };

    s32 relIdx = mPlayerNum;
    s32 absIdx = rel2Abs[--relIdx];

    if (mpOldData[idx + absIdx].dataSource >= RFLDataSource_Controller1 &&
        mpOldData[idx + absIdx].dataSource <= RFLDataSource_Controller4 &&
        mpOldData[idx + absIdx].index < RFL_CTRL_CHAR_MAX) {

        return true;
    }

    return false;
}

void RPSysPlayerMgr::calcOldData() {
    // Can't use old data when people are sharing remotes
    if (mPlayerNum == mRemoteNum) {
        s32 rel2Abs[RP_MAX_PLAYERS] = {
            0,     // 1 player
            0 + 1, // 2 players
#if defined(PACK_SPORTS)
            0 + 1 + 2,     // 3 players
            0 + 1 + 2 + 3, // 4 players
#endif
        };

        s32 relIdx = mPlayerNum;
        s32 absIdx = rel2Abs[--relIdx];

        // Wii Play: a second counter for the old data index
        for (int i = 0, j = absIdx; i < mPlayerNum; i++, j++) {
            mpOldData[j].dataSource = mpPlayers[i].getDataSource();
            mpOldData[j].index = mpPlayers[i].getIndex();
            mpOldData[j].createID = mpPlayers[i].getCreateID();
            mpOldData[j].address = RP_GET_CORE_CTRL(i)->getAddress();
        }
    }
}

void RPSysPlayerMgr::loadOldData() {
    s32 rel2Abs[RP_MAX_PLAYERS] = {
        0,     // 1 player
        0 + 1, // 2 players
#if defined(PACK_SPORTS)
        0 + 1 + 2,     // 3 players
        0 + 1 + 2 + 3, // 4 players
#endif
    };

    for (int i = 0; i < RP_MAX_PLAYERS; i++) {
        // Already in use
        // Wii Play: rel2Abs[i] is read again at each use
        if (mpOldData[rel2Abs[i]].dataSource != -1 ||
            mpOldData[rel2Abs[i]].index != 0xFFFF) {
            continue;
        }

        for (int j = 0; j < i + 1; j++) {
            s32 currIdx = j + rel2Abs[i];

#if defined(PACK_SPORTS)
            RPSportsSystemData systemData =
                RP_GET_INSTANCE(RPSysSaveDataMgr)->getSportsSystemData();
#elif defined(PACK_PARTY)
            RPPartySystemData systemData =
                RP_GET_INSTANCE(RPSysSaveDataMgr)->getPartySystemData();
#endif

            s8 index;
            u8 addr[WPAD_ADDR_LEN];
            systemData.getOldData(&index, addr, i + 1, j);

            if (index >= 0) {
                mpOldData[currIdx].dataSource = RFLDataSource_Official;
                mpOldData[currIdx].index = index;

#if defined(PACK_SPORTS)
                RP_GET_INSTANCE(RPSysSaveDataMgr)
                    ->getSportsPlayerData(index)
                    .getCreateID(&mpOldData[currIdx].createID);
#elif defined(PACK_PARTY)
                RP_GET_INSTANCE(RPSysSaveDataMgr)
                    ->getPartyPlayerData(index)
                    .getCreateID(&mpOldData[currIdx].createID);
#endif

                mpOldData[currIdx].address.set(addr);
            }
        }
    }
}

void RPSysPlayerMgr::saveOldData() {
    s32 rel2Abs[RP_MAX_PLAYERS] = {
        0,     // 1 player
        0 + 1, // 2 players
#if defined(PACK_SPORTS)
        0 + 1 + 2,     // 3 players
        0 + 1 + 2 + 3, // 4 players
#endif
    };

    bool valid[RP_MAX_PLAYERS] = {
        true, // 1 player
        true, // 2 players
#if defined(PACK_SPORTS)
        true, // 3 players
        true, // 4 players
#endif
    };

    // Wii Play: every entry of a player count is checked
    for (int i = 0; i < RP_MAX_PLAYERS; i++) {
        for (int j = 0; j < i + 1; j++) {
            s32 currIdx = j + rel2Abs[i];

            if (mpOldData[currIdx].dataSource != RFLDataSource_Official ||
                mpOldData[currIdx].index >= RFL_DB_CHAR_MAX ||
                !mpOldData[currIdx].address.isValid()) {

                valid[i] = false;
            }
        }
    }

    for (int i = 0; i < RP_MAX_PLAYERS; i++) {
        // Wii Play: only valid player counts are saved
        if (!valid[i]) {
            continue;
        }

        for (int j = 0; j < i + 1; j++) {
            s32 currIdx = j + rel2Abs[i];

            u8 addr[WPAD_ADDR_LEN];
            for (int k = 0; k < WPAD_ADDR_LEN; k++) {
                addr[k] = mpOldData[currIdx].address.get(k);
            }

#if defined(PACK_SPORTS)
            RPSportsSystemData systemData =
                RP_GET_INSTANCE(RPSysSaveDataMgr)->getSportsSystemData();
#elif defined(PACK_PARTY)
            RPPartySystemData systemData =
                RP_GET_INSTANCE(RPSysSaveDataMgr)->getPartySystemData();
#endif

            systemData.setOldData(mpOldData[currIdx].index, addr, i + 1, j);

#if defined(PACK_SPORTS)
            RP_GET_INSTANCE(RPSysSaveDataMgr)->setSportsSystemData(systemData);
#elif defined(PACK_PARTY)
            RP_GET_INSTANCE(RPSysSaveDataMgr)->setPartySystemData(systemData);
#endif
        }
    }
}

void RPSysPlayerMgr::loadData(s32 idx) {
    RPSysPlayerArg arg;
    arg.pDefaultSportsPlayerData = mpDefaultSportsPlayerData;
    arg.ppCtrlSportsPlayerData = mppCtrlSportsPlayerData;
    arg.pDefaultPartyPlayerData = mpDefaultPartyPlayerData;
    arg.ppCtrlPartyPlayerData = mppCtrlPartyPlayerData;

    mpPlayers[idx].loadData(arg);
}

void RPSysPlayerMgr::loadAllData() {
    RPSysPlayerArg arg;
    arg.pDefaultSportsPlayerData = mpDefaultSportsPlayerData;
    arg.ppCtrlSportsPlayerData = mppCtrlSportsPlayerData;
    arg.pDefaultPartyPlayerData = mpDefaultPartyPlayerData;
    arg.ppCtrlPartyPlayerData = mppCtrlPartyPlayerData;

    for (int i = 0; i < mPlayerNum; i++) {
        mpPlayers[i].loadData(arg);
    }
}

void RPSysPlayerMgr::setAvatar(s8 dataSrc, u16 index,
                               const RFLCreateID& rCreateID, s32 idx) {

    mpPlayers[idx].setAvatar(dataSrc, index, rCreateID);
}

RPSysPlayerMgr::RPSysPlayerMgr(EGG::Heap* pHeap)
    : mpParentHeap(pHeap),
      mPlayerNum(0),
      mRemoteNum(0),
      mFlags(0),
      mpPlayers(new RPSysPlayer[RP_MAX_PLAYERS]),
      mpOldData(new RPSysPlayerOldData[OLD_DATA_LEN]),
      mpDefaultSportsPlayerData(NULL),
      mppCtrlSportsPlayerData(NULL),
      mpDefaultPartyPlayerData(NULL),
      mppCtrlPartyPlayerData(NULL) {
    mpDefaultPartyPlayerData = new RPPartyPlayerData[6];
    mppCtrlPartyPlayerData = new RPPartyPlayerData*[4];

    int i;
    int offset;

    i = 0;
    offset = 0;
    do {
        mppCtrlPartyPlayerData[i] = new RPPartyPlayerData[10];
        i++;
        offset += sizeof(RPPartyPlayerData*);
    } while (i < 4);

    ResetAllOldData(mpOldData);
}

RPSysPlayerMgr::~RPSysPlayerMgr() {}

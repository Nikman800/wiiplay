#include <Pack/RPSystem.h>
// Ported from doldecomp/ogws (CC0) src/Pack/RPSystem/RPPartySystemData.cpp @ f5a1bb5e; Wii Play changes marked "Wii Play:".
//
// Wii Play: the Wii Play class has the accessors of Wii Sports' RPSportsSystemData (old data for
// one and two players, regist date and count) and game/message flag setters.

RPPartySystemData::RPPartySystemData() {
    reset();
}

void RPPartySystemData::reset() {
    for (int i = 0; i < OLD_DATA_LEN; i++) {
        mOldIndex[i] = -1;

        for (int j = 0; j < WPAD_ADDR_LEN; j++) {
            mOldAddress[i][j] = 0;
        }
    }

    mRegistTodayCount = 0;
    mRegistLastDate = 0;
    mGameFlags.makeAllZero();
    // Wii Play: the first game is open from the start
    mGameFlags.setBit(EGame_Duc);
    mMsgFlags.makeAllZero();
}

void RPPartySystemData::getOldData(s8* pIndex, u8 pAddr[WPAD_ADDR_LEN],
                                   s32 playerNum, s32 playerNo) const {
    u32 rel2Abs[2] = {
        0,     // 1 player
        0 + 1, // 2 players
    };

    u32 absIdx = rel2Abs[--playerNum] + playerNo;

    *pIndex = mOldIndex[absIdx];

    for (int i = 0; i < WPAD_ADDR_LEN; i++) {
        pAddr[i] = mOldAddress[absIdx][i];
    }
}

void RPPartySystemData::setOldData(s8 index, const u8 pAddr[WPAD_ADDR_LEN],
                                   s32 playerNum, s32 playerNo) {
    u32 rel2Abs[2] = {
        0,     // 1 player
        0 + 1, // 2 players
    };

    u32 absIdx = rel2Abs[--playerNum] + playerNo;

    mOldIndex[absIdx] = index;

    for (int i = 0; i < WPAD_ADDR_LEN; i++) {
        mOldAddress[absIdx][i] = pAddr[i];
    }
}

void RPPartySystemData::setRegistTodayCount(u8 count) {
    mRegistTodayCount = count;
}

u8 RPPartySystemData::getRegistTodayCount() const {
    return mRegistTodayCount;
}

void RPPartySystemData::setRegistLastDate(RPTime16 date) {
    mRegistLastDate = date;
}

RPTime16 RPPartySystemData::getRegistLastDate() const {
    return mRegistLastDate;
}

void RPPartySystemData::setGameOpen(s32 idx, bool open) {
    mGameFlags.changeBit(idx, open);
}

bool RPPartySystemData::isGameOpen(s32 idx) const {
    return mGameFlags.onBit(idx);
}

void RPPartySystemData::setMsgFlag(u8 msg, bool on) {
    mMsgFlags.changeBit(msg, on);
}

bool RPPartySystemData::isMsgFlag(u8 msg) const {
    return mMsgFlags.onBit(msg);
}

void RPPartySystemData::read(EGG::RamStream& rStrm) {
    for (int i = 0; i < OLD_DATA_LEN; i++) {
        mOldIndex[i] = rStrm.read_s8();

        for (int j = 0; j < WPAD_ADDR_LEN; j++) {
            mOldAddress[i][j] = rStrm.read_u8();
        }
    }

    mRegistTodayCount = rStrm.read_u8();
    mRegistLastDate = rStrm.read_u16();
    mGameFlags = rStrm.read_u32();
    mMsgFlags = rStrm.read_u32();
}

void RPPartySystemData::write(EGG::RamStream& rStrm) {
    for (int i = 0; i < OLD_DATA_LEN; i++) {
        rStrm.write_s8(mOldIndex[i]);

        for (int j = 0; j < WPAD_ADDR_LEN; j++) {
            rStrm.write_u8(mOldAddress[i][j]);
        }
    }

    rStrm.write_u8(mRegistTodayCount);
    rStrm.write_u16(mRegistLastDate);
    rStrm.write_u32(mGameFlags);
    rStrm.write_u32(mMsgFlags);
}

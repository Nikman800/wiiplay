#include <Pack/RPSystem.h>
// Ported from doldecomp/ogws (CC0) src/Pack/RPSystem/RPPartyPlayerData.cpp @ f5a1bb5e; Wii Play changes marked "Wii Play:".

/**
 * @brief Constructor
 */
RPPartyPlayerData::RPPartyPlayerData() {
    reset();
}

/**
 * @brief Resets the data to a default save state
 */
void RPPartyPlayerData::reset() {
    mPlayerFlags.makeAllZero();

    for (int i = 0; i < RFL_CREATEID_LEN; i++) {
        mCreateID.data[i] = 0;
    }

    mDebutTime = 0;

    for (int i = 0; i < EGame_Max; i++) {
        for (int j = 0; j < MY_RECORDS_LENGTH; j++) {
            mMyRecords[i][j] = 0;
        }

        mMedals[i] = EMedal_None;
    }

    mMedalDemoFlags.makeAllZero();
}

// Wii Play: not in Wii Sports
s32 RPPartyPlayerData::getMyRecord(EGame game, int idx) const {
    return mMyRecords[game][idx];
}

// Wii Play: not in Wii Sports
void RPPartyPlayerData::setMyRecord(s32 record, EGame game, int idx) {
    mMyRecords[game][idx] = record;
}

// Wii Play: not in Wii Sports
u8 RPPartyPlayerData::getMedal(EGame game) const {
    return mMedals[game];
}

// Wii Play: not in Wii Sports
void RPPartyPlayerData::setMedal(u8 medal, EGame game) {
    mMedals[game] = medal;
}

// Wii Play: not in Wii Sports
bool RPPartyPlayerData::isFirstPlay() const {
    return mPlayerFlags.offBit(EFlag_Played);
}

// Wii Play: not in Wii Sports
void RPPartyPlayerData::setFirstPlay() {
    mPlayerFlags.setBit(EFlag_Played);
}

// Wii Play: not in Wii Sports
bool RPPartyPlayerData::isMedalDemo(u8 demo) const {
    return mMedalDemoFlags.onBit(demo);
}

// Wii Play: not in Wii Sports
void RPPartyPlayerData::setMedalDemo(u8 demo) {
    mMedalDemoFlags.setBit(demo);
}

/**
 * @brief Tests whether this player has been registered with the player list
 */
bool RPPartyPlayerData::isRegistered() const {
    return mPlayerFlags.onBit(EFlag_Registered);
}

/**
 * @brief Sets whether this player has been registered with the player list
 *
 * @param registered Registration status
 */
void RPPartyPlayerData::setRegistered(bool registered) {
    mPlayerFlags.changeBit(EFlag_Registered, registered);
}

/**
 * @brief Gets this player's Mii create ID
 *
 * @param[out] pCreateID Mii create ID
 */
void RPPartyPlayerData::getCreateID(RFLCreateID* pCreateID) const {
    for (int i = 0; i < RFL_CREATEID_LEN; i++) {
        pCreateID->data[i] = mCreateID.data[i];
    }
}

/**
 * @brief Set this player's Mii create ID
 *
 * @param pCreateID Mii create ID
 */
void RPPartyPlayerData::setCreateID(const RFLCreateID* pCreateID) {
    for (int i = 0; i < RFL_CREATEID_LEN; i++) {
        mCreateID.data[i] = pCreateID->data[i];
    }
}

/**
 * @brief Gets the time this player was registered with the player list
 */
RPTime32 RPPartyPlayerData::getDebutTime() const {
    return mDebutTime;
}

/**
 * @brief Sets the time this player was registered with the player list
 *
 * @param time Debut time
 */
void RPPartyPlayerData::setDebutTime(RPTime32 time) {
    mDebutTime = time;
}

// Wii Play: read and write are defined here, after the accessors
void RPPartyPlayerData::read(EGG::RamStream& rStrm) {
    mPlayerFlags.setDirect(rStrm.read_u32());

    for (int i = 0; i < RFL_CREATEID_LEN; i++) {
        mCreateID.data[i] = rStrm.read_u8();
    }

    mDebutTime = rStrm.read_u32();

    for (int i = 0; i < EGame_Max; i++) {
        for (int j = 0; j < MY_RECORDS_LENGTH; j++) {
            mMyRecords[i][j] = rStrm.read_s32();
        }

        mMedals[i] = rStrm.read_u8();
    }

    mMedalDemoFlags.setDirect(rStrm.read_u8());
}

void RPPartyPlayerData::write(EGG::RamStream& rStrm) {
    rStrm.write_u32(mPlayerFlags.getDirect());

    for (int i = 0; i < RFL_CREATEID_LEN; i++) {
        rStrm.write_u8(mCreateID.data[i]);
    }

    rStrm.write_u32(mDebutTime);

    for (int i = 0; i < EGame_Max; i++) {
        for (int j = 0; j < MY_RECORDS_LENGTH; j++) {
            rStrm.write_s32(mMyRecords[i][j]);
        }

        rStrm.write_u8(mMedals[i]);
    }

    rStrm.write_u8(mMedalDemoFlags.getDirect());
}

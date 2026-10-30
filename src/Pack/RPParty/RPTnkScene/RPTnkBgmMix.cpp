// Tanks stage music (RPTnkBgmMix): the track mask follows the enemies left and the strongest enemy.
// Unit 0x80267F4C-0x80268488 (dtk/game_units.tsv). Functions in reverse address order.
#include <Pack/RPParty/RPTnkScene/RPTnkBgmMix.h>

#include <Pack/RPAudio.h>

// [strongest enemy type - 1][min(enemies left, 4) - 1]
static const u32 sTrackMasks[9][4] = {
    {0x411, 0x411, 0x411, 0x411}, {0x491, 0x491, 0x493, 0x493}, {0x4C3, 0x4D3, 0x4D3, 0x4D9},
    {0x419, 0x459, 0x4D9, 0x4D9}, {0x4E3, 0x4F3, 0x4F9, 0x4F9}, {0x543, 0x573, 0x5F9, 0x5F9},
    {0x547, 0x567, 0x5F7, 0x5FD}, {0x713, 0x7B9, 0x7F9, 0x7F9}, {0x7ED, 0x7ED, 0x7ED, 0x7ED},
};

inline void RPTnkBgmMix::Reset() {
    mTrackMask = 0x411;
    mStage = 0;
    mEnemyNum = 0;
    mMaxEnemyType = 0;
    mPlaying = false;
}

RPTnkBgmMix::RPTnkBgmMix() {
    Reset();
}

void RPTnkBgmMix::StartStageBgm(s32 stage, s32 enemyNum, s32 maxEnemyType) {
    if (stage < 1) {
        stage = 1;
    }

    if (enemyNum < 1) {
        enemyNum = 1;
    } else if (enemyNum > 8) {
        enemyNum = 8;
    }

    if (maxEnemyType < 1) {
        maxEnemyType = 1;
    } else if (maxEnemyType > 9) {
        maxEnemyType = 9;
    }

    mTrackMask = 0x411;
    mStage = stage;
    mEnemyNum = enemyNum;
    mMaxEnemyType = maxEnemyType;
    mPlaying = true;

    RPSndAudioMgr::instance()->stopBgm(0);
    RPSndAudioMgr::instance()->startBgm(0x3B);
    ApplyTrackMask();
}

void RPTnkBgmMix::SetParams(s32 enemyNum, s32 maxEnemyType) {
    if (enemyNum < 1) {
        enemyNum = 1;
    } else if (enemyNum > 8) {
        enemyNum = 8;
    }

    if (maxEnemyType < 1) {
        maxEnemyType = 1;
    } else if (maxEnemyType > 9) {
        maxEnemyType = 9;
    }

    if (mEnemyNum != enemyNum) {
        mEnemyNum = enemyNum;
        ApplyTrackMask();
    }

    if (mMaxEnemyType != maxEnemyType) {
        mMaxEnemyType = maxEnemyType;
        ApplyTrackMask();
    }
}

void RPTnkBgmMix::ApplyTrackMask() {
    nw4r::snd::SeqSoundHandle handle(&RPSndAudioMgr::instance()->getBgmHandle());

    if (mStage < 1) {
        mStage = 1;
    }

    if (mEnemyNum < 1) {
        mEnemyNum = 1;
    } else if (mEnemyNum > 8) {
        mEnemyNum = 8;
    }

    if (mMaxEnemyType < 1) {
        mMaxEnemyType = 1;
    } else if (mMaxEnemyType > 9) {
        mMaxEnemyType = 9;
    }

    if (mPlaying) {
        s32 enemyNum = mEnemyNum;
        if (enemyNum > 4) {
            enemyNum = 4;
        }
        s32 col = enemyNum - 1;
        s32 row = mMaxEnemyType - 1;
        mTrackMask = sTrackMasks[row][col];
    } else {
        mTrackMask = 0xFFFF;
    }

    handle.SetTrackMute(0xFFFF, nw4r::snd::MUTE_NO_STOP);
    handle.SetTrackMute(mTrackMask, nw4r::snd::MUTE_OFF);
}

void RPTnkBgmMix::PlayStageClearBgm() {
    RPSndAudioMgr::instance()->stopBgm(10);
    RPSndAudioMgr::instance()->startBgm(0x3C);
    Reset();
}

void RPTnkBgmMix::Stop() {
    RPSndAudioMgr::instance()->stopBgm(10);
    mTrackMask = 0xFFFF;
    mStage = 0;
    mEnemyNum = 0;
    mMaxEnemyType = 0;
    mPlaying = false;
}

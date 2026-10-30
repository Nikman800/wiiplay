#include <Pack/RPParty/RPWlyScene/RPWlyGameConfig.h>

RPWlyMiiEntry::RPWlyMiiEntry() {
    mConfig.mMode = 0;
    mConfig.mScores[0] = 0;
    mConfig.mScores[1] = 0;
    mConfig.mTimer = 1800;
    mConfig.mpSceneSound = NULL;
    mConfig.mTierOffsets.mTier[0] = -1;
    mConfig.mTierOffsets.mTier[1] = -1;
    mConfig.mTierOffsets.mTier[2] = -1;
    mConfig.mTierOffsets.mTier[3] = -1;
    mConfig.mCursorPos[0].x = mConfig.mCursorPos[0].y = 0.0f;
    mConfig.mCursorAngles[0] = 0.0f;
    mConfig.mCursorPos[1].x = mConfig.mCursorPos[1].y = 0.0f;
    mConfig.mCursorAngles[1] = 0.0f;
}

RPWlyMiiEntry::~RPWlyMiiEntry() {}

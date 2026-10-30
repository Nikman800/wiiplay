// Pose Mii spawn pattern generator over param/BomQuestionParam.bin (RPBomQuestionParam).
#include <Pack/RPParty/RPBomScene/RPBomQuestionParam.h>

#include <Pack/RPParty/RPBomScene/RPBomGameManager.h>
#include <Pack/RPParty/RPBomScene/RPBomObjPool.h>
#include <Pack/RPParty/RPBomScene/RPBomPanel_3.h>
#include <Pack/RPParty/RPBomScene/RPBomScore.h>
#include <Pack/RPParty/RPDucScene/RPDucUtil.h>
#include <Pack/RPSystem/RPSysResourceManager.h>

#include <egg/gfxe/eggScreen.h>

#pragma opt_unroll_loops off

// RPBomCamera_2.cpp
extern "C" void Bom_shuffleArray(s32* pValues, u32 num);
extern "C" f32 Bom_getScreenHalfWidth();

// The same generator as RPBomCamera_2's BomRand
inline u32 BomRandU32() {
    RP_randSeedX = RP_randSeedX * 0x41C64E6D + 12345;
    if (RP_randSeedY & 1) {
        RP_randSeedY ^= 0x11020;
    }
    RP_randSeedY >>= 1;
    return RP_randSeedX ^ RP_randSeedY;
}

inline f32 BomRandF32() {
    return 1.192093e-7f * (f32)(BomRandU32() & 0x7FFFFF);
}

inline f32 BomRandRange(f32 lo, f32 hi) {
    if (lo == hi) {
        return lo;
    }
    return BomRandF32() * (hi - lo) + lo;
}

inline s32 BomRandRange(s32 lo, s32 hi) {
    if (lo == hi) {
        return lo;
    }
    return lo + BomRandU32() % (hi - lo);
}

inline bool IsPose(s32 pose) {
    bool valid = false;
    if (pose >= 0 && pose <= 2) {
        valid = true;
    }
    return valid;
}

// The spawn info is passed on by value (copied word by word)
inline void SpawnPanel(const nw4r::math::VEC3& rPos, u32 flags, RPBomSpawnInfo info) {
    RPBomPanelPool::sInstance->SpawnPanel(rPos, flags, info);
}

// Read through an enum-typed local, the mode loads into the base pointer's register (Reset)
enum BomGameMode { BOM_MODE_1P, BOM_MODE_2P };

inline BomGameMode BomGetMode() {
    return (BomGameMode)RPBomGameManager::sInstance->mMode;
}

inline void RPBomQuestionParam::ClearBubbles() {
    s32 i = 0;
    do {
        mSpawnPos[i] = RP_vecZero;
        mSpawnPos[i].y = 500.0f;
        mAngles[i] = 0.0f;
        mOrbitRadius[i] = 0.0f;
        mOrbitStep[i] = 0.0f;
        mRotSpeeds[i] = 0.0f;
        mFlags[i] = 0;
    } while (++i < 4);
}

inline void RPBomQuestionParam::ClearWave() {
    mLastSpawnPos = RP_vecZero;
    mLastSpawnPos.y = 500.0f;
    ClearBubbles();
}

inline void RPBomQuestionParam::PickPatterns() {
    mWaveNum = 1;
    s32 idx = mPatternOrder[mPatternOrderPos];
    unk8 = mpFile->mPatterns[0];
    s32 waveNum = mWaveNum;
    if (waveNum > 0) {
        mPatternIdx = idx;
        s32 i = 0;
        do {
            mPatterns[i] = mpFile->mPatterns[mPatternIdx];
            mPatternOrderPos++;
            if (mPatternOrderPos == mPatternOrderNum) {
                ShufflePatternOrder(mQuestion, FALSE);
            }
            mPatternIdx = mPatternOrder[mPatternOrderPos];
            i++;
        } while (i < waveNum);
    }
}

inline void RPBomQuestionParam::StartQuestion(s32 question) {
    mQuestion = question;
    mPrevQuestion = question;
    mLastSpawnPos = RP_vecZero;
    mLastSpawnPos.y = 500.0f;
    f32 speed;
    BomGameMode mode = BomGetMode();
    if (mode == BOM_MODE_1P) {
        speed = mpFile->mQuestions[mQuestion].mSpeed[0];
    } else {
        speed = mpFile->mQuestions[mQuestion].mSpeed[1];
    }
    mTargetSpeed = speed;
    mSpeed = mTargetSpeed;
    mSpeedStep = 0.0f;
    mBubbleCount = 0;
    mLastSpecialAt = 0;
    ClearBubbles();
    ShufflePatternOrder(mQuestion, TRUE);
    PickPatterns();
}

// The member functions call each other out of line.
#pragma auto_inline off

void RPBomQuestionParam::CreateInstance() {
    sInstance = new RPBomQuestionParam;
}

void RPBomQuestionParam::DestroyInstance() {
    delete sInstance;
    sInstance = NULL;
}

RPBomQuestionParam::~RPBomQuestionParam() {}

void RPBomQuestionParam::BuildLevelTable() {
    s32 num;
    mQuestionNum = 0;
    s32 i = 0;
    do {
        mPatternStart[i] = 0;
        mPatternNum[i] = 0;
        i++;
    } while (i < 128);

    s32 last = 0;
    num = GetPatternNum();
    i = 0;
    do {
        s32 question = mpFile->mPatterns[i].mQuestion;
        if (question <= last) {
            mPatternNum[last]++;
        } else if (question > last) {
            last = question;
            mPatternStart[question] = i;
            mPatternNum[question] = 1;
        }
        i++;
    } while (i < num);
    mQuestionNum = last;
}

BOOL RPBomQuestionParam::FindSpawnPosition() {
    f32 scale;
    if (EGG::Screen::GetTVMode() == EGG::Screen::TV_MODE_WIDE) {
        scale = mpFile->mBubbleScale169;
    } else {
        scale = mpFile->mBubbleScale43;
    }
    f32 radius = mpFile->mSpawnXMargin * scale;
    f32 left = 0.9f * -Bom_getScreenHalfWidth() + radius;
    f32 step = (-left - left) / 100.0f;
    f32 zMin = mPatterns[0].mSpawnZMin;
    f32 zMax = mPatterns[0].mSpawnZMax;
    f32 z = mpFile->mSpawnZ + BomRandRange(zMin, zMax);

    s32 slots[101];
    bool occupied[101];
    s32 i = 0;
    do {
        nw4r::math::VEC3 slotPos(step * i + left, 500.0f, z);
        slots[i] = i;
        nw4r::math::VEC3 pos = slotPos;
        occupied[i] = RPBomPanelPool::sInstance->IsPositionOccupied(pos, radius);
        i++;
    } while (i < 101);

    Bom_shuffleArray(slots, 101);
    i = 0;
    do {
        if (!occupied[slots[i]]) {
            mSpawnPos[0].x = step * slots[i] + left;
            mSpawnPos[0].y = 500.0f;
            mSpawnPos[0].z = z;
            return TRUE;
        }
        i++;
    } while (i < 101);
    return FALSE;
}

void RPBomQuestionParam::RandomizeSpeedAndSegments() {
    s32 waveNum = mWaveNum;
    s32 i = 0;
    do {
        f32 radiusMin = mPatterns[i].mRadiusMin;
        f32 radiusMax = mPatterns[i].mRadiusMax;
        mOrbitRadius[i] = BomRandRange(radiusMin, radiusMax);

        s32 segmentsMax;
        s32 segmentsMin = mPatterns[i].mSegmentsMin;
        segmentsMax = mPatterns[i].mSegmentsMax;
        s32 segments = BomRandRange(segmentsMin, segmentsMax);
        if (segments != 0) {
            mOrbitStep[i] = 360.0f / segments;
        } else {
            mOrbitStep[i] = 0.0f;
        }
        i++;
    } while (i < waveNum);
}

void RPBomQuestionParam::RandomizeRotSpeed() {
    s32 waveNum = mWaveNum;
    s32 i = 0;
    do {
        f32 speedMin = mPatterns[i].mRotSpeedMin;
        f32 speedMax = mPatterns[i].mRotSpeedMax;
        mRotSpeeds[i] = BomRandRange(speedMin, speedMax);

        bool positive = true;
        switch (mPatterns[i].mRotDirMode) {
        case 0:
            positive = BomRandU32() & 0x10;
            break;
        case 1:
            positive = true;
            break;
        case 2:
            positive = false;
            break;
        }
        if (!positive) {
            mRotSpeeds[i] = -mRotSpeeds[i];
        }
    } while (++i < waveNum);
}

void RPBomQuestionParam::PickWavePose() {
    // Slots of the last two waves' poses (a third repeat is avoided); ledger Bom_lastWavePoseSlots
    static u32 sLastSlots[2];
    s32 chosen = 0;
    s32 count = 0;
    s32 question = mQuestion;
    s32 slots[3] = {-1, -1, -1};

    if (IsPose(mForcedPose)) {
        mFlags[0] = mForcedPose;
        mForcedPose = -1;
    } else {
        if (mpFile->mQuestions[question].mPoseEnabled[0] == 1) {
            slots[0] = 0;
            count = 1;
        }
        if (mpFile->mQuestions[question].mPoseEnabled[1] == 1) {
            slots[count++] = 1;
        }
        if (mpFile->mQuestions[question].mPoseEnabled[2] == 1) {
            slots[count++] = 2;
        }

        if (count == 1) {
            chosen = slots[0];
            mFlags[0] = mPoses[chosen];
        } else if (count >= 2) {
            Bom_shuffleArray(slots, count);
            s32 i = 0;
            do {
                if (sLastSlots[0] != slots[i] || sLastSlots[1] != slots[i]) {
                    break;
                }
                i++;
            } while (i < count);
            chosen = slots[i];
            mFlags[0] = mPoses[chosen];
        }
    }
    u32 prev = sLastSlots[0];
    sLastSlots[0] = chosen;
    sLastSlots[1] = prev;
}

BOOL RPBomQuestionParam::RollTargetPlayer() {
    s32 score0 = RPBomScore::sInstance->mScores[0];
    s32 score1 = RPBomScore::sInstance->mScores[1];
    if (score0 == 0 && score1 == 0) {
        if (BomRandU32() & 0x10) {
            return TRUE;
        }
        return FALSE;
    }
    if (score0 == 0) {
        return TRUE;
    }
    if (score1 == 0) {
        return FALSE;
    }
    f32 ratio = (f32)score1 / (f32)(score0 + score1);
    if (BomRandRange(0.0f, 1.0f) < ratio) {
        return TRUE;
    }
    return FALSE;
}

BOOL RPBomQuestionParam::RollSpecialBubble() {
    if (RPBomPanelPool::sInstance->CountSpecialInRange() != 0) {
        return FALSE;
    }

    RPBomLevelData level = mpFile->mLevels[RPBomGameManager::sInstance->mLevel];
    if (mBubbleCount - mLastSpecialAt <= level.mSpecialGap) {
        return FALSE;
    }

    s32 chances[11] = {
        level.mSpecialChance[0], level.mSpecialChance[1], level.mSpecialChance[3],
        level.mSpecialChance[4], level.mSpecialChance[5], level.mSpecialChance[6],
        level.mSpecialChance[7], level.mSpecialChance[8], level.mSpecialChance[9],
        level.mSpecialChance[10], level.mSpecialChance[2],
    };
    s32 active = RPBomPanelPool::sInstance->CountActiveInRange();
    if (active > 10) {
        active = 10;
    }
    f32 chance = chances[active];
    if (BomRandRange(0.0f, 100.0f) >= chance) {
        return FALSE;
    }
    return TRUE;
}

void RPBomQuestionParam::Load() {
    EGG::Archive* pArchive = RPSysResourceManager::LoadGameCommonArchive();
    u32 size = 0;
    mpFile = static_cast<RPBomQuestionParamFile*>(
        RPSysResourceManager::GetFileFromArchive(pArchive, "param/BomQuestionParam.bin", &size));
    BuildLevelTable();
}

void RPBomQuestionParam::Reset() {
    mWaveNum = 1;
    mForcedPose = -1;
    if (BomRandU32() & 0x10) {
        mPoses[0] = 0;
        mPoses[1] = 2;
        mPoses[2] = 1;
    } else {
        mPoses[0] = 1;
        mPoses[1] = 2;
        mPoses[2] = 0;
    }
    StartQuestion(0);
    ClearWave();
}

void RPBomQuestionParam::SetQuestion(s32 question) {
    StartQuestion(question);
}

void RPBomQuestionParam::ShufflePatternOrder(s32 question, BOOL keepFirst) {
    s32 i = 0;
    do {
        mPatternOrder[i] = i + mPatternStart[question];
        i++;
    } while (i < mPatternNum[question]);

    if (mPatternNum[question] > 1) {
        if (keepFirst == 1U && mPatternNum[question] > 2) {
            Bom_shuffleArray(&mPatternOrder[1], mPatternNum[question] - 1);
        } else {
            Bom_shuffleArray(mPatternOrder, mPatternNum[question]);
        }
    }

    i = mPatternNum[question];
    do {
        mPatternOrder[i] = -1;
        i++;
    } while (i < 500);
    mPatternOrderNum = mPatternNum[question];
    mPatternOrderPos = 0;
}

inline void RPBomQuestionParam::RandomizeAngles() {
    s32 waveNum = mWaveNum;
    s32 i = 0;
    do {
        f32 angleMin = mPatterns[i].mAngleMin;
        f32 angleMax = mPatterns[i].mAngleMax;
        if (angleMin > angleMax) {
            angleMax += 360.0f;
        }
        mAngles[i] = BomRandRange(angleMin, angleMax);
        if (mAngles[i] > 360.0f) {
            mAngles[i] -= 360.0f;
        }
        i++;
    } while (i < waveNum);
}

BOOL RPBomQuestionParam::GenerateWave() {
    ClearBubbles();
    if (FindSpawnPosition()) {
        RandomizeAngles();
        RandomizeSpeedAndSegments();
        RandomizeRotSpeed();
        PickWavePose();
        mLastSpawnPos = mSpawnPos[0];
        return TRUE;
    }
    return FALSE;
}

void RPBomQuestionParam::SpawnBubbles() {
    f32 scale;
    if (EGG::Screen::GetTVMode() == EGG::Screen::TV_MODE_WIDE) {
        scale = mpFile->mBubbleScale169;
    } else {
        scale = mpFile->mBubbleScale43;
    }

    s32 waveNum = mWaveNum;
    s32 i = 0;
    do {
        nw4r::math::VEC3 pos = mSpawnPos[i];
        nw4r::math::VEC3 vel(0.0f, 0.0f, mSpeed);
        u32 flags = mFlags[i];
        if (RollSpecialBubble()) {
            flags |= 0x20;
            RPBomGameManager::sInstance->PlaySpecialJingle();
            mLastSpecialAt = mBubbleCount + 1;
        } else if (RPBomGameManager::sInstance->mMode == 1 && RollTargetPlayer()) {
            flags |= 0x10;
        }

        RPBomSpawnInfo spawnInfo;
        spawnInfo.mAngle = mAngles[i];
        spawnInfo.mVel = vel;
        spawnInfo.mRotSpeed = mRotSpeeds[i];
        spawnInfo.mScale = scale;
        spawnInfo.mOrbitRadius = mOrbitRadius[i];
        spawnInfo.mOrbitStep = mOrbitStep[i];
        SpawnPanel(pos, flags, spawnInfo);
        mBubbleCount++;
        i++;
    } while (i < waveNum);
}

void RPBomQuestionParam::NextPattern(s32 question) {
    ClearWave();
    mQuestion = question;
    if (question > mPrevQuestion) {
        ShufflePatternOrder(question, TRUE);
    }
    PickPatterns();
    mPrevQuestion = mQuestion;
}

void RPBomQuestionParam::UpdateSpeed(s32 question) {
    f32 target;
    if (RPBomGameManager::sInstance->mMode == 0) {
        target = mpFile->mQuestions[question].mSpeed[0];
    } else {
        target = mpFile->mQuestions[question].mSpeed[1];
    }

    if (mTargetSpeed != target) {
        mSpeedStep = target - mTargetSpeed;
        mSpeedStep /= mpFile->mSpeedChangeFrames;
        mTargetSpeed = target;
    }

    if (mTargetSpeed != mSpeed) {
        if (mSpeedStep > 0.0f) {
            mSpeed += mSpeedStep;
            if (mSpeed > mTargetSpeed) {
                mSpeed = mTargetSpeed;
            }
        } else {
            mSpeed += mSpeedStep;
            if (mSpeed < mTargetSpeed) {
                mSpeed = mTargetSpeed;
            }
        }
    }
}

s32 RPBomQuestionParam::PickRandomPose() {
    s32 slots[3] = {-1, -1, -1};
    s32 count = 0;
    if (mpFile->mQuestions[mQuestion].mPoseEnabled[0] == 1) {
        slots[0] = 0;
        count = 1;
    }
    if (mpFile->mQuestions[mQuestion].mPoseEnabled[1] == 1) {
        slots[count++] = 1;
    }
    if (mpFile->mQuestions[mQuestion].mPoseEnabled[2] == 1) {
        slots[count++] = 2;
    }

    if (count == 1) {
        return mPoses[slots[0]];
    }
    if (count >= 2) {
        Bom_shuffleArray(slots, count);
        return mPoses[slots[0]];
    }
    return 0;
}

// Stand-in (unreferenced, dead-stripped at link): compiled first, it puts 0.0f and 1.0f ahead
// of UpdateSpeed's constants in .sdata2, as the original (RPBomStartBg has the same pattern).
extern "C" void Bom_QuestionParam_poolOrder(RPBomQuestionParam* pParam) {
    pParam->mSpeed = 0.0f;
    pParam->mTargetSpeed = 1.0f;
}

// The original's .sdata2 has this constant after all the functions' literal pool: the functions
// above are emitted before it.
#pragma defer_codegen off

const s32 RPBomQuestionParam::sPatternNum = 250;

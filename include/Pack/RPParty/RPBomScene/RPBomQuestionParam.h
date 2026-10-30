#ifndef RP_PARTY_BOM_QUESTION_PARAM_H
#define RP_PARTY_BOM_QUESTION_PARAM_H
#include <Pack/types_pack.h>

#include <Pack/RPParty/RPPartyCommon/RPSceneObj.h>

#include <nw4r/math.h>

//! @addtogroup rp_bom
//! @{

/**
 * @brief One question (difficulty step) record of BomQuestionParam.bin
 * @details Size 0x28 (indexed * 0x28 from file offset 0x3C, 101 records). Field names from
 * their users in RPBomGameManager / RPBomQuestionParam.
 */
struct RPBomQuestionData {
    s32 mMissNext;       // at 0x0 question after a miss
    s32 mCost;           // at 0x4 points needed to advance (x file 0xC in two-player mode)
    s32 mPoseEnabled[3]; // at 0x8 1: the pose may be asked for
    s32 mWaveInterval;   // at 0x14 frames between waves (x level 0x30 in two-player mode)
    s32 mWaveNum[2];     // at 0x18 waves per burst, one / two players
    f32 mSpeed[2];       // at 0x20 bubble speed, one / two players
};

/**
 * @brief One spawn pattern record of BomQuestionParam.bin
 * @details Size 0x38 (250 records from file offset 0x1004). Patterns are grouped by question
 * (mQuestion, ascending); RPBomQuestionParam picks them in shuffled order.
 */
struct RPBomPatternData {
    u32 mSegmentsMax;    // at 0x0 orbit segments (360 / n degrees per frame) max
    u32 mSegmentsMin;    // at 0x4
    f32 mRadiusMax;      // at 0x8 orbit radius max
    f32 mRadiusMin;      // at 0xC
    u32 unk10;           // at 0x10 [L]
    f32 unk14;           // at 0x14 [L]
    s32 mQuestion;       // at 0x18
    s32 mRotDirMode;     // at 0x1C 0 random, 1 positive, 2 negative
    f32 mRotSpeedMax;    // at 0x20
    f32 mRotSpeedMin;    // at 0x24
    f32 mAngleMin;       // at 0x28 degrees
    f32 mAngleMax;       // at 0x2C
    f32 mSpawnZMax;      // at 0x30 [M]
    f32 mSpawnZMin;      // at 0x34 [M]
};

/**
 * @brief One level record of BomQuestionParam.bin
 * @details Size 0x44 (16 records from file offset 0x46B4).
 */
struct RPBomLevelData {
    s32 unk0;                // at 0x0 [L] never read
    s32 mSpecialChance[11];  // at 0x4 [M] percent chance of a special bubble by bubbles on
                             // screen (0-10); read through a reordered local copy
    f32 mWaveIntervalScale;  // at 0x30 two-player wave interval multiplier [M]
    s32 mTime;               // at 0x34 seconds of spawning
    s32 mStartQuestion;      // at 0x38
    s32 mBgFrame;            // at 0x3C background colour frame (RPBomStartBg)
    s32 mSpecialGap;         // at 0x40 bubbles between special bubbles
};

/**
 * @brief Pose Mii parameter file (param/BomQuestionParam.bin) [M]
 * @details Only the fields read by the game are named; the header layout before 0x3C is
 * partly unknown.
 */
struct RPBomQuestionParamFile {
    f32 mMissZ;                          // at 0x0 bubbles past this z are missed
    f32 mHitZoneFar;                     // at 0x4 hit zone along z (RPBomPanelPool)
    f32 mHitZoneNear;                    // at 0x8
    f32 mTwoPlayerCostScale;             // at 0xC
    u32 unk10;                           // at 0x10 [L]
    s32 mSpeedChangeFrames;              // at 0x14 frames to ease to a new question's speed
    f32 mBubbleScale169;                 // at 0x18 bubble / Mii scale on 16:9
    f32 mBubbleScale43;                  // at 0x1C on 4:3
    u32 unk20[2];                        // at 0x20 [L]
    s32 mMissWait;                       // at 0x28 frames after a miss
    s32 mMissMax;                        // at 0x2C misses before game over
    f32 mSpawnXMargin;                   // at 0x30 [M] times the bubble scale
    f32 mSpawnZ;                         // at 0x34 [M]
    s32 mBubbleWait;                     // at 0x38 frames after a special bubble
    RPBomQuestionData mQuestions[101];   // at 0x3C
    RPBomPatternData mPatterns[250];     // at 0x1004
    RPBomLevelData mLevels[16];          // at 0x46B4
    s32 mComboPoints[10];                // at 0x4AF4 points per bubble by combo
};

/**
 * @brief Spawn pattern generator over BomQuestionParam.bin (singleton
 * RPBomQuestionParam::sInstance)
 * @details vtable 8035EE38. Size 0xDBC, verified (Bom_QuestionParam_createInstance). Load
 * reads the file and builds the per-question pattern ranges (BuildLevelTable). Each wave
 * picks the next pattern of the current question (shuffled order, mPatternOrder), finds a
 * free spawn position among 101 slots across the screen and randomises angle, orbit, rotation
 * and pose per bubble (up to mWaveNum = 4 bubbles; always 1 in practice).
 * mPoses holds the three pose indices in a random order chosen on Reset.
 * @note Name is a project convention (ledger Bom_QuestionParam).
 */
class RPBomQuestionParam : public RPSceneObj {
public:
    static RPBomQuestionParam* sInstance; // 8043CAA0

    RPBomQuestionParam() : mpFile(NULL) {}

    virtual ~RPBomQuestionParam(); // at 0x8

    virtual void Load();  // at 0x10
    virtual void Reset(); // at 0x14

    void BuildLevelTable();
    void SetQuestion(s32 question);
    void NextPattern(s32 question);
    void ShufflePatternOrder(s32 question, BOOL keepFirst);
    void UpdateSpeed(s32 question);
    BOOL GenerateWave();
    void SpawnBubbles();
    BOOL FindSpawnPosition();
    void RandomizeSpeedAndSegments();
    void RandomizeRotSpeed();
    void PickWavePose();
    s32 PickRandomPose();
    BOOL RollSpecialBubble();
    BOOL RollTargetPlayer();

    static void CreateInstance();
    static void DestroyInstance();

    /**
     * @brief Number of pattern records; read from memory (the definition follows the code) [H]
     */
    static s32 GetPatternNum() {
        return sPatternNum;
    }

    static const s32 sPatternNum; // 80441748 (250 patterns)

private:
    // Inline steps shared by Reset, SetQuestion, NextPattern and GenerateWave [H]
    void ClearBubbles();
    void RandomizeAngles();
    void ClearWave();
    void PickPatterns();
    void StartQuestion(s32 question);

public:
    RPBomQuestionParamFile* mpFile;          // at 0x4
    RPBomPatternData unk8;                   // at 0x8 [L] copy of pattern 0, never read
    RPBomPatternData mPatterns[4];           // at 0x40 per bubble of the wave (only [0] set)
    s32 mQuestionNum;                        // at 0x120 last question index in the patterns
    s32 mPatternStart[128];                  // at 0x124 first pattern per question
    s32 mPatternNum[128];                    // at 0x324 patterns per question
    s32 mQuestion;                           // at 0x524
    s32 mPrevQuestion;                       // at 0x528
    s32 mPatternIdx;                         // at 0x52C
    s32 mPatternOrder[500];                  // at 0x530 -1 after the question's patterns
    s32 mPatternOrderNum;                    // at 0xD00
    s32 mPatternOrderPos;                    // at 0xD04
    s32 mPoses[3];                           // at 0xD08 random permutation of 0-2
    f32 mTargetSpeed;                        // at 0xD14
    f32 mSpeed;                              // at 0xD18 bubble fall speed (eased)
    f32 mSpeedStep;                          // at 0xD1C
    s32 mBubbleCount;                        // at 0xD20 bubbles spawned this question
    s32 mLastSpecialAt;                      // at 0xD24 mBubbleCount at the last special
    s32 mWaveNum;                            // at 0xD28 bubbles per wave (1)
    nw4r::math::VEC3 mLastSpawnPos;          // at 0xD2C
    nw4r::math::VEC3 mSpawnPos[4];           // at 0xD38 (x, 500, z)
    f32 mAngles[4];                          // at 0xD68 degrees
    f32 mOrbitRadius[4];                     // at 0xD78
    f32 mOrbitStep[4];                       // at 0xD88 360 / segments
    f32 mRotSpeeds[4];                       // at 0xD98
    u32 mFlags[4];                           // at 0xDA8 pose | 0x10 (2P target) | 0x20 (special)
    s32 mForcedPose;                         // at 0xDB8 pose of the next wave, -1 for random
};

//! @}

#endif
